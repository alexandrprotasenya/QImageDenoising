#ifndef DIFFIMAGES_H
#define DIFFIMAGES_H

#include <QImage>
#include <QSize>
#include <QImageReader>
#include <QImageWriter>

void diff_images(QImage* image1, QImage* image2, QString fileName = "diff.png");

#endif // DIFFIMAGES_H
