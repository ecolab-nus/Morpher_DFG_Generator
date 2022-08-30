#include <stdio.h>

volatile int * N;
volatile int **A;
volatile int *p;
volatile int *r;
volatile int *s;
volatile int *q;


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
        s[j] = s[j] + r[i] * A[i][j];
	  q[i] = q[i] + A[i][j] * p[j];


        s[j+1] = s[j+1] + r[i] * A[i][j+1];
	  q[i] = q[i] + A[i][j+1] * p[j+1];
    }


// #pragma scop
//   for (i = 0; i < _PB_NY; i++)
//     s[i] = 0;
//   for (i = 0; i < _PB_NX; i++)
//     {
//       q[i] = 0;
//       for (j = 0; j < _PB_NY; j++)
// 	{
// 	  s[j] = s[j] + r[i] * A[i][j];
// 	  q[i] = q[i] + A[i][j] * p[j];
// 	}
//     }
// #pragma endscop


    return sum;
}
