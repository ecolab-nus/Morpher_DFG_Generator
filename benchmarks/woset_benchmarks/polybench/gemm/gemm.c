#include <stdio.h>

volatile int * N;
volatile int *a;
volatile int *b;
int *c;
volatile int **A;
volatile int **B;
volatile int **C;


int main() {
    int i, j, k;
    int n = *N;
    int alpha = 20;
    int tmp;

    int sum = 0;
    for (k = 1; k < n-1; k++) {
                #ifdef CGRA_COMPILER
           please_map_me();
           #endif
     tmp = alpha * A[i][k];
     tmp = tmp * B[k][j];
         C[i][j] += tmp ;
    }

     /* C := alpha*A*B + beta*C */
//   for (i = 0; i < _PB_NI; i++)
//     for (j = 0; j < _PB_NJ; j++)
//       {
// 	C[i][j] *= beta;
// 	for (k = 0; k < _PB_NK; ++k)
// 	  C[i][j] += alpha * A[i][k] * B[k][j];
//       }

    return sum;
}
