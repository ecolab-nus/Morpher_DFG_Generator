#include <stdio.h>

volatile int * N;
volatile int *a;
volatile int *b;
int *c;
int **A;
int *tmp;
int *x;
int *y;
int **B;
int main() {
    int i,j;
    int n = *N;

    int sum = 0;
    for (j = 1; j < n-1; j++) {
               #ifdef CGRA_COMPILER
           please_map_me();
           #endif
        tmp[i] = A[i][j] * x[j] + tmp[i];
	  y[i] = B[i][j] * x[j] + y[i];

       tmp[i] = A[i][j+1] * x[j+1] + tmp[i];
	  y[i] = B[i][j+1] * x[j+1] + y[i];
    }

    return sum;
}
