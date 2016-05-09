#ifndef CHINA_DENOISE_H
#define CHINA_DENOISE_H

#include <QImage>
#include <QSize>

void china_denoise(QImage* input, QImage* output, double dSigma, int iK);

#endif // CHINA_DENOISE_H
