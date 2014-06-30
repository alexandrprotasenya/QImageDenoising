#include "awgn.h"

#include <QDebug>

#define PI 3.1415926536

double AWGN_generator() {
    return AWGN_generator(1);
}

double AWGN_generator(double std)
{/* Генерация аддитивного белого гауссовского шума с нулевым средним и стандартным отклонением, равным 1. */

    double temp1;
    double temp2;
    double result;
    int p;

    p = 1;

    while( p > 0 )
    {
        temp2 = ( rand() / ( (double)RAND_MAX ) ); /* функция rand() генерирует
                                                       целое число между 0 и  RAND_MAX,
                                                       которое определено в stdlib.h.
                                                   */

        if ( temp2 == 0 )
        {// temp2 >= (RAND_MAX / 2)
            p = 1;
        }// конец if
        else
        {// temp2 < (RAND_MAX / 2)
            p = -1;
        }// конец else

    }// конец while()

    temp1 = cos( ( 2.0 * (double)PI ) * rand() / ( (double)RAND_MAX ) );
    result = sqrt( -2.0 * log( temp2 ) ) * temp1;

    return result*std;        // возвращаем сгенерированный сэмпл

}// конец AWGN_generator()
