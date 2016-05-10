#include "nlm_random_cuda.h"
#include "utils.h"
#include <cuda.h>


// Forward declare the function in the .cu file
void nlm_filter_random_CUDA(const float* h_src, float* h_dst, int w, int h, float fSigma, float fParam, int patch, int window);

void nlm_random_cuda(QImage *imageNoise, QImage *imageFiltered, QSize imageSize, int halfWindowSize, int halfPatchSize, float fSigma, float fParam)
{

}
