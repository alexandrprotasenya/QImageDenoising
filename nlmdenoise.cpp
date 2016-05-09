#include "nlmdenoise.h"

static void NLMDenois::dClear(double *array, double n, int size)
{
    for (int i = 0; i < size; i++) {
        array[i] = n;
    }
}

void NLMDenois::getArrayFromImage(QImage *src)
{
//    int d_w = src->width();
//    int d_h = src->height();

//    int d_c = src->isGrayscale()?1:3;

//    int d_wh = d_w * d_h;
//    int d_whc = d_c * d_w * d_h;
    dSrc = new double[iWidth];
    dDst = new double[iWidth];
}

NLMDenois::NLMDenois(QImage *src, double dSigma)
{
    int iWin = 1;
    int iBlock = 3;
    double dParam = 0.4;
    if (dSigma > 0.0f && dSigma <= 15.0f) {
        iWin = 1;
        iBlock = 10;
        dParam = 0.4f;
    } else if ( dSigma > 15.0f && dSigma <= 30.0f) {
        iWin = 2;
        iBlock = 10;
        dParam = 0.4f;
    } else if ( dSigma > 30.0f && dSigma <= 45.0f) {
        iWin = 3;
        iBlock = 17;
        dParam = 0.35f;
    } else if ( dSigma > 45.0f && dSigma <= 75.0f) {
        iWin = 4;
        iBlock = 17;
        dParam = 0.35f;
    } else if (dSigma <= 100.0f) {
        iWin = 5;
        iBlock = 17;
        dParam = 0.30f;
    }
    init(src, dSigma, iWin, iBlock, dParam);
}

NLMDenois::NLMDenois(QImage *src, double dSigma, int iWin, int iBlock, double dParam)
{
    init(src, dSigma, iWin, iBlock, dParam);
}

void NLMDenois::init(QImage *src, double dSigma, int iWin, int iBlock, double dParam)
{
    this->iWin = iWin;
    this->iBlock = iBlock;
    this->dParam = dParam;
    this->dSigma = dSigma;

    this->iWidth = src->width();
    this->iHeight = src->height();

    if (src->isGrayscale()) {
        iChannels = 1;
    } else {
        iChannels = 3;
    }

    iwxh = iWidth * iHeight;

    getArrayFromImage(src);


    //  length of comparison window
    ihwl = (2*this->iWin+1);
    iwl = (2*this->iWin+1) * (2*this->iWin+1);
    icwl = iChannels * iwl;

    dSigma2 = dSigma * dSigma;
    float fH = dParam * dSigma;
    dH2 = fH * fH;

    // multiply by size of patch, since distances are not normalized
    dH2 *= (double) icwl;

    dCount = new double[iwxh];

}

void NLMDenois::denoise()
{

}

QImage NLMDenois::getImage()
{

}

NLMDenois::~NLMDenois()
{

}
