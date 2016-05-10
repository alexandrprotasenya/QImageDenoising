#ifndef NLM_RANDOM_CUDA_H
#define NLM_RANDOM_CUDA_H

#include <QImage>
#include <QSize>

void nlm_filter_cuda(QImage* imageNoise,
                     QImage *imageFiltered,
                     QSize imageSize,
                     int halfWindowSize,
                     int halfPatchSize,
                     float fSigma, float fParam);

#endif // NLM_RANDOM_CUDA_H
