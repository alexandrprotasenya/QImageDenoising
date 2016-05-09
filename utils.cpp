#include "utils.h"

#define _USE_MATH_DEFINES

#include <math.h>
#include <iostream>
#include <algorithm>

using namespace std;

#define TEST(X) cout << X << endl;

void calcMsePsnr(double* mse, double* psnr, QImage *image1, QImage *image2, QSize size) {
    double P = 0;
    for (int i = 0; i < size.height(); i++) {
        for (int j = 0; j < size.width(); j++) {
            int pixel1 = qGray(image1->pixel(i,j));
            int pixel2 = qGray(image2->pixel(i,j));
            double SQ = pixel1-pixel2;
            P += SQ*SQ;
        }
    }
    P /= size.width()*size.height();
    *mse = sqrt(P);
    *psnr = 10 * log10(65025./((*mse) * (*mse)));

}

void make_kernel(double** gKernel, int kernel_size, double sigma) {
    double r, s = 2.0 * sigma * sigma;

    // sum is for normalization
    double sum = 0.0;

    int size = kernel_size / 2;

    int dd = kernel_size%2?0:1;

    // generate kernel
    for (int x = -size; x <= size-dd; x++)
    {
        for(int y = -size; y <= size-dd; y++)
        {
            r = sqrt(x*x + y*y);
            gKernel[x + size][y + size] = (exp(-(r*r)/s))/(M_PI * s);
            sum += gKernel[x + size][y + size];
        }
    }
    // normalize the Kernel
    for(int i = 0; i < kernel_size; ++i)
        for(int j = 0; j < kernel_size; ++j)
            gKernel[i][j] /= sum;
}

void nlm_increse_image(QImage* image, QImage* imageFiltered, QSize size, int d) {
    d = d-1;
    int dh = d/2;
    // copy image
    for (int i = dh; i < size.width()+dh; i++) {
        for (int j = dh; j < size.height()+dh; j++) {
            imageFiltered->setPixel(i, j, image->pixel(i-dh, j-dh));
        }
    }

    // copy borders
    for (int i = dh; i < size.width()+dh; i++) {
        for (int j = 0; j < dh; j++) {
            imageFiltered->setPixel(i, j, image->pixel(i-dh, dh-j));
        }
    }

    for (int i = dh; i < size.width()+dh; i++) {
        for (int j = size.height()+dh, k = 1; j < size.height()+d; j++) {
            imageFiltered->setPixel(i, j, image->pixel(i-dh, size.height()-(k++)));
        }
    }

    for (int i = 0; i < dh; i++) {
        for (int j = dh; j < size.height()+dh; j++) {
            imageFiltered->setPixel(i, j, image->pixel(dh-i, j-dh));
        }
    }

    for (int i = size.width()+dh, k = 1; i < size.width()+d; i++, k++) {
        for (int j = dh; j < size.height()+dh; j++) {
            imageFiltered->setPixel(i, j, image->pixel(size.width()-k, j-dh));
        }
    }

    // copy corners
    for (int i = 0; i < dh; i++) {
        for (int j = 0; j < dh; j++) {
            imageFiltered->setPixel(i, j, image->pixel(dh-i, dh-j));
        }
    }

    for (int i = 0; i < dh; i++) {
        for (int j = size.height()+dh, k = 1; j < size.height()+d; j++, k++) {
            imageFiltered->setPixel(i, j, image->pixel(dh-i, size.height()-k));
        }
    }

    for (int i = size.width()+dh, k = 1; i < size.width()+d; i++, k++) {
        for (int j = 0; j < dh; j++) {
            imageFiltered->setPixel(i, j, image->pixel(size.width()-k, dh-j));
        }
    }

    for (int i = size.width()+dh, k = 1; i < size.width()+d; i++, k++) {
        for (int j = size.height()+dh, n = 1; j < size.height()+d; j++, n++) {
            imageFiltered->setPixel(i, j, image->pixel(size.width()-k, size.height()-n));
        }
    }
}

void nlm_increse_image(double ** src, double ** dst, QSize srcImageSize, int inc) {
    nlm_increse_image2(src,dst,srcImageSize,inc);
    return;
    int srcW = srcImageSize.width();
    int srcH = srcImageSize.height();
    // COPY ORIGINAL IMAGE
    for (int i = inc; i < srcW+inc; i++) {
        for  (int j = inc; j < srcH+inc; j++) {
            dst[j][i] = src[j-inc][i-inc];
        }
    }

    for (int j1 = 0, j2 = srcH+inc, k1 = 1, k2 = 1; j1 < inc; j1++, k1-=1, k2+=2, j2++)
    {
        for (int i = inc; i < srcW+inc; i++) {
            dst[i][j1] = dst[i][j1+inc+k1]; // !!!!!!!!!!!
            dst[i][j2] = dst[i][j2-k2];
        }
    }

    for (int i1 = 0, i2 = srcW+inc, k1 = 1, k2 = 1; i1 < inc; i1++, i2++, k1-=1, k2+=2) {
        for (int j = 0; j < srcH+2*inc; j++) {
            dst[i1][j] = dst[i1+inc+k1][j]; // !!!!!!!!!!!
            dst[i2][j] = dst[i2-k2][j];
        }
    }

    int ww = srcW + 2*inc;
    int hh = srcH + 2*inc;
    int cnt = 0;
    cout << ww << " " << hh << endl;
    for (int i = 0; i < ww; i++) {
        for  (int j = 0; j < hh; j++) {
            if (dst[i][j] < 0.0) {
                cout<<"Error in pixel(" << i << ", " << j << ")" << endl;
                cnt++;
            }
        }
    }
    cout << "Cnt: " << cnt << endl;
}

void nlm_increse_image2(double ** src, double ** dst, QSize srcImageSize, int inc) {
    int w = srcImageSize.width();
    int h = srcImageSize.height();

    // COPY ORIGINAL IMAGE
    for (int i = inc; i < w+inc; i++) {
        for  (int j = inc; j < h+inc; j++) {
            dst[j][i] = src[j-inc][i-inc];
        }
    }

    double **left, **right, **top, **bottom;
    left = new double*[inc];
    right = new double*[inc];
    for (int i = 0; i < inc; i++) {
        left[i] = new double[h];
        right[i] = new double[h];
    }

    top = new double*[w+2*inc];
    bottom = new double*[w+2*inc];
    for (int i = 0; i < w+2*inc; i++) {
        top[i] = new double[inc];
        bottom[i] = new double[inc];
    }

    ////////////////////////////////

    for (int i = 0; i < inc; i++) {
        for (int j = 0; j < h; j++) {
            left[i][j] = src[inc-i-1][j];
        }
    }

    for (int i = 0; i < inc; i++) {
        for (int j = 0; j < h; j++) {
            right[inc-i-1][j] = src[w-inc+i][j];
        }
    }

    //////////////

    for (int i = 0; i < inc; i++) {
        for (int j = 0; j < h; j++) {
            dst[i][j+inc] = left[i][j];
        }
    }

    for (int i = 0; i < inc; i++) {
        for (int j = 0; j < h; j++) {
            dst[w+inc+i][j+inc] = right[i][j];
        }
    }

    ////////////////////////////////

    for (int i = 0; i < w+2*inc; i++) {
        for (int j = 0; j < inc; j++) {
            top[i][inc-j-1] = dst[i][j+inc];
        }
    }

    for (int i = 0; i < w+2*inc; i++) {
        for (int j = 0; j < inc; j++) {
            bottom[i][inc-j-1] = dst[i][j+h];
        }
    }

    //////////////

    for (int i = 0; i < w+2*inc; i++) {
        for (int j = 0; j < inc; j++) {
            dst[i][j] = top[i][j];
        }
    }

    for (int i = 0; i < w+2*inc; i++) {
        for (int j = 0; j < inc; j++) {
            dst[i][inc+h+j] = bottom[i][j];
        }
    }

    ////////////////////////////////

    for (int i = 0; i < inc; i++) {
        delete[] left[i];
        delete[] right[i];
    }

    for (int i = 0; i < w+2*inc; i++) {
        delete[] top[i];
        delete[] bottom[i];
    }
    delete[] left;
    delete[] right;
    delete[] top;
    delete[] bottom;
}

void nlm_increse_image2(float ** src, float ** dst, QSize srcImageSize, int inc) {
    int w = srcImageSize.width();
    int h = srcImageSize.height();

    // COPY ORIGINAL IMAGE
    for (int i = inc; i < w+inc; i++) {
        for  (int j = inc; j < h+inc; j++) {
            dst[j][i] = src[j-inc][i-inc];
        }
    }

    float **left, **right, **top, **bottom;
    left = new float*[inc];
    right = new float*[inc];
    for (int i = 0; i < inc; i++) {
        left[i] = new float[h];
        right[i] = new float[h];
    }

    top = new float*[w+2*inc];
    bottom = new float*[w+2*inc];
    for (int i = 0; i < w+2*inc; i++) {
        top[i] = new float[inc];
        bottom[i] = new float[inc];
    }

    ////////////////////////////////

    for (int i = 0; i < inc; i++) {
        for (int j = 0; j < h; j++) {
            left[i][j] = src[inc-i-1][j];
        }
    }

    for (int i = 0; i < inc; i++) {
        for (int j = 0; j < h; j++) {
            right[inc-i-1][j] = src[w-inc+i][j];
        }
    }

    //////////////

    for (int i = 0; i < inc; i++) {
        for (int j = 0; j < h; j++) {
            dst[i][j+inc] = left[i][j];
        }
    }

    for (int i = 0; i < inc; i++) {
        for (int j = 0; j < h; j++) {
            dst[w+inc+i][j+inc] = right[i][j];
        }
    }

    ////////////////////////////////

    for (int i = 0; i < w+2*inc; i++) {
        for (int j = 0; j < inc; j++) {
            top[i][inc-j-1] = dst[i][j+inc];
        }
    }

    for (int i = 0; i < w+2*inc; i++) {
        for (int j = 0; j < inc; j++) {
            bottom[i][inc-j-1] = dst[i][j+h];
        }
    }

    //////////////

    for (int i = 0; i < w+2*inc; i++) {
        for (int j = 0; j < inc; j++) {
            dst[i][j] = top[i][j];
        }
    }

    for (int i = 0; i < w+2*inc; i++) {
        for (int j = 0; j < inc; j++) {
            dst[i][inc+h+j] = bottom[i][j];
        }
    }

    ////////////////////////////////

    for (int i = 0; i < inc; i++) {
        delete[] left[i];
        delete[] right[i];
    }

    for (int i = 0; i < w+2*inc; i++) {
        delete[] top[i];
        delete[] bottom[i];
    }
    delete[] left;
    delete[] right;
    delete[] top;
    delete[] bottom;
}
