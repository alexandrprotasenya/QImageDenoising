#include "nlm_filter_gray.h"
#include "utils.h"
#include <omp.h>
#include <cmath>

void nlm_filter_gray(QImage* imageNoise,
                     QImage* imageFiltered,
                     QSize imageSize,
                     int halfWindowSize,
                     int halfPatchSize,
                     float fSigma,
                     float fParam) {

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
    float** colorInput = new float*[w];
    float** colorOutput = new float*[w];
    for (int i = 0; i < w; i++) {
        colorInput[i] = new float[h];
        colorOutput[i] = new float[h];
    }
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            colorInput[i][j] = qGray(imageNoise->pixel(i,j));
        }
    }

    /* FILTER PARAMETER */
    float fSigma2 = fSigma * fSigma;
    float fH = fParam * fSigma;
    float fH2 = fH * fH;
    float icwl = patchSize * patchSize;
    fH2 *= icwl;

    /* INCREASE IMAGE */
    float** increasedImage = new float*[incWidth];
    for (int i = 0; i < incWidth; i++) {
        increasedImage[i] = new float[incHeight];
    }
    nlm_increse_image2(colorInput, increasedImage, imageSize, halfPatchSize);

    /* FILTER */
    T_START
#pragma omp parallel shared(colorInput, colorOutput)
{
#pragma omp for schedule(dynamic) nowait
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int i1 = i+halfPatchSize;
            int j1 = j+halfPatchSize;

            float wmax = 0;
            float average = 0;
            float sweight = 0;

            int rmin = std::max(i1-halfWindowSize,halfPatchSize);
            int rmax = std::min(i1+halfWindowSize,w+halfPatchSize);
            int smin = std::max(j1-halfWindowSize,halfPatchSize);
            int smax = std::min(j1+halfWindowSize,h+halfPatchSize);

            for (int r = rmin; r < rmax; r++) {
                for (int s = smin; s < smax; s++) {
                    if (r == i1 && s == j1) {
                        continue;
                    }

                    float fDif = 0;
                    float dif = 0;
                    for (int ii = -halfPatchSize; ii <= halfPatchSize; ii++) {
                        for (int jj = -halfPatchSize; jj <= halfPatchSize; jj++) {
                            dif = increasedImage[i1+ii][j1+jj]-increasedImage[r+ii][s+jj];
                            fDif += dif*dif;
                        }
                    }

                    fDif = std::max(fDif - 2.0 * (float) icwl *  fSigma2, 0.0);
                    fDif = fDif / fH2;
                    float W = exp(-fDif);

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
}
    T_END

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
}
