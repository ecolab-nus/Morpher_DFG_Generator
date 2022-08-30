#include <stdio.h>

volatile int *N;
volatile int *a;
volatile int *b;
volatile int **A;
volatile int *p;

int *c;

int main()
{
    int i;
    int n = *N;
    int _PB_N, x = 1000, j, k;

    int sum = 0;
    x = 100;
    A[0][0] = 1;
    for (k = 1; k < n - 1; k++)
    {
                   #ifdef CGRA_COMPILER
           please_map_me();
           #endif
        x = x - A[j][k] * A[i][k];
        x = x - A[j][k+1] * A[i][k+1];
    }

    for (i = 0; i < _PB_N; ++i)
    {
        x = A[i][i];
        for (j = 0; j <= i - 1; ++j)
            x = x - A[i][j] * A[i][j];
        p[i] = x;
        for (j = i + 1; j < _PB_N; ++j)
        {
            x = A[i][j];
            for (k = 0; k <= i - 1; ++k)
            {
                x = x - A[j][k] * A[i][k];
            }
            A[j][i] = x * p[i];
        }
    }

    return x;
}
