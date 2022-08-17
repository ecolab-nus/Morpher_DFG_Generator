#include <stdio.h>
#include <math.h>

#define SIZE 10
int n = SIZE, m = SIZE;
int A[SIZE][SIZE], p[SIZE], r[SIZE], s[SIZE], q[SIZE];
int i = 0, j = 0;

__attribute__((noinline))
void cholesky()
{   
    int i = 0 ,  j = 0, k = 0;
    for (i = 0; i < SIZE; i++)
    {
        // j<i
        for (j = 0; j < i; j++)
        {
            for (k = 0; k < j; k++)
            {
#ifdef CGRA_COMPILER
    please_map_me();
#endif
                A[i][j] -= A[i][k] * A[j][k];
            }
            A[i][j] /= A[j][j];
        }
        for (k = 0; k < i; k++)
        {
            A[i][i] -= A[i][k] * A[i][k];
        }
        A[i][i] = sqrtf(A[i][i]);
    }
}


int main()
{
    for (i = 0; i < n; i++)
    {
        for (j = 0; j <= i; j++)
            A[i][j] = (-j % n) / n + 1;
        for (j = i + 1; j < n; j++)
        {
            A[i][j] = 0;
        }
        A[i][i] = 1;
    }
    cholesky();
    return 0;
}
