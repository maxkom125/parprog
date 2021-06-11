#include <stdio.h>
#include <stdlib.h>
#include "unistd.h"
#include "LongNumber.h"
#include "mpi.h"

LongNumber LongNumberConstruct(llu number, llu len, short sign){ // array of elements < 10^9
    LongNumber ln;
    if (len > 0) {
        ln.array = (llu *) calloc(len, len * sizeof(llu));

        if (ln.array == NULL)
            printf("ERROR in %s: can't allocate array!\n", __PRETTY_FUNCTION__);
        else
            ln.array[0] = number;
    }
    else
        ln.array = NULL;


    ln.len = len;
    ln.sign = sign;

    return ln;
}

LongNumber LongNumberConstructMove(llu* array, llu len, short sign){
    LongNumber ln;

    ln.len = len;
    ln.sign = sign;
    ln.array = array;

    return ln;
}

void Print0(int count){
    int i;
    for (i = 0; i < count; i++)
        printf("0");
}

int GetLenOfllu(llu x){
    int ans = 0;
    while (x > 0){
        x /= 10;
        ans++;
    }
    return ans;
}

void LongNumberPrint(const LongNumber* ln){
    if (ln->sign <= 0)
        printf("-");

    printf("%llu|", ln->array[ln->len - 1]);

    llu i = ln->len - 1;
    do {
        i--;
        Print0(7 - GetLenOfllu(ln->array[i]));
        printf("%llu|", ln->array[i]);
    } while (i > 0);

    printf("\n");
}

LongNumber LongNumberSameDigitConstruct(llu digit, llu len, short sign){ // array of elements < 10^9
    LongNumber ln;
    ln = LongNumberConstruct(digit, len, sign);

    llu elem = 0;
    llu i;

    for (i = 0; i < 8; i++){
        elem += digit;
        elem *= 10;
    }
    elem /= 10;
    //printf("%llu", elem);

    for (i = 0; i < ln.len; i++){
        //printf("%llu", elem);
        ln.array[i] = elem;
    }

    return ln;
}

void LongNumberDestruct(LongNumber *ln) {
    if (ln->array != NULL)
        free(ln->array);

    ln->array = NULL;
    ln->len = 0;
    ln->sign = 0;
}

llu lluMax(llu x1, llu x2){
    if (x1 < x2)
        return x2;
    else
        return x1;
}

short _LongNumberSumPlus(LongNumber* sum, LongNumber* num0, LongNumber* num1, llu plus){
    llu i, maxlen = lluMax(num0->len, num1->len);
    int j, t = 10000;

    for (i = 0; i < maxlen; i++) {
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!ONLY FOR LONGER WORKTIME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        for (j = 0; j < 5e5; j++)
            t = (t * 9999 - 10000) / 9998;
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        sum->array[i] = num0->array[i] + num1->array[i] + plus;
        if (sum->array[i] >= 1e9) {
            sum->array[i] -= 1e9;
            plus = 1;
        }
        else
            plus = 0;
    }
    return (short)plus;
}

int LongNumberResize(LongNumber* number, llu new_len){
    llu* buff = (llu*)realloc(number->array, new_len * sizeof(llu));
    if (buff != NULL)
        number->array = buff;
    else
        return -1;

    llu i;
    for (i = number->len; i < new_len; i++)
        number->array[i] = 0;

    number->len = new_len;
    return 1;
}


LongNumber LongNumbersParallelSum(LongNumber* number1, LongNumber* number2){
    int numtasks;
    int rank, r;
    llu i;
    short addOne0, addOne1, addOne;
    llu part_start, part_len;

    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //printf("0) %d \n", rank);

    llu maxlen;
    //int minlen;

    LongNumber part_number1;
    LongNumber part_number2;

    if (rank == 0) {
        //minlen = intMin(number1->len, number2->len);
        maxlen = lluMax(number1->len, number2->len);

        if (number1->len == maxlen) {
            if (LongNumberResize(number2, maxlen) < 0)
                exit(-1);
        }
        else {
            if (LongNumberResize(number1, maxlen) < 0)
                exit(-1);
        }

        //LongNumberPrint(number1);
        //LongNumberPrint(number2);
        // 1) If there are too many tasks: return some of them
        int numtasks_prev = numtasks;
        if ((llu)numtasks > maxlen)
            numtasks = maxlen;

        for (r = 1; r < numtasks_prev; r++)
            MPI_Ssend(&numtasks, 1, MPI_INT, r, 222, MPI_COMM_WORLD);


        for (r = 1; r < numtasks; r++) {
            part_start = maxlen * r       / numtasks;
            part_len   = maxlen * (r + 1) / numtasks - part_start;

            MPI_Ssend(&part_len, 1, MPI_UNSIGNED_LONG_LONG, r, 0, MPI_COMM_WORLD);
            MPI_Ssend(number1->array + part_start, part_len, MPI_UNSIGNED_LONG_LONG, r, 1, MPI_COMM_WORLD);
            MPI_Ssend(number2->array + part_start, part_len, MPI_UNSIGNED_LONG_LONG, r, 2, MPI_COMM_WORLD);
        }

        part_len = maxlen * 1 / numtasks;
        part_number1 = LongNumberConstructMove(number1->array, part_len, 1);
        part_number2 = LongNumberConstructMove(number2->array, part_len, 1);
        addOne = 1;
    }
    else {
        // 2) If there are too many tasks: return some of them
        MPI_Recv(&numtasks, 1, MPI_INT, 0, 222, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (rank >= numtasks) {
            part_number1 = LongNumberConstruct(0, 0, 1);
            return part_number1;
        }

        MPI_Recv(&part_len, 1, MPI_UNSIGNED_LONG_LONG, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        part_number1 = LongNumberConstruct(0, part_len, 1);
        part_number2 = LongNumberConstruct(0, part_len, 1);

        MPI_Recv(part_number1.array, part_len, MPI_UNSIGNED_LONG_LONG, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(part_number2.array, part_len, MPI_UNSIGNED_LONG_LONG, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    //printf("1) %d \n", rank);
    //MPI_Barrier(MPI_COMM_WORLD);

    LongNumber part_sum0 = LongNumberConstruct(0, part_len, 1); //sum
    LongNumber part_sum1 = LongNumberConstruct(0, part_len, 1); //sum + 1


    //ADD CHECK SIGN, SUBTRUCT
    //printf("%d \n", rank);
    //LongNumberPrint(&part_number1);
    //LongNumberPrint(&part_number2);
    addOne0 = _LongNumberSumPlus(&part_sum0, &part_number1, &part_number2, 0);
    addOne1 = _LongNumberSumPlus(&part_sum1, &part_number1, &part_number2, 1);


    LongNumber part_sum, result = LongNumberConstruct(0, 0, 1);

    //printf("2) %d \n", rank);
    //MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {//sum first elements of array (last digits of number)
        part_sum = part_sum0;
        LongNumberDestruct(&part_sum1);

        //LongNumberPrint(part_sum);
        //printf("1) send %d -> %d \n", 0, 1);
        if (numtasks != 1)
            MPI_Ssend(&addOne0, 1, MPI_SHORT, 1, 3, MPI_COMM_WORLD);
    }
    else {
        //printf("2-3) Recv %d <- %d \n", rank, rank - 1);
        MPI_Recv(&addOne, 1, MPI_SHORT, rank - 1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (addOne > 0) { //if get 1: part res = sum + 1;
            part_sum = part_sum1;
            LongNumberDestruct(&part_sum0);

            if (rank < numtasks - 1) {
                //printf("2) send %d -> %d \n", rank, rank + 1);
                MPI_Ssend(&addOne1, 1, MPI_SHORT, rank + 1, 3, MPI_COMM_WORLD);
            }
        }
        else { //if get 0: part res = sum + 0;
            part_sum = part_sum0;
            LongNumberDestruct(&part_sum1);

            if (rank < numtasks - 1) {
                //printf("3) send %d -> %d \n", rank, rank + 1);
                MPI_Ssend(&addOne0, 1, MPI_SHORT, rank + 1, 3, MPI_COMM_WORLD);
            }
        }
        //printf("arr) send %d -> %d \n", rank, 0);
        MPI_Ssend(part_sum.array, part_len, MPI_UNSIGNED_LONG_LONG, 0, 4, MPI_COMM_WORLD);
        LongNumberDestruct(&part_sum);
    }

    short resultSign = 1;

    if (rank == numtasks - 1 && numtasks != 1){
        MPI_Ssend(&addOne, 1, MPI_SHORT, 0, 5, MPI_COMM_WORLD); //need recize?
    }

    if (rank == 0) {
        result = LongNumberConstruct(0, maxlen, resultSign);
        llu *resultArr = result.array;

        for (i = 0; i < part_len; i++){
            resultArr[i] = part_sum.array[i];
        }
        LongNumberDestruct(&part_sum);

        for (r = 1; r < numtasks; r++) {
            part_start  = maxlen * r       / numtasks;
            part_len    = maxlen * (r + 1) / numtasks - part_start;

            //printf("End: Recv %d <- %d \n", 0, r);
            MPI_Recv(resultArr + part_start, part_len, MPI_UNSIGNED_LONG_LONG, r, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        if (numtasks != 1)
            MPI_Recv(&addOne, 1, MPI_SHORT, numtasks - 1, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //need recize?
        if (addOne > 0) {
            if (LongNumberResize(&result, maxlen + 1) >= 0)
                result.array[maxlen] = 1;
            else
                exit(-1);

        }

    }




    return result;
}



