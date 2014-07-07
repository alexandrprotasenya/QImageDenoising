#include "nlm_filter.h"

#include <QImageWriter>
#include <QVector>
#include <QDebug>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

#include <cmath>
#include <iostream>
#include <algorithm>

void nlm_increse_image(QImage*, QImage*, QSize, int);

#define MTHREADING

#define H 10.0

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

int comp1 (const double *i, const double *j) {
    return *i - *j;
}

void processColorArray(double** colorInput, double** colorOutput, QSize size, int d, int ld) {
    int w = size.width();
    int h = size.height();
    int patchSize = d;
    int patchSizeQ = ld;

    double H2 = H*H;

    int t = patchSize/2;
    int f = patchSizeQ/2;

    int m = w - patchSize;
    int n = h - patchSize;

    double** kernel = new double*[patchSizeQ];
    for (int i = 0; i < patchSizeQ; i++) {
        kernel[i] = new double[patchSizeQ];
        for (int j = 0; j < patchSizeQ; j++) {
            kernel[i][j] = 0;
        }
    }

    for (int k = 0; k < f; k++) {
        double value = 1. / ((2*k+1)*(2*k+1));
        for (int i = 0; i < patchSizeQ; i++) {
            for (int j = 0; j < patchSizeQ; j++) {
                kernel[i][j] += value;
            }
        }
    }

    double ks = 0;
    for (int i = 0; i < patchSizeQ; i++) {
        for (int j = 0; j < patchSizeQ; j++) {
            kernel[i][j] /= f;
            ks += kernel[i][j];
        }
    }

    for (int i = 0; i < patchSizeQ; i++) {
        for (int j = 0; j < patchSizeQ; j++) {
            kernel[i][j] /= ks;
        }
    }

    double** W1 = new double*[patchSizeQ];
    double** W2 = new double*[patchSizeQ];
    for (int i = 0; i < patchSizeQ; i++) {
        W1[i] = new double[patchSizeQ];
        W2[i] = new double[patchSizeQ];
    }

    for (int i = 0; i < w-t; i++) {
        for (int j = 0; j < h-t; j++) {
            int i1 = i + f;
            int j1 = j + f;
            for (int a = 0; a < patchSizeQ; a++) {
                for (int b = 0; b < patchSizeQ; b++) {
                    W1[a][b] = colorInput[i1-f+a][j1-f+a];
                }
            }

            double wmax = 0;
            double average = 0;
            double sweight = 0;

            int rmin = std::max(i1-t,f+1) - 1;
            int rmax = std::min(i1+t,m+f);
            int smin = std::max(j1-t,f+1) - 1;
            int smax = std::min(j1+t,n+f);

            for (int r = rmin; r < rmax; r++) {
                for (int s = smin; s < smax; s++) {
                    if (r == i1 && s == j1) {
                        continue;
                    }

                    for (int a = 0; a < patchSizeQ; a++) {
                        for (int b = 0; b < patchSizeQ; b++) {
                            W2[a][b] = colorInput[r-f+a][s-f+a];
                        }
                    }

                    double N = 0;
                    for (int a = 0; a < patchSizeQ; a++) {
                        for (int b = 0; b < patchSizeQ; b++) {
                            N += kernel[a][b] * ((W1[a][b] - W2[a][b]) *
                                                 (W1[a][b] - W2[a][b]));
                        }
                    }

                    double Z = exp(-N/H2);

                    if (Z > wmax) {
                        wmax = Z;
                    }

                    sweight += Z;
                    average += Z * colorInput[r][s];
                }
            }

            average += wmax * colorInput[i1][j1];
            sweight += wmax;

            if (sweight > 0) {
                colorOutput[i1][j1] = average / sweight;
            } else {
                colorOutput[i1][j1] = colorInput[i1][j1];
            }
        }
    }

    for (int i = 0; i < patchSizeQ; i++) {
        delete W1[i];
        delete W2[i];
    }
    delete kernel;
    delete W1;
    delete W2;
}

QImage* nlm_filter(QImage* image, QSize size, int d, int ld) {
    int pW = size.width()+d-1;
    int pH = size.height()+d-1;

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
    QFuture<void> prRed = QtConcurrent::run(processColorArray, redArray, redArrayOutput, QSize(pW, pH), d, ld);
    QFuture<void> prGreen = QtConcurrent::run(processColorArray, greenArray, greenArrayOutput, QSize(pW, pH), d, ld);
    QFuture<void> prBlue = QtConcurrent::run(processColorArray, blueArray, blueArrayOutput, QSize(pW, pH), d, ld);
    prRed.waitForFinished();
    prGreen.waitForFinished();
    prBlue.waitForFinished();
#else
    processColorArray(redArray, redArrayOutput, QSize(pW, pH), d, ld);
    processColorArray(greenArray, greenArrayOutput, QSize(pW, pH), d, ld);
    processColorArray(blueArray, blueArrayOutput, QSize(pW, pH), d, ld);
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

void nlm_increse_image(QImage* image, QImage* imageFiltered, QSize size, int d) {
    d = d-1;
    int dh = d/2;
    // copy image
    for (int i = dh; i < size.width()+dh; i++) {
        for (int j = dh; j < size.height()+dh; j++) {
            imageFiltered->setPixel(i, j, image->pixel(i-dh, j-dh));
        }
    }

    // copy borders
    for (int i = dh; i < size.width()+dh; i++) {
        for (int j = 0; j < dh; j++) {
            imageFiltered->setPixel(i, j, image->pixel(i-dh, 0));
        }
    }

    for (int i = dh; i < size.width()+dh; i++) {
        for (int j = size.height()+dh; j < size.height()+d; j++) {
            imageFiltered->setPixel(i, j, image->pixel(i-dh, size.height()-1));
        }
    }

    for (int i = 0; i < dh; i++) {
        for (int j = dh; j < size.height()+dh; j++) {
            imageFiltered->setPixel(i, j, image->pixel(0, j-dh));
        }
    }

    for (int i = size.width()+dh; i < size.width()+d; i++) {
        for (int j = dh; j < size.height()+dh; j++) {
            imageFiltered->setPixel(i, j, image->pixel(size.width()-1, j-dh));
        }
    }

    // copy corners
    for (int i = 0; i < dh; i++) {
        for (int j = 0; j < dh; j++) {
            imageFiltered->setPixel(i, j, image->pixel(0, 0));
        }
    }

    for (int i = 0; i < dh; i++) {
        for (int j = size.height()+dh; j < size.height()+d; j++) {
            imageFiltered->setPixel(i, j, image->pixel(0, size.height()-1));
        }
    }

    for (int i = size.width()+dh; i < size.width()+d; i++) {
        for (int j = 0; j < dh; j++) {
            imageFiltered->setPixel(i, j, image->pixel(size.width()-1, 0));
        }
    }

    for (int i = size.width()+dh; i < size.width()+d; i++) {
        for (int j = size.height()+dh; j < size.height()+d; j++) {
            imageFiltered->setPixel(i, j, image->pixel(size.width()-1, size.height()-1));
        }
    }
}
