#include "median_filter.h"

#include <stdlib.h>

int comp (const int *i, const int *j) {
    return *i - *j;
}

QImage* median_filter(QImage* image, QSize size) {
    QImage* imageFiltered = new QImage(size, QImage::Format_RGB32);

    for (int i = 0; i < size.width(); i++) {
        for (int j = 0; j < size.height(); j++) {
            imageFiltered->setPixel(i, j, image->pixel(i, j));
        }
    }

    for (int i = 1; i < size.width()-1; i++) {
        for (int j = 1; j < size.height()-1; j++) {
            QImage* tmpImage = new QImage(3, 3, QImage::Format_RGB32);
            for (int n = 0; n < 3; n++) {
                for (int m = 0; m < 3; m++) {
                    tmpImage->setPixel(n, m, imageFiltered->pixel(i+n-1, j+m-1));
                }
            }
            int* red = new int[9];
            int* green = new int[9];
            int* blue = new int[9];

            int k = 0;

            for (int n = 0; n < 3; n++) {
                for (int m = 0; m < 3; m++) {
                    QRgb p = tmpImage->pixel(n, m);
                    red[k] = qRed(p);
                    green[k] = qGreen(p);
                    blue[k] = qBlue(p);
                    k++;
                }
            }
            qsort(red, 9, sizeof(int), (int(*) (const void *, const void *)) comp);
            qsort(green, 9, sizeof(int), (int(*) (const void *, const void *)) comp);
            qsort(blue, 9, sizeof(int), (int(*) (const void *, const void *)) comp);

            QRgb np = qRgb(red[4], green[4], blue[4]);

            imageFiltered->setPixel(i, j, np);

            delete red;
            delete green;
            delete blue;
            delete tmpImage;
        }
    }
    return imageFiltered;
}
