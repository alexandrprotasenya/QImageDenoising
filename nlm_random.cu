#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <stdio.h>

#include <algorithm>
#include <vector>

#define BLOCKDIM_X 32
#define BLOCKDIM_Y 32

int iDivUp2(int a, int b)
{
    return ((a % b) != 0) ? (a / b + 1) : (a / b);
}

struct PatchDist {
    int iX;
    int iY;
    double fDist;
};

bool compareDist(PatchDist d1, PatchDist d2) {
    return d1.fDist < d2.fDist;
}

class PriorityQueue {
public:
    PriorityQueue() {
    }

    int size() {
        return pqDist.size();
    }

    void add(int x, int y, double dist) {
        PatchDist pd;
        pd.iX = x;
        pd.iY = y;
        pd.fDist = dist;
        if (pqDist.size() == 0 || pqDist.size() <= ss) {
            pqDist.push_back(pd);
            std::sort(pqDist.begin(), pqDist.end(), compareDist);
        } else {
            PatchDist pqLast = pqDist.at(pqDist.size()-1);
            if (pqDist.size() < ss || pqLast.fDist > dist) {
                pqDist.pop_back();
                pqDist.push_back(pd);
                std::sort(pqDist.begin(), pqDist.end(), compareDist);
            }
        }
    }

    PatchDist get(int pos) {
        return pqDist.at(pos);
    }

    void clear() {
        pqDist.clear();
    }

private:
    int ss = 11;
    std::vector<PatchDist> pqDist;
};


__device__ int2 randPoint(int cx, int cy, int sigma, int i) {
    curandState randState;
    unsigned int seed = (unsigned int) clock64();
    curand_init(seed , 0, 0, &randState);
    float2 fpoint;
    fpoint = curand_normal2(&randState);
    double k = 1;
    if (i != 0)
        k  = pow(0.5, i);
    int2 ipoint;
    ipoint.x = fpoint.x*100.0f;//cx + int(sigma * k * fpoint.x);
    ipoint.y = fpoint.y*100.0f;//cy + int(sigma * k * fpoint.y);
    return ipoint;
}

__global__ void getRand(int2 *rnd) {
    int2 r = randPoint(5, 3, 30, 0);
    rnd->x = r.x;
    rnd->y = r.y;
}

__global__ void initQueue(const float *d_src, int width, int height,
                          int patchRadius, int searchRadius, int queueSize) {

}

__global__ void nlm_filter_random_global(const float *d_src, float* d_dst, int width, int height,
                                         float fSigma, float fParam, int patchRadius,
                                         int searchRadius, int queueSize, int steps) {
    const int ix = blockDim.x * blockIdx.x + threadIdx.x;
    const int iy = blockDim.y * blockIdx.y + threadIdx.y;
    if (ix < width && iy < height)
    {
        int i1 = ix+patchRadius;
        int j1 = iy+patchRadius;

        d_dst[width*j1 + i1] += d_src[width*j1 + i1]/(float)steps;
    }
}

void nlm_filter_random_CUDA(const float* h_src, float* h_dst,
                            int width, int height,
                            float fSigma, float fParam,
                            int patchRadius, int searchRadius,
                            int queueSize, int steps) {
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
    dim3 grid(iDivUp2(width, BLOCKDIM_X), iDivUp2(height, BLOCKDIM_Y));

    int2 *p, *h_p;
    h_p = (int2*) malloc(sizeof(int2));
    cudaMalloc((void **)& p, sizeof(int2));

    for (int i = 0; i < steps; ++i) {
        getRand<<<1, 1>>>(p);

        cudaMemcpy(h_p, p, sizeof(int2), cudaMemcpyDeviceToHost);
        printf("%f\t%f\n", h_p->x, h_p->y);

        initQueue<<<grid, threads>>>(d_src, width, height, patchRadius, searchRadius, queueSize);
        nlm_filter_random_global<<<grid, threads>>>(d_src, d_dst, width, height,
                                                    fSigma, fParam, patchRadius, searchRadius, queueSize, steps);
    }

    err = cudaGetLastError();
    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to launch nlm_random_device kernel (error code %s)!\n", cudaGetErrorString(err));
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
