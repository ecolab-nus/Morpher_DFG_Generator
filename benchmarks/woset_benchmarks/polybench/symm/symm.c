#include <stdio.h>

volatile int * N;
volatile int *a;
volatile int *b;
int **A;
int **B;
int **C;

int main() {
    int i, j, k, alpha = 1, acc = 100;
    int n = *N;

    int sum = 0;
    for (k = 1; k < n-1; k++) {
               #ifdef CGRA_COMPILER
           please_map_me();
           #endif
         C[k][j] += alpha * A[k][i] * B[i][j];
	    acc += B[k][j] * A[k][i];
    }

    return sum;
}
