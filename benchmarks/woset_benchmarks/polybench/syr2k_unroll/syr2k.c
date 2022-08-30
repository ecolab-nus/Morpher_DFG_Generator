#include <stdio.h>

volatile int * N;
volatile int *a;
volatile int *b;
int *c;
int **A;
int **B;
int **C;
int main() {
    int i, j, k, alpha = 2;
    int n = *N;

    int sum = 0;
    for (k = 1; k< n-1; k++) {
               #ifdef CGRA_COMPILER
           please_map_me();
           #endif
       C[i][j] += alpha * A[i][k] * B[j][k];
	  C[i][j] += alpha * B[i][k] * A[j][k];

      C[i][j] += alpha * A[i][k+1] * B[j][k+1];
	  C[i][j] += alpha * B[i][k+1] * A[j][k+1];
    }

    return sum;
}
