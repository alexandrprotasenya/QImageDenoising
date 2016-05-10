#include <cuda_runtime.h>
#include <stdio.h>

#include <algorithm>
#include <vector>

#define BLOCKDIM_X 8
#define BLOCKDIM_Y 8

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

void nlm_filter_random_CUDA(const float* h_src, float* h_dst, int w, int h, float fSigma, float fParam, int patch, int window) {
    cudaError_t err = cudaSuccess;
}
