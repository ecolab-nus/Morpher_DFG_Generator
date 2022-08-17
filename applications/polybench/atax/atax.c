#include <string.h>
#include <stdio.h>

#define SIZE 10
int n = SIZE;
int A[SIZE][SIZE], x[SIZE], y[SIZE], tmp[SIZE];
int i = 0, j = 0;
__attribute__((noinline))
void atax()
{

    int i = 0, j = 0, k = 0, sum = 0, alpha = 10;

    for (i = 0; i < SIZE; i++)
    {
        tmp[i] = 0;
        for (j = 0; j < SIZE; j++)
        {
#ifdef CGRA_COMPILER
            please_map_me();
#endif
            tmp[i] = tmp[i] + A[i][j] * x[j];
        }

        for (j = 0; j < SIZE; j++)
            y[j] = y[j] + A[i][j] * tmp[i];
    }

    printf("%i\n", sum);
}

void main()
{

    int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
        {
            A[i][j] = i * j;
            tmp[i] = j * i;
            x[i] = j * i;
            y[i] = j * i;
        }

    atax();

    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
        {
            printf("%d\n", A[i][j]);
        }
}
