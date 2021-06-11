#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <time.h>
#include <omp.h>
//#include <unistd.h>
/*
Решаем одномерное уравнение теплопроводности. Гранияные условия const.
Делим 1 слой на количество кусков, равных количеству процессов.
Создаем массивы такие, чтобы первые два элемента были равны первым двум предидущего массива.
В нулевой и последней точке(узла) необходима информация из предидущего слоя (первая и предпоследняя) соседних процессов.
Также значения первой и предпоследней точки пересылаем соседним процессам.
Пересылки между процессами делаются одновременно: Сначала чётные шлют нечетным потом наоборот.
Ответ записывается в  PA_task2_out.txt процессами последовательно. Каждый следующий ждет Send от предидущего.

 0 1 2 3 4 5 6 7 8 9 10 11
 0 1 2 3
     2 3 4 5 6 7
     \ | /   6 7 8 9 10 11
       3 . . 6      \ | /
               7 . . 10

 */

double Get_U(double* previous_row, long int i);
void Save_row(const double* current_row, const int rank, const long int array_len);
void Swap_double_arrays(double** array1, double** array2);

const double a   = 0;
const double b   = 1;
const double c   = 1;
const double phi = 0;
const double l = 1;
const double integr_step = 0.3;

double N;
double M;
double h;
double tau;

int process_count;


//double finish_time = -2;

int main(int argc, char* argv[]) {
    float start_time = omp_get_wtime();
    N = (double)atol(argv[1]) + 2;

    const double T = atof(argv[2]);

    h   = (l - 0) / ((double)N - 1);
    tau = integr_step * h * h / (c * c);

    M = T / tau;

    long int array_len = N;
    if(array_len < 3) {
        fprintf(stderr, "Too many tasks (comm_size) and too low N (first parameter)!\n");
        exit(-1);
    }

    double* current_row  = (double*) calloc (array_len, sizeof(double));
    double* previous_row = (double*) calloc (array_len, sizeof(double));

    long int n;
    if (process_count == 1)
        current_row[array_len - 1] = b;

    for(n = 1; n <= M; n++) {
#pragma omp parallel for
        for (long int m = 1; m < array_len - 1; m++) {
            current_row[m] = Get_U(previous_row, m);
        }
        #pragma omp single
        {
        current_row[0] = a;
        current_row[array_len - 1] = b;

        Swap_double_arrays(&previous_row, &current_row);
        }
    }
    long int m_i;
    FILE* fp = fopen("PA_task2_out.txt", "w");

    for(m_i = 0; m_i < array_len; m_i++)
        fprintf(fp, "%f %f\n", h * (m_i), current_row[m_i]);
    fclose(fp);

    printf("%f \n", (omp_get_wtime() - start_time));

    free(previous_row);
    free(current_row);
    return 0;
}



double Get_U(double* previous_row, long int i) {
    return previous_row[i] + integr_step * (previous_row[i + 1] - 2 * previous_row[i] + previous_row[i - 1]);
}

void Swap_double_arrays(double** array1, double** array2){
    double* x = *(array1);
    *array1 = *array2;
    *array2 = x;
}
/*
void Save_row(const double* current_row, const int rank, const long int array_len){
    long int m = 0;
    if(rank != 0) {
        MPI_Recv(&m, 1, MPI_DOUBLE, rank - 1, rank - 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        FILE* fp = fopen("savefile.txt", "a");

        for(long int m_i = 2; m_i < array_len; m_i++)
            fprintf(fp, "%f", current_row[m_i]);

        //m += m_i;
        fclose(fp);

        if(rank < process_count - 1)
            MPI_Ssend(&m, 1, MPI_DOUBLE, rank + 1, rank, MPI_COMM_WORLD);
        else{
            fp = fopen("savefile.txt", "a");
            fprintf(fp, "\n");
            fclose(fp);
        }
    }
    else { //rank == 0
        FILE* fp = fopen("savefile.txt", "a");

        for(m = 0; m < array_len; m++)
            fprintf(fp, "%f", current_row[m]);

        fclose(fp);

        if(rank < process_count - 1)
            MPI_Ssend(&m, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
    }
}
*/
