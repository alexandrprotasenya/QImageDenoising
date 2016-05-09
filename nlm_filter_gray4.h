#ifndef NLM_FILTER_GRAY4_H
#define NLM_FILTER_GRAY4_H

#include <QImage>
#include <QSize>

void nlm_filter_gray4(QImage* imageNoise,
                        QImage *imageFiltered,
                        QSize imageSize,
                        int iWin,
                        int iPatch,
                        float fSigma, float fParam);

#endif // NLM_FILTER_GRAY3_H
