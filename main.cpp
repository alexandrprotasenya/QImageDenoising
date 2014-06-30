#include <QCoreApplication>
#include <QImageReader>
#include <QImageWriter>
#include <QRgb>
#include <QColor>
#include <QTime>
#include <QDebug>

//#include "awgn.h"
#include "noise.h"
//#include "blur.h"
#include "median_filter.h"
//#include "nlm_filter.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QImageWriter* imageWriterNoise = new QImageWriter();
    imageWriterNoise->setFileName("Lenna_AWGN.png");

    QImageWriter* imageWriterFiltered = new QImageWriter();
    imageWriterFiltered->setFileName("Lenna_MedianFilter.png");

    QImageReader* imageReader = new QImageReader("Lenna.png");

    if (imageReader->canRead()) {
        QSize size = imageReader->size();
        qDebug() << "Size:" << imageReader->size();

        QImage* image = new QImage();
        imageReader->read(image);

        QImage* imageNoise = addAWGN(image, size, 15);
        imageWriterNoise->write(*imageNoise);

        QImage* imageFiltered = median_filter(imageNoise, size);
        imageWriterFiltered->write(*imageFiltered);

        delete imageFiltered;
        delete imageNoise;
        delete image;
    }

    delete imageWriterNoise;
    delete imageReader;

    a.exit(0);
    return 0;
}
