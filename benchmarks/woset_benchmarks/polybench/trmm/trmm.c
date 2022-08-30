#include <stdio.h>

volatile int * N;
volatile int *a;
volatile int *b;
int *c;
int **B;
int **A;


int main() {
    int i, j, k, alpha = 5;
    int n = *N;

    int sum = 0;
    for (i = 1; i < n-1; i++) {
               #ifdef CGRA_COMPILER
           please_map_me();
           #endif
        B[i][j] += alpha * A[i][k] * B[j][k];
    }

    return sum;
}
