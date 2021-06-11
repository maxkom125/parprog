// Parrallel integration. function on [a, b].

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double function(double x);

int main(int argc, char* argv[]) {
    float t = omp_get_wtime();

    if(argc < 2) {
        fprintf(stderr, "Wrong argc\n");
        return -1;
    }

    const long int N = atol(argv[1]);

    const double a = 0;
    const double b = 2;

    const double h = (b - a) / N;

    double sum_i = 0;
    long int i;

#pragma omp parallel for reduction (+:sum_i)
        for (i = 1; i < N; i++) {
            sum_i += function(a + i * h);
        }

    sum_i += (function(a) + function(b)) / 2;
    sum_i *= h;

    printf("Integral of sqrt(4 - x^2) approximetly equals to %.6f. Calculating time: %f \n", sum_i, (omp_get_wtime() - t));

    //printf("%f \n", (omp_get_wtime() - t));
    return 0;
}

double function(double x) {
    return sqrt(4 - pow(x, 2));
}
