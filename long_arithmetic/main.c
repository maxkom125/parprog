#include <stdio.h>
//#include <string.h>
#include <stdlib.h>
//#include <time.h>

#include "mpi.h"
#include "LongNumber.h"

int main(int argc, char* argv[]){

    MPI_Init(&argc, &argv);

    LongNumber ln1, ln2, sum;

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double start_time = MPI_Wtime();


    if (rank == 0) {
        ln1 = LongNumberSameDigitConstruct(9, 400, 1);
        //LongNumberPrint(&ln1);
        ln2 = LongNumberSameDigitConstruct(2, 400, 1);
        //LongNumberPrint(&ln2);

    }
    sum = LongNumbersParallelSum(&ln1, &ln2);

    if (rank == 0) {
        //LongNumberPrint(&sum);
        printf("Need time: %f\n", MPI_Wtime() - start_time);

        LongNumberDestruct(&ln1);
        LongNumberDestruct(&ln2);
        LongNumberDestruct(&sum);
    }


    MPI_Finalize();

    return 0;
}

