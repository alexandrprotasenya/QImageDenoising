#include "diffimages.h"


void diff_images(QImage *image1, QImage *image2, QString fileName)
{
    // TODO: Check images size

    QImage* image3 = new QImage(image1->size(), QImage::Format_RGB32);

    for (int i = 0; i < image1->size().width(); i++) {
        for (int j = 0; j < image1->size().height(); j++) {
            int p1 = qGray(image1->pixel(i,j));
            int p2 = qGray(image2->pixel(i,j));
            int p3 = p1-p2;
            double fSigma = 0.4 * 25.0;
            double diff = (static_cast<double>(p3)+ fSigma) * 255.0 / (2.0 * fSigma);
            if (diff < 0.0) diff = 0;
            if (diff > 255.5) diff = 255;
            p3 = diff;
            image3->setPixel(i,j,qRgb(p3,p3,p3));
        }
    }

    QImageWriter* imageDiff = new QImageWriter();
    imageDiff->setFileName(fileName);
    imageDiff->write(*image3);

    delete image3;
    delete imageDiff;
}
