#include "nlm_filter_gray2.h"
#include "utils.h"
#include <cmath>

void nlm_filter_gray2(QImage* imageNoise,
                        QImage* imageFiltered,
                        QSize imageSize,
                        int halfWindowSize,
                        int halfPatchSize,
                        double fSigma,
                        double fParam) {
    if (fSigma > 0.0f && fSigma <= 15.0f) {
        halfPatchSize = 1;
        halfWindowSize = 10;
        fParam = 0.4f;
    } else if ( fSigma > 15.0f && fSigma <= 30.0f) {
        halfPatchSize = 2;
        halfWindowSize = 10;
        fParam = 0.4f;
    } else if ( fSigma > 30.0f && fSigma <= 45.0f) {
        halfPatchSize = 3;
        halfWindowSize = 17;
        fParam = 0.35f;
    } else if ( fSigma > 45.0f && fSigma <= 75.0f) {
        halfPatchSize = 4;
        halfWindowSize = 17;
        fParam = 0.35f;
    } else if (fSigma <= 100.0f) {
        halfPatchSize = 5;
        halfWindowSize = 17;
        fParam = 0.30f;
    }

    /* SIZES */
    int patchSize = halfPatchSize * 2 + 1;
    int m = imageSize.width();
    int n = imageSize.height();
    int w = m;
    int h = n;
    int incWidth = patchSize - 1 + w;
    int incHeight = patchSize - 1 + h;

    /* CREATE ARRAYS */
    double** colorInput = new double*[w];
    double** colorOutput = new double*[w];
    for (int i = 0; i < imageSize.width(); i++) {
        colorInput[i] = new double[h];
        colorOutput[i] = new double[h];
    }
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            colorInput[i][j] = qGray(imageNoise->pixel(i,j));
        }
    }
    double** kernel = new double*[patchSize];
    for (int i = 0; i < patchSize; i++) {
        kernel[i] = new double[patchSize];
    }
    make_kernel(kernel,patchSize,fSigma);

    /* FILTER PARAMETER */
    double fSigma2 = fSigma * fSigma;

    /* INCREASE IMAGE */
    double** increasedImage = new double*[incWidth];
    for (int i = 0; i < incWidth; i++) {
        increasedImage[i] = new double[incHeight];
    }
    nlm_increse_image(colorInput, increasedImage, imageSize, halfPatchSize);

    /* FILTER */
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int i1 = i+halfPatchSize;
            int j1 = j+halfPatchSize;

            double wmax = 0;
            double average = 0;
            double sweight = 0;

            int rmin = std::max(i1-halfWindowSize,halfPatchSize+1);
            int rmax = std::min(i1+halfWindowSize,m+halfPatchSize);
            int smin = std::max(j1-halfWindowSize,halfPatchSize+1);
            int smax = std::min(j1+halfWindowSize,n+halfPatchSize);

            for (int r = rmin; r < rmax; r++) {
                for (int s = smin; s < smax; s++) {
                    if (r == i1 && s == j1) {
                        continue;
                    }

                    double fDif = 0;
                    double dif = 0;
                    for (int ii = -halfPatchSize; ii <= halfPatchSize; ii++) {
                        for (int jj = -halfPatchSize; jj <= halfPatchSize; jj++) {
                            dif = increasedImage[i1+ii][j1+jj]-increasedImage[r+ii][s+jj];
                            fDif += kernel[ii+halfPatchSize][jj+halfPatchSize] * dif*dif;
                        }
                    }

                    double W = exp(-fDif/fSigma2);

                    if (W > wmax) {
                        wmax = W;
                    }

                    sweight += W;
                    average += W * increasedImage[r][s];
                }
            }

            average += wmax * increasedImage[i1][j1];
            sweight += wmax;

            if (sweight > 0) {
                colorOutput[i][j] = average / sweight;
            }
            else {
                colorOutput[i][j] = colorInput[i][j];
            }
        }
    }

    /* CREATE FILTERED IMAGE */
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int gray = colorOutput[i][j];
            imageFiltered->setPixel(i, j, qRgb(gray, gray, gray));
        }
    }

    /* CLEAR MEMORY */
    for (int i = 0; i < w; i++) {
        delete []colorInput[i];
        delete []colorOutput[i];
    }
    delete []colorOutput;
    delete []colorInput;
    for (int i = 0; i < patchSize; i++) {
        delete []kernel[i];
    }
    delete []kernel;
}
