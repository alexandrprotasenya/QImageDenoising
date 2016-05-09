#include "nlm_filter_gray3.h"
#include "utils.h"
#include <cmath>

#include <QImageWriter>

void double2img(double** src, int w, int h) {
    QImage* TESTIMG = new QImage(w, h, QImage::Format_RGB32);

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            TESTIMG->setPixel(i,j,qRgb(src[i][j],src[i][j],src[i][j]));
        }
    }

    QImageWriter* TESTIMG_W = new QImageWriter();
    TESTIMG_W->setFileName("TESTIMG.png");
    TESTIMG_W->write(*TESTIMG);
    delete TESTIMG;
    delete TESTIMG_W;
}

void nlm_filter_gray3(QImage* imageNoise,
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


//    halfWindowSize = 6;
//    halfPatchSize = 1;
//    fParam = 0.99f;

    /* SIZES */
    int patchSize = halfPatchSize * 2 + 1;
    //    int winSize = halfWindowSize * 2 + 1;
    int m = imageSize.width();
    int n = imageSize.height();
    int w = m;
    int h = n;
    int incWidth = patchSize - 1 + w;
    int incHeight = patchSize - 1 + h;

    /* CREATE ARRAYS */
    double** colorInput = new double*[w];
    for (int i = 0; i < w; i++) {
        colorInput[i] = new double[h];
    }
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            colorInput[i][j] = qGray(imageNoise->pixel(i,j));
        }
    }
    double** dCount = new double*[incWidth];
    double** colorOutput = new double*[incWidth];
    for (int i = 0; i < incWidth; i++) {
        colorOutput[i] = new double[incHeight];
        dCount[i] = new double[incHeight];
    }
    for (int i = 0; i < incWidth; i++)
        for (int j = 0; j < incHeight; j++) {
            dCount[i][j] = 0.0;
            colorOutput[i][j] = 0.0;
        }

    /* FILTER PARAMETER */
    double dSigma2 = fSigma * fSigma;
    double dH = fParam * fSigma;
    double dH2 = dH * dH;
    int icwl = 1 * patchSize * patchSize;
    dH2 *= icwl;

    /* INCREASE IMAGE */
    double** increasedImage = new double*[incWidth];
    for (int i = 0; i < incWidth; i++) {
        increasedImage[i] = new double[incHeight];
    }
    nlm_increse_image2(colorInput, increasedImage, imageSize, halfPatchSize);
    double2img(increasedImage,incWidth,incHeight);
    /* FILTER */
    for (int i = 0; i < w; i++) {
        double** denoised = new double*[patchSize];
        for (int id = 0; id < patchSize; id++) {
            denoised[id] = new double[patchSize];
        }
        for (int j = 0; j < h; j++) {
            int x = i+halfPatchSize;
            int y = j+halfPatchSize;

            double dMaxWeight = 0.0;
            double dTotalWeight = 0.0;

            int rmin = std::max(x-halfWindowSize,halfPatchSize);
            int smin = std::max(y-halfWindowSize,halfPatchSize);

            int rmax = std::min(x+halfWindowSize,m+halfPatchSize);
            int smax = std::min(y+halfWindowSize,n+halfPatchSize);

            for (int ii = 0; ii < patchSize; ii++) {
                for (int jj = 0; jj < patchSize; jj++) {
                    denoised[ii][jj] = 0.0;
                }
            }

            for (int r = rmin; r < rmax; r++) {
                for (int s = smin; s < smax; s++) {
                    if (r == x && s == y) {
                        continue;
                    }

                    double dDist = 0;
                    double diff = 0;
                    for (int ii = -halfPatchSize; ii <= halfPatchSize; ii++) {
                        for (int jj = -halfPatchSize; jj <= halfPatchSize; jj++) {
                            diff = increasedImage[x+ii][y+jj]-increasedImage[r+ii][s+jj];
                            dDist += diff*diff;
                        }
                    }

                    dDist = std::max(dDist - 2.0 * (double) icwl *  dSigma2, 0.0);
                    dDist = dDist / dH2;
                    double dWeight = exp(-dDist);

                    if (dWeight > dMaxWeight) {
                        dMaxWeight = dWeight;
                    }

                    dTotalWeight += dWeight;

                    for (int ii = -halfPatchSize; ii <= halfPatchSize; ii++) {
                        int il = ii + halfPatchSize;
                        for (int jj = -halfPatchSize; jj <= halfPatchSize; jj++) {
                            int jl =  jj + halfPatchSize;
                            denoised[il][jl] += dWeight * increasedImage[r+ii][s+jj];
                        }
                    }
                }

                for (int ii = -halfPatchSize; ii <= halfPatchSize; ii++) {
                    int il = ii + halfPatchSize;
                    for (int jj = -halfPatchSize; jj <= halfPatchSize; jj++) {
                        int jl =  jj + halfPatchSize;
                        denoised[il][jl] += dMaxWeight * increasedImage[x+ii][y+jj];
                    }
                }

                dTotalWeight += dMaxWeight;

                if (dTotalWeight > 0.0001) {
                    for (int ii = -halfPatchSize; ii <= halfPatchSize; ii++) {
                        int il = ii + halfPatchSize;
                        for (int jj = -halfPatchSize; jj <= halfPatchSize; jj++) {
                            int jl =  jj + halfPatchSize;
                            dCount[x+ii][y+jj]++;
                            colorOutput[x+ii][y+jj] += denoised[il][jl] / dTotalWeight;
                        }
                    }
                }
            }

        }
        for (int id = 0; id < patchSize; id++) {
            delete[] denoised[id];
        }
        delete []denoised;
    }


    //double2img(colorOutput,incWidth,incHeight);

    //    for (int i = 0; i < incWidth; i++) {
    //        for (int j = 0; j < incHeight; j++) {
    //            if (fpCount[i][j] > 0.0) {
    //                colorOutput[i][j] /= fpCount[i][j];
    //            }
    //        }
    //    }

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            if (dCount[i+halfPatchSize][j+halfPatchSize] > 0.0) {
                colorOutput[i+halfPatchSize][j+halfPatchSize] /=
                        dCount[i+halfPatchSize][j+halfPatchSize];
            } else {
                colorOutput[i+halfPatchSize][j+halfPatchSize] = colorInput[i][j];
            }
        }
    }

    /* CREATE FILTERED IMAGE */
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int gray = colorOutput[i+halfPatchSize][j+halfPatchSize];
            imageFiltered->setPixel(i, j, qRgb(gray, gray, gray));
        }
    }

    /* CLEAR MEMORY */
    for (int i = 0; i < w; i++) {
        delete []colorInput[i];
    }
    for (int i = 0; i < incWidth; i++) {
        delete []colorOutput[i];
        delete []dCount[i];
    }
    delete []colorOutput;
    delete []colorInput;
    delete []dCount;
}
