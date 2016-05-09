#ifndef NOISE_H
#define NOISE_H

#include <QImage>
#include <QSize>

#include "awgn.h"

QImage* addAWGN_GRAY(QImage*, QSize, double);
QImage* addAWGN(QImage*, QSize, double);

#endif // NOISE_H
