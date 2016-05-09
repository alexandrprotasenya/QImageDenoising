#ifndef UTILS
#define UTILS

#include <QImage>
#include <QSize>

#include <QDebug>
#include <QDateTime>

#define T_START QDateTime mStartTime = QDateTime::currentDateTime();
#define T_END QDateTime mFinishTime = QDateTime::currentDateTime();qDebug() << "LNM" << QDateTime::fromMSecsSinceEpoch(mFinishTime.toMSecsSinceEpoch() - mStartTime.toMSecsSinceEpoch()).time();

void calcMsePsnr(double* mse, double* psnr, QImage *image1, QImage *image2, QSize size);

void nlm_increse_image(QImage *, QImage *, QSize, int);
void nlm_increse_image(double ** src, double ** dst, QSize srcImageSize, int inc);
void nlm_increse_image2(double ** src, double ** dst, QSize srcImageSize, int inc);
void nlm_increse_image2(float ** src, float ** dst, QSize srcImageSize, int inc);
void make_kernel(double** gKernel, int kernel_size, double sigma);

#endif // UTILS

