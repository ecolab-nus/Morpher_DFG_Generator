#include <stdio.h>

#define SIZE 4
int n = SIZE, m = SIZE;
int A[SIZE][SIZE][SIZE], C4[SIZE][SIZE],sum[SIZE];
int i = 0, j = 0, np = 0, k = 0, r = 0, p = 0, q = 0, s = 0;

__attribute__((noinline))
void doitgen()
{
    int r = 0, p = 0, q = 0, s = 0;
    for (r = 0; r < SIZE; r++)
        for (q = 0; q < SIZE; q++)
        {
            for (p = 0; p < SIZE; p++)
            {
                sum[p] = 0;
                for (s = 0; s < SIZE; s++){
#ifdef CGRA_COMPILER
    please_map_me();
#endif
                    sum[p] += A[r][q][s] * C4[s][p];

                }
            }
            for (p = 0; p < SIZE; p++)
                A[r][q][p] = sum[p];
        }
}

int main()
{
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            for (k = 0; k < SIZE; k++)
                A[i][j][k] = ((i * j + k) % np) / np;
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            C4[i][j] = (i * j % np) / np;
    doitgen();
    return 0;
}
