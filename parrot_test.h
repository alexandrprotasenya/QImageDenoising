#ifndef PARROT_TEST
#define PARROT_TEST

#include <QImage>
#include <QSize>
#include <QImageReader>
#include <QImageWriter>
#include <QDebug>
#include <QDateTime>

#include "median_filter.h"
#include "nlm_filter_gray.h"
#include "nlm_filter_gray2.h"
#include "nlm_filter_gray3.h"
#include "nlm_filter_cuda.h"
#include "nlm_random_cuda.h"
#include "china_denoise.h"
#include "utils.h"
#include "diffimages.h"

void parrot_test();
void test_nlm();

#endif // PARROT_TEST

