#include "noise.h"


QImage* addAWGN(QImage* image, QSize size, double std) {
    QImage* imageNoise = new QImage(size, QImage::Format_RGB32);
    for (int i = 0; i < size.width(); i++) {
        for (int j = 0; j < size.height(); j++) {
            QRgb p = image->pixel(i, j);
            int r = qRed(p);
            int g = qGreen(p);
            int b = qBlue(p);

            double rn = r+AWGN_generator(std);
            double gn = g+AWGN_generator(std);
            double bn = b+AWGN_generator(std);

            if (rn > 255) {
                rn = 255;
            } else if (rn < 0) {
                rn = 0;
            }

            if (gn > 255) {
                gn = 255;
            } else if (gn < 0) {
                gn = 0;
            }

            if (bn > 255) {
                bn = 255;
            } else if (bn < 0) {
                bn = 0;
            }

            QRgb n = qRgb(rn, gn, bn);

            imageNoise->setPixel(i, j, n);
        }
    }
    return imageNoise;
}
