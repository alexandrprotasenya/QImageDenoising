#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#include <QImage>
#include <QSize>

void median_filter(QImage* image, QImage* imageFiltered, QSize size);

#endif // MEDIAN_FILTER_H
