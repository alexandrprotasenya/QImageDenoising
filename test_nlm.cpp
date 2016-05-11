#include "parrot_test.h"
#include "noise.h"

#include <ctime>

#include <iostream>
using namespace std;

#define NOISE 20

void test_nlm() {
    srand(time(0));
    QString sourceName = "Parrot";
    QString ext = ".png";

    double mse_filtered;
    double psnr_filtered;
    double mse_noise;
    double psnr_noise;

    QImageReader* imageIdealReader = new QImageReader(sourceName+ext);
    QImage* imageIdeal = new QImage();
    imageIdealReader->read(imageIdeal);

/*    QImage* imageFiltered1 = new QImage(imageIdeal->size(), QImage::Format_RGB32);
    QImage* imageNoise1 = addAWGN_GRAY(imageIdeal, imageIdeal->size(),(double)NOISE);

    QDateTime mStartTime = QDateTime::currentDateTime();

//    nlm_filter_gray(imageNoise1,imageFiltered1,imageNoise1->size(),2,10,NOISE,0.4);
    china_denoise(imageNoise1, imageFiltered1, NOISE, 11);

    QDateTime mFinishTime = QDateTime::currentDateTime();
    qDebug() << QDateTime::fromMSecsSinceEpoch(mFinishTime.toMSecsSinceEpoch() - mStartTime.toMSecsSinceEpoch()).time();

    diff_images(imageNoise1,imageFiltered1);

    calcMsePsnr(&mse_filtered, &psnr_filtered, imageIdeal, imageFiltered1, imageIdeal->size());
    calcMsePsnr(&mse_noise, &psnr_noise, imageIdeal, imageNoise1, imageIdeal->size());

    cout << "NOISE" << "\t";
    cout << "mse_n" << "\t" << "psnr_n" << "\t";
    cout << "mse_f" << "\t" << "psnr_f" << endl;

    cout << NOISE << "\t";
    cout << mse_noise << "\t" << psnr_noise << "\t";
    cout << mse_filtered<< "\t" << psnr_filtered << endl;

    QImageWriter* imageWriterNoise = new QImageWriter();
    imageWriterNoise->setFileName(sourceName+"_noise"+ext);
    imageWriterNoise->write(*imageNoise1);

    QImageWriter* imageWriterFiltered = new QImageWriter();
    imageWriterFiltered->setFileName(sourceName+"_filtered"+ext);
    imageWriterFiltered->write(*imageFiltered1);*/

    cout << "SIGMA\tMSE*\tPSNR*\tMSE\tPSNR" << endl;
    for (int i = 5; i <= 50; i+=3) {
        double avg_mse = 0;
        double avg_psnr = 0;
        double avgmn = 0;
        double avgpn = 0;
        int N = 1;
        for (int j = 0; j < N; j++) {
            QImage* imageNoise = addAWGN_GRAY(imageIdeal, imageIdeal->size(),(double)i);
            QImage* imageFiltered = new QImage(imageNoise->size(), QImage::Format_RGB32);
            QImage* imageFiltered2 = new QImage(imageNoise->size(), QImage::Format_RGB32);

            nlm_filter_gray(imageNoise,imageFiltered,imageNoise->size(),2,10,i,0.4f);
            nlm_filter_cuda(imageNoise, imageFiltered2, imageNoise->size(), 2, 10, i, 0.4f);
//            china_denoise(imageNoise, imageFiltered, (double)i, 11);
            double mse0, mse1, psnr0, psnr1;
            calcMsePsnr(&mse0, &psnr0, imageIdeal, imageFiltered, imageIdeal->size());
            calcMsePsnr(&mse1, &psnr1, imageIdeal, imageFiltered2, imageIdeal->size());
            printf("\t\tCPU: \t%f \t%f\n\t\tGPU \t%f \t%f\n", mse0, psnr0, mse1, psnr1);calcMsePsnr(&mse0, &psnr0, imageIdeal, imageFiltered, imageIdeal->size());

            QImageWriter* imageWriterFiltered = new QImageWriter();
            imageWriterFiltered->setFileName("filtered/filtered_" + sourceName+"_filtered_" + QString::number(i) +ext);
            imageWriterFiltered->write(*imageFiltered);

            diff_images(imageNoise,imageFiltered, QString("diff/diff_") + QString::number(i) + QString(".png"));

            calcMsePsnr(&mse_filtered, &psnr_filtered, imageIdeal, imageFiltered, imageIdeal->size());
            calcMsePsnr(&mse_noise, &psnr_noise, imageIdeal, imageNoise, imageIdeal->size());

            avg_mse += mse_filtered;
            avg_psnr += psnr_filtered;

            avgmn += mse_noise;
            avgpn += psnr_noise;

            delete imageNoise;
            delete imageFiltered;
            delete imageFiltered2;
        }
        //cout << i << "\t" << avg_mse/N << "\t" << avg_psnr/N << endl;
        cout << i << "\t";
        cout << avgmn/N << "\t" << avgpn/N << "\t";
        cout << avg_mse/N<< "\t" << avg_psnr/N << endl;
    }
}
