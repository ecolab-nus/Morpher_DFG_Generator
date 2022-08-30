#include <stdio.h>

volatile int * N;
//int *a;
//int *b;
int *m;

int a[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
int b[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

int main() {
    int i, j;
    int n = 10;// *N;

    int sum = 0;

    for (i = 1; i < n-1; i++) {
        //DFGLoop: loop
#ifdef CGRA_COMPILER
please_map_me();
#endif 
        sum += (a[i] * 2 + a[i + 1] * 2) * (b[i] * 2 + b[i + 3] * 2) * (b[i + 3] * 3) * b[i];
    }

    return sum;
}
