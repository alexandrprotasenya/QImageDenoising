#include "nlm_filter_gray3.h"
#include "utils.h"
#include <cmath>

#include <QImageWriter>

void debug_float2img(float** src, int w, int h) {
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

void nlm_filter_gray4(QImage* imageNoise,
                      QImage* imageFiltered,
                      QSize imageSize,
                      int iWin,
                      int iPatch,
                      float fSigma,
                      float fParam) {
    if (fSigma > 0.0f && fSigma <= 15.0f) {
        iPatch = 1;
        iWin = 10;
        fParam = 0.4f;
    } else if ( fSigma > 15.0f && fSigma <= 30.0f) {
        iPatch = 2;
        iWin = 10;
        fParam = 0.4f;
    } else if ( fSigma > 30.0f && fSigma <= 45.0f) {
        iPatch = 3;
        iWin = 17;
        fParam = 0.35f;
    } else if ( fSigma > 45.0f && fSigma <= 75.0f) {
        iPatch = 4;
        iWin = 17;
        fParam = 0.35f;
    } else if (fSigma <= 100.0f) {
        iPatch = 5;
        iWin = 17;
        fParam = 0.30f;
    }


        iWin = 5;
        iPatch = 1;
//        fParam = 0.3f;

    /* SIZES */
    int patchSize = iPatch * 2 + 1;
    //    int winSize = halfWindowSize * 2 + 1;
    int m = imageSize.width();
    int n = imageSize.height();
    int w = m;
    int h = n;
    //    int incWidth = patchSize - 1 + w;
    //    int incHeight = patchSize - 1 + h;

    /* CREATE ARRAYS */
    float** colorInput = new float*[w];
    for (int i = 0; i < w; i++) {
        colorInput[i] = new float[h];
    }
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            colorInput[i][j] = qGray(imageNoise->pixel(i,j));
        }
    }
    float** dCount = new float*[w];
    float** colorOutput = new float*[w];
    for (int i = 0; i < w; i++) {
        colorOutput[i] = new float[h];
        dCount[i] = new float[h];
    }
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++) {
            dCount[i][j] = 0.0;
            colorOutput[i][j] = 0.0;
        }

    /* FILTER PARAMETER */
    float dSigma2 = fSigma * fSigma;
    float dH = fParam * fSigma;
    float dH2 = dH * dH;
    int icwl = 1 * patchSize * patchSize;
    dH2 *= (float)icwl;

    /* FILTER */
    for (int x = 0; x < w; x++) {
        float** denoised = new float*[patchSize];
        for (int id = 0; id < patchSize; id++) {
            denoised[id] = new float[patchSize];
        }
        for (int y = 0; y < h; y++) {

            float dMaxWeight = 0.0;
            float dTotalWeight = 0.0;

            int iPatch0 = std::min(iPatch,std::min(w-1-x,std::min(h-1-y,std::min(x,y))));

            int rmin = std::max(x-iWin,iPatch0);
            int smin = std::max(y-iWin,iPatch0);

            int rmax = std::min(x+iWin,w-1-iPatch0);
            int smax = std::min(y+iWin,h-1-iPatch0);

            for (int ii = 0; ii < patchSize; ii++) {
                for (int jj = 0; jj < patchSize; jj++) {
                    denoised[ii][jj] = 0.0;
                }
            }

            for (int r = rmin; r < rmax; r++) {
                for (int s = smin; s < smax; s++) {
                    if (r != x || s != y) {

                        float dDist = 0;
                        float diff = 0;
                        for (int ii = -iPatch0; ii <= iPatch0; ii++) {
                            for (int jj = -iPatch0; jj <= iPatch0; jj++) {
                                diff = colorInput[x+ii][y+jj]-colorInput[r+ii][s+jj];
                                dDist += diff*diff;
                            }
                        }

                        dDist = std::max(dDist - 2.0 * (float) icwl *  dSigma2, 0.0);
                        dDist = dDist / dH2;
                        float dWeight = expf(-dDist);

                        if (dWeight > dMaxWeight) {
                            dMaxWeight = dWeight;
                        }

                        dTotalWeight += dWeight;

                        for (int ii = -iPatch0; ii <= iPatch0; ii++) {
                            int il = ii + iPatch0;
                            for (int jj = -iPatch0; jj <= iPatch0; jj++) {
                                int jl =  jj + iPatch0;
                                denoised[il][jl] += dWeight * colorInput[r+ii][s+jj];
                            }
                        }
                    }
                }

                for (int ii = -iPatch0; ii <= iPatch0; ii++) {
                    int il = ii + iPatch0;
                    for (int jj = -iPatch0; jj <= iPatch0; jj++) {
                        int jl =  jj + iPatch0;
                        denoised[il][jl] += dMaxWeight * colorInput[x+ii][y+jj];
                    }
                }

                dTotalWeight += dMaxWeight;

                if (dTotalWeight > 0.0001) {
                    for (int ii = -iPatch0; ii <= iPatch0; ii++) {
                        int il = ii + iPatch0;
                        for (int jj = -iPatch0; jj <= iPatch0; jj++) {
                            int jl =  jj + iPatch0;
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


    //float2img(colorOutput,incWidth,incHeight);

    //    for (int i = 0; i < incWidth; i++) {
    //        for (int j = 0; j < incHeight; j++) {
    //            if (fpCount[i][j] > 0.0) {
    //                colorOutput[i][j] /= fpCount[i][j];
    //            }
    //        }
    //    }

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            if (dCount[i][j] > 0.0) {
                colorOutput[i][j] /= dCount[i][j];
            } else {
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
    }
    for (int i = 0; i < w; i++) {
        delete []colorOutput[i];
        delete []dCount[i];
    }
    delete []colorOutput;
    delete []colorInput;
    delete []dCount;
}
