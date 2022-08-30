#include <stdio.h>

volatile int * N;
volatile int **A;
volatile int *y;
volatile int *tmp;
int *c;

int main() {
    int i;
    int j;
    int n = *N;

    int sum = 0;
    for (j = 1; j < n-1; j++) {
               #ifdef CGRA_COMPILER
           please_map_me();
           #endif
        y[j] = y[j] + A[i][j] * tmp[i];
    }

//     #pragma scop
//   for (i = 0; i < _PB_NY; i++)
//     y[i] = 0;
//   for (i = 0; i < _PB_NX; i++)
//     {
//       tmp[i] = 0;
//       for (j = 0; j < _PB_NY; j++)
// 	tmp[i] = tmp[i] + A[i][j] * x[j];
//       for (j = 0; j < _PB_NY; j++)
// 	y[j] = y[j] + A[i][j] * tmp[i];
//     }
// #pragma endscop

    return sum;
}
