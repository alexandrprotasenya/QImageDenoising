#include "nlm_random_cuda.h"
#include "utils.h"
#include <cuda.h>

#include <iostream>
using namespace std;


#define QUEUE_SIZE 128
#define PATCH_RADIUS 3
#define STEPS 10 // 4
#define RAND_SEARCH_STEPS 25 //25
#define SEARCH_RADIUS 15 //10
#define FPARAM 0.4f


// Forward declare the function in the .cu file
void nlm_filter_random_CUDA(const float* h_src, float* h_dst,
                            int width, int height,
                            float fSigma, float fParam,
                            int patchRadius, int searchRadius,
                            int queueSize, int steps);

void nlm_random_cuda(QImage *imageNoise, QImage *imageFiltered,
                     float fSigma, float fParam,
                     int patchRadius, int searchRadius,
                     int queueSize, int steps)
{
    // SIZES
    patchRadius = PATCH_RADIUS;
    fParam = FPARAM;
    searchRadius = SEARCH_RADIUS;
    steps = STEPS;

//    int patchSize = patchRadius * 2 + 1;
    int width = imageNoise->size().width();
    int height = imageNoise->size().height();
    int incWidth = patchRadius*2 + width;
    int incHeight = patchRadius*2 + height;

    // CREATE ARRAYS
    float** colorInput  = new float*[width];
    float** colorOutput = new float*[width];
    for (int i = 0; i < width; i++) {
        colorInput[i]  = new float[height];
        colorOutput[i] = new float[height];
    }
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            colorInput[i][j] = qGray(imageNoise->pixel(i,j));
        }
    }

    // INCREASE IMAGE
    float** increasedImage = new float*[incWidth];
    for (int i = 0; i < incWidth; i++) {
        increasedImage[i] = new float[incHeight];
    }
    nlm_increse_image2(colorInput, increasedImage, imageNoise->size(), patchRadius);

    // Creating arrays for processing on device
    float* h_input  = new float[incHeight*incWidth];
    float* h_output = new float[incHeight*incWidth];

    for (int i = 0; i < incWidth; i++) {
        for (int j = 0; j < incHeight; j++) {
            h_output[incWidth*j+i] = 0;
            h_input [incWidth*j+i] = increasedImage[i][j];
        }
    }

    nlm_filter_random_CUDA(h_input, h_output, incWidth, incHeight, fSigma,
                           fParam, patchRadius, searchRadius, queueSize, steps);

    // CREATE FILTERED IMAGE
    for (int i = 0; i < incWidth; i++) {
        for (int j = 0; j < incHeight; j++) {
            increasedImage[i][j] = h_output[incWidth*j+i];
        }
    }
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int gray = increasedImage[i+patchRadius][j+patchRadius];
            imageFiltered->setPixel(i, j, qRgb(gray, gray, gray));
        }
    }

    /* CLEAR MEMORY */
    for (int i = 0; i < width; i++) {
        delete []colorInput[i];
        delete []colorOutput[i];
    }
    for (int i = 0; i < incWidth; ++i) {
        delete []increasedImage[i];
    }
    delete []colorOutput;
    delete []colorInput;
    delete []increasedImage;

}
