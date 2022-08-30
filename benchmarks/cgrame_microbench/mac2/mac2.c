#include <stdio.h>

volatile int * N;
//int *a;
//int *b;
//int *c;
//int *d;
int a[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
int b[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
int c[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
int d[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

int main() {
    int i;
    int n = 20;//*N;

    int sum = 0;
    int sum2 = 0;

    for (i = 1; i < n-1; i++) {
    //DFGLoop: loop
#ifdef CGRA_COMPILER
please_map_me();
#endif 
        sum += a[i] * b[i];
        sum2 += a[i] * (b[i] + 1) * c[i] * d[i];
    }

    return sum + sum2;
}
