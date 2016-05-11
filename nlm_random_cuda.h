#ifndef NLM_RANDOM_CUDA_H
#define NLM_RANDOM_CUDA_H

#include <QImage>
#include <QSize>

void nlm_random_cuda(QImage *imageNoise, QImage *imageFiltered,
                     float fSigma, float fParam,
                     int patchRadius, int searchRadius,
                     int queueSize, int steps = 4);

#endif // NLM_RANDOM_CUDA_H
