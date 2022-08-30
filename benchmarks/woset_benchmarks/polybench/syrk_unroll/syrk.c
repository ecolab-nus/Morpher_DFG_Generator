#include <stdio.h>

volatile int * N;
volatile int *a;
volatile int *b;
int *c;
int **A;
int **C;
int main() {
    int i, j ,k, alpha=10;
    int n = *N;

    int sum = 0;
    for (k = 1; k < n-1; k++) {
               #ifdef CGRA_COMPILER
           please_map_me();
           #endif
        C[i][j] += alpha * A[i][k] * A[j][k];
            C[i][j] += alpha * A[i][k+1] * A[j][k+1];

    }

    return sum;
}
