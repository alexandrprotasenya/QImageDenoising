#ifndef NLMDENOIS_H
#define NLMDENOIS_H

#include <QImage>

class NLMDenois
{
private:
    double* dSrc;
    double* dDst;

    double dParam;
    double dSigma;
    double dSigma2;
    int iWin;
    int iBlock;
    int iWidth;
    int iHeight;

    int iChannels;
    int iwxh;
    int ihwl;
    int iwl;
    int icwl;

    double dH2;

    double** dCount;
    double** denoised;

    static void dClear(double* array, double n, int size);
    void getArrayFromImage(QImage* src);
public:
    NLMDenois(QImage* src, double dSigma);
    NLMDenois(QImage* src, double dSigma, int iWin, int iBlock, double dParam);

    void init(QImage* src, double dSigma, int iWin, int iBlock, double dParam);

    void denoise();
    QImage getImage();

    ~NLMDenois();
};

#endif // NLMDENOIS_H
