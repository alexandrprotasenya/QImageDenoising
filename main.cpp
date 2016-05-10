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
#include "nlm_filter.h"

#include "utils.h"
#include "parrot_test.h"

#include <iostream>

using namespace std;

#define TEST

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    parrot_test();
//    test_nlm();

//    system("pause");

    a.exit(0);
    return 0;
}
