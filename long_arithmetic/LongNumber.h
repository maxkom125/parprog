
typedef unsigned long long llu; // > 1,8*10^19 > (1 * 10^9) * (1 * 10^9) > 99'999'999 * 99'999'999


typedef struct LongNumber {
    llu* array;
    llu len;
    short sign;
} LongNumber;

// CAN BE IMPROVED! STORE IN UNSIGNED INT, DO OPERATIONS IN llu
LongNumber LongNumberConstruct(llu number, llu len, short sign); // array of elements < 10^9

LongNumber LongNumberConstructMove(llu* array, llu len, short sign); // don't use new memory for the array

void Print0(int count); // printf 000...0

int GetLenOfllu(llu x); // len of llu number

void LongNumberPrint(const LongNumber* ln);

LongNumber LongNumberSameDigitConstruct(llu digit, llu len, short sign);  // Long number with same digits (8 * len)

void LongNumberDestruct(LongNumber *ln);

llu lluMax(llu x1, llu x2); // max(x1, x2)

llu lluMin(llu x1, llu x2); // min(x1, x2)

short _LongNumberSumPlus(LongNumber* sum, LongNumber* num0, LongNumber* num1, llu plus); // Long Number + Long Number + plus;
                                                                                         // must be num0->len == num1->len !!!!
                                                                                         // returns 1 if one goes to the next digit
                                                                                         // else returns 0

int LongNumberResize(LongNumber* number, llu new_len); // realloc array

// ONLY for >=0 Long Numbers!
LongNumber LongNumbersParallelSum(LongNumber* number1, LongNumber* number2); // PARALLEL SUM OF long numbers

