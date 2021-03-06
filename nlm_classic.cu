#include <cuda_runtime.h>
#include <stdio.h>

#define BLOCKDIM_X 32
#define BLOCKDIM_Y 32

__device__ float Max(float x, float y)
{
    return (x > y) ? x : y;
}

__device__ float Min(float x, float y)
{
    return (x < y) ? x : y;
}

__device__ int Max(int x, int y)
{
    return (x > y) ? x : y;
}

__device__ int Min(int x, int y)
{
    return (x < y) ? x : y;
}

int iDivUp(int a, int b)
{
    return ((a % b) != 0) ? (a / b + 1) : (a / b);
}

__global__ void nlm_classic_global(const float* d_src,
                                   float* d_dst,
                                   int patch, int window,
                                   int width, int height, float fSigma2, float fH2, float icwl) {

    const int ix = blockDim.x * blockIdx.x + threadIdx.x;
    const int iy = blockDim.y * blockIdx.y + threadIdx.y;
    if (ix < width && iy < height)
    {
        int i1 = ix+patch;
        int j1 = iy+patch;

        float wmax = 0;
        float average = 0;
        float sweight = 0;

        int rmin = Max(i1-window,patch+1);
        int rmax = Min(i1+window,width+patch);
        int smin = Max(j1-window,patch+1);
        int smax = Min(j1+window,height+patch);

        for (int r = rmin; r < rmax; r++) {
            for (int s = smin; s < smax; s++) {
                if (r == i1 && s == j1) {
                    continue;
                }
                float diff = 0;
                for (int ii = -patch; ii <= patch; ii++) {
                    for (int jj = -patch; jj <= patch; jj++) {
                        float a = d_src[width*(j1+jj)+(i1+ii)];
                        float b = d_src[width*(s+jj)+(r+ii)];
                        float c = a-b;
                        diff += c*c;
                    }
                }
                diff = Max(float(diff - 2.0 * (double) icwl *  fSigma2), 0.0f);
                diff = diff / fH2;
                float W = __expf(-diff);

                if (W > wmax) {
                    wmax = W;
                }

                sweight += W;
                average += W * d_src[width*s + r];
            }
        }
        average += wmax * d_src[width*j1+i1];
        sweight += wmax;

        if (sweight > 0) {
            d_dst[width*j1+i1] = average / sweight;
        }
        else {
            d_dst[width*j1+i1] = d_src[width*j1+i1];
        }
    }
}

void nlm_filter_classic_CUDA(const float* h_src, float* h_dst, int width, int height, float fSigma, float fParam, int patch, int window) {
    cudaError_t err = cudaSuccess;

    float* d_src = NULL, *d_dst = NULL;
    unsigned int nBytes = sizeof(float) * (width*height);

    err = cudaMalloc((void **)& d_src, nBytes);
    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to allocate device vector SRC (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaMalloc((void **)& d_dst, nBytes);
    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to allocate device vector DST (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaMemcpy(d_src, h_src, nBytes, cudaMemcpyHostToDevice);
    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to copy vector SRC from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaMemcpy(d_dst, h_dst, nBytes, cudaMemcpyHostToDevice);
    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to copy vector DST from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
    dim3 grid(iDivUp(width, BLOCKDIM_X), iDivUp(height, BLOCKDIM_Y));

    int patchSize = patch*2+1;
    float fSigma2 = fSigma * fSigma;
    float fH = fParam * fSigma;
    float fH2 = fH * fH;
    float icwl = patchSize * patchSize;
    fH2 *= icwl;

    nlm_classic_global<<<grid, threads>>>(d_src, d_dst, patch, window, width, height, fSigma2, fH2, icwl);

    err = cudaGetLastError();
    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to launch nlm_classic_device kernel (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // load the answer back into the host
    err = cudaMemcpy(h_dst, d_dst, nBytes, cudaMemcpyDeviceToHost);
    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to copy vector DST from device to host (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    cudaFree(d_src);
    cudaFree(d_dst);
}
