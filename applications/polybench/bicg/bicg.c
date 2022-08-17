#include <stdio.h>

#define SIZE 10
int n = SIZE, m = SIZE;
int A[SIZE][SIZE], p[SIZE], r[SIZE], s[SIZE], q[SIZE];
int i = 0, j = 0;

__attribute__((noinline))
void bicg()
{
    int i = 0, j = 0;
     for (i = 0; i < SIZE; i++)
    {
        q[i] = 0;
        for (j = 0; j < SIZE; j++)
        {
#ifdef CGRA_COMPILER
    please_map_me();
#endif
            s[j] = s[j] + r[i] * A[i][j];
            q[i] = q[i] + A[i][j] * p[j];
        }
    }
}
int main()
{
    int i, j;

    for (i = 0; i < m; i++){
        p[i] = (i % m) / m;
        r[i] =  i ;
        s[i] =  i ;
        q[i] =  i ;
    }
        
    for (i = 0; i < n; i++)
    {
        r[i] = (i % n) / n;
        for (j = 0; j < m; j++)
            A[i][j] = (i * (j + 1) % n) / n;
    }

    bicg();

    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
        {
            printf("%d\n", A[i][j]);
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

    return 0;
}
