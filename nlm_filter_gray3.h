#ifndef NLM_FILTER_GRAY3_H
#define NLM_FILTER_GRAY3_H

#include <QImage>
#include <QSize>

void nlm_filter_gray3(QImage* imageNoise,
                        QImage *imageFiltered,
                        QSize imageSize,
                        int halfWindowSize,
                        int halfPatchSize,
                        double fSigma, double fParam);

#endif // NLM_FILTER_GRAY3_H
