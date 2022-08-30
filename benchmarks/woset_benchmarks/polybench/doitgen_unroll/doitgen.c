#include <stdio.h>

volatile int * N;
volatile int *a;
volatile int *b;
volatile int ***sum;
volatile int ***A;
volatile int **C4;

int *c;

int main() {
    int i;
    int n = *N;
    int r;
    int  p, q, s;
    int _PB_NR, _PB_NQ, _PB_NP;

    // int sum = 0;
    for (s = 1; s < n-1; s++) {
               #ifdef CGRA_COMPILER
           please_map_me();
           #endif
         sum[r][q][p] = sum[r][q][p] + A[r][q][s] * C4[s][p];

         sum[r][q][p] = sum[r][q][p] + A[r][q][s+1] * C4[s+1][p];
    }

//     #pragma scop
//   for (r = 0; r < _PB_NR; r++)
//     for (q = 0; q < _PB_NQ; q++)  {
//       for (p = 0; p < _PB_NP; p++)  {
// 	sum[r][q][p] = 0;
// 	for (s = 0; s < _PB_NP; s++)
// 	  sum[r][q][p] = sum[r][q][p] + A[r][q][s] * C4[s][p];
//       }
//       for (p = 0; p < _PB_NR; p++)
// 	A[r][q][p] = sum[r][q][p];
//     }
// #pragma endscop

    return 0;
}
