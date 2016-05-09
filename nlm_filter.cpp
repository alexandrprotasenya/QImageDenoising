#include "nlm_filter.h"

#include <QImageWriter>
#include <QVector>
#include <QDebug>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

#include <cmath>
#include <iostream>
#include <algorithm>

#include <QDebug>

#include "utils.h"

//#define MTHREADING

class FilterParam {
private:
    QSize m_imageSize;
    int m_windowSize;
    int m_patchSize;
    double m_sigma;
    double m_param;
public:
    FilterParam(QSize size, int windowSize, int patchSize, double sigma) {
        m_imageSize = size;
        m_windowSize = windowSize;
        m_patchSize = patchSize;
        m_sigma = sigma;
    }
    QSize getImageSize() const {
        return m_imageSize;
    }
    int getWindowSize() const {
        return m_windowSize;
    }
    int getPatchSize() const {
        return m_patchSize;
    }
    double getSigma() const {
        return m_sigma;
    }
};

void createColorArrays(double** red, double** green, double** blue, QImage* image, int w, int h) {
    for (int i = 0; i < w; i++) {
        red[i] = new double[h];
        green[i] = new double[h];
        blue[i] = new double[h];
        for (int j = 0; j < h; j++) {
            QRgb p = image->pixel(i, j);
            red[i][j] = qRed(p);
            green[i][j] = qGreen(p);
            blue[i][j] = qBlue(p);
        }
    }
}

void deleteColorsArrays(double** red, double** green, double** blue, int w) {
    for (int i = 0; i < w; i++) {
        delete red[i];
        delete green[i];
        delete blue[i];
    }
    delete red;
    delete green;
    delete blue;
}

inline void createImageFromColors(double** red, double** green, double** blue, QImage* image, int w, int h, int d) {
    d = d/2;
    for (int i = d; i < w-d; i++) {
        for (int j = d; j < h-d; j++) {
            QRgb p = qRgb(red[i][j], green[i][j], blue[i][j]);
            image->setPixel(i-d, j-d, p);
        }
    }
}

void processColorArray(double** colorInput, double** colorOutput, FilterParam filterParam) {
    int imageWidth = filterParam.getImageSize().width();
    int imageHeight = filterParam.getImageSize().height();
    int windowSize = filterParam.getWindowSize();
    int patchSize = filterParam.getPatchSize();

    double filterP = 0.4f;
    double fSigma2 = filterParam.getSigma() * filterParam.getSigma();
    double fH2 = filterP*filterP;

    int halfWindowSize = windowSize/2;
    int halfPatchSize = patchSize/2;

    int m = imageWidth - windowSize;
    int n = imageHeight - windowSize;

    double** kernel = new double*[patchSize];
    double** Patch1 = new double*[patchSize];
    double** Patch2 = new double*[patchSize];
    for (int i = 0; i < patchSize; i++) {
        Patch1[i] = new double[patchSize];
        Patch2[i] = new double[patchSize];
        kernel[i] = new double[patchSize];
    }
    make_kernel(kernel,patchSize,filterParam.getSigma());

    std::cout << imageWidth << " " << imageHeight << std::endl;
    for (int i = 0; i < imageWidth; i++) {
        for (int j = 0; j < imageHeight; j++) {
//            std::cout << i << " " << j << std::endl;
            int i1 = i + halfPatchSize;
            int j1 = j + halfPatchSize;
            for (int a = 0; a < patchSize; a++) {
                for (int b = 0; b < patchSize; b++) {
                    Patch1[a][b] = colorInput[i1-halfPatchSize+a][j1-halfPatchSize+a];
                }
            }

            double wmax = 0;
            double average = 0;
            double sweight = 0;

            int rmin = std::max(i1-halfWindowSize,halfPatchSize+1) - 1;
            int rmax = std::min(i1+halfWindowSize,m+halfPatchSize);
            int smin = std::max(j1-halfWindowSize,halfPatchSize+1) - 1;
            int smax = std::min(j1+halfWindowSize,n+halfPatchSize);

            for (int r = rmin; r < rmax; r++) {
                for (int s = smin; s < smax; s++) {
                    if (r == i1 && s == j1) {
                        continue;
                    }

                    for (int a = 0; a < patchSize; a++) {
                        for (int b = 0; b < patchSize; b++) {
                            Patch2[a][b] = colorInput[r-halfPatchSize+a][s-halfPatchSize+a];
                        }
                    }

                    double N = 0;
                    for (int a = 0; a < patchSize; a++) {
                        for (int b = 0; b < patchSize; b++) {
                            N += kernel[a][b] * ((Patch1[a][b] - Patch2[a][b]) *
                                                 (Patch1[a][b] - Patch2[a][b]));
                        }
                    }

//                                        double W = exp(-N/fSigma2);
                    double W = exp(-N/((patchSize+1)*(patchSize+1)*fH2 * fSigma2));

                    if (W > wmax) {
                        wmax = W;
                    }

                    sweight += W;
                    average += W * colorInput[r][s];
                }
            }

            average += wmax * colorInput[i1][j1];
            sweight += wmax;

            if (sweight > 0) {
                colorOutput[i][j] = average / sweight;
            } else {
                colorOutput[i][j] = colorInput[i1][j1];
            }
        }
    }

    for (int i = 0; i < patchSize; i++) {
        delete Patch1[i];
        delete Patch2[i];
        delete kernel[i];
    }
    delete kernel;
    delete Patch1;
    delete Patch2;
}

QImage* nlm_filter(QImage* image, QSize size, int d, int ld, double sigma) {
    int pW = size.width()+d-(d%2!=0?1:0);
    int pH = size.height()+d-(d%2!=0?1:0);

    QImage* imageFiltered = new QImage(pW, pH, QImage::Format_RGB32);
    QImage* tmpImage1 = new QImage(pW, pH, QImage::Format_RGB32);

    nlm_increse_image(image, imageFiltered, size, d);


    double** redArray = new double*[pW];
    double** greenArray = new double*[pW];
    double** blueArray = new double*[pW];

    double** redArrayOutput = new double*[pW];
    double** greenArrayOutput = new double*[pW];
    double** blueArrayOutput = new double*[pW];

    createColorArrays(redArray, greenArray, blueArray, imageFiltered, pW, pH);
    createColorArrays(redArrayOutput, greenArrayOutput, blueArrayOutput, tmpImage1, pW, pH);

#ifdef MTHREADING
    QFuture<void> prRed = QtConcurrent::run(
                processColorArray,
                redArray,
                redArrayOutput,
                FilterParam(QSize(pW, pH), d, ld, sigma)
                );
    QFuture<void> prGreen = QtConcurrent::run(
                processColorArray,
                greenArray,
                greenArrayOutput,
                FilterParam(QSize(pW, pH), d, ld, sigma)
                );
    QFuture<void> prBlue = QtConcurrent::run(
                processColorArray,
                blueArray,
                blueArrayOutput,
                FilterParam(QSize(pW, pH), d, ld, sigma)
                );
    prRed.waitForFinished();
    prGreen.waitForFinished();
    prBlue.waitForFinished();
#else
    processColorArray(redArray, redArrayOutput, FilterParam(QSize(pW, pH), d, ld, sigma));
    processColorArray(greenArray, greenArrayOutput, FilterParam(QSize(pW, pH), d, ld, sigma));
    processColorArray(blueArray, blueArrayOutput, FilterParam(QSize(pW, pH), d, ld, sigma));
#endif

    QImage* tmpImage = new QImage(size, QImage::Format_RGB32);
    createImageFromColors(redArrayOutput, greenArrayOutput, blueArrayOutput, tmpImage, pW, pH, d);

    deleteColorsArrays(redArray, greenArray, blueArray, pW);
    deleteColorsArrays(redArrayOutput, greenArrayOutput, blueArrayOutput, pW);

    QImageWriter* writer = new QImageWriter();
    writer->setFileName("tmp.png");
    writer->write(*tmpImage);

    delete tmpImage1;
    delete imageFiltered;

    return tmpImage;
}
