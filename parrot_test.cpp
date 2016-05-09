#include "parrot_test.h"

#include <iostream>

using namespace std;

#define NOISE 25

void parrot_test() {
    // Open ideal
    QImageReader* imageIdealReader = new QImageReader("Parrot_ideal.png");
    QImage* imageIdeal = new QImage();
    imageIdealReader->read(imageIdeal);

    QImageReader* imageIdealFiltReader = new QImageReader("Parrot_ideal1.png");
    QImage* imageIdealFilt = new QImage();
    imageIdealFiltReader->read(imageIdealFilt);

    // Open noise
    QImageReader* imageNoiseReader = new QImageReader("Parrot_WGN_stdev_25.png");
    QImage* imageNoise = new QImage();
    imageNoiseReader->read(imageNoise);

//    QImage* imageFiltered0 = new QImage(imageNoise->size(), QImage::Format_RGB32);
    QImage* imageFiltered = new QImage(imageNoise->size(), QImage::Format_RGB32);
    // Filter
    // 5 3 46
    QDateTime mStartTime = QDateTime::currentDateTime();
//    nlm_filter_gray(imageNoise, imageFiltered, imageNoise->size(), 8, 1, 25, 0.9f);
    nlm_filter_cuda(imageNoise, imageFiltered, imageNoise->size(), 8, 1, 25, 0.9f);
//    china_denoise(imageNoise, imageFiltered, 25, 11);
    QDateTime mFinishTime = QDateTime::currentDateTime();
    qDebug() << QDateTime::fromMSecsSinceEpoch(mFinishTime.toMSecsSinceEpoch() - mStartTime.toMSecsSinceEpoch()).time();
    diff_images(imageNoise,imageFiltered);

    // Compare
    double mse_ideal_noise;
    double psnr_ideal_noise;
    calcMsePsnr(&mse_ideal_noise, &psnr_ideal_noise, imageIdeal, imageNoise, imageIdeal->size());
    double mse_ideal_filtered;
    double psnr_ideal_filtered;
    calcMsePsnr(&mse_ideal_filtered, &psnr_ideal_filtered, imageIdeal, imageFiltered, imageIdeal->size());
    double mse_ideal_filt_filtered;
    double psnr_ideal_filt_filtered;
    calcMsePsnr(&mse_ideal_filt_filtered, &psnr_ideal_filt_filtered, imageIdeal, imageIdealFilt, imageIdeal->size());

    // Print result
    cout << endl
         << "Noise MSE: \t" << mse_ideal_noise << endl
         << "Noise PSNR: \t" << psnr_ideal_noise << endl
         << "Filtered MSE:\t" << mse_ideal_filtered << endl
         << "Filtered PSNR:\t" << psnr_ideal_filtered << endl << endl
         << "Filtered 2 MSE:\t" << mse_ideal_filt_filtered << endl
         << "Filtered 2 PSNR:\t" << psnr_ideal_filt_filtered << endl;

    // Save result
    QImageWriter* imageWriterFiltered = new QImageWriter();
    imageWriterFiltered->setFileName("Parrot_filtered.png");
    imageWriterFiltered->write(*imageFiltered);

    // Clean memory
    delete imageIdealReader;
    delete imageFiltered;
    delete imageNoiseReader;
    delete imageWriterFiltered;
    delete imageIdealFilt;
    delete imageIdealFiltReader;
}
