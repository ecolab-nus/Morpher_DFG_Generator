#include <stdio.h>

volatile int a[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

volatile int * n;
// Simple accumulation
int main() {

    int N = 20;//*n;
    int sum = 0;
    int i;
    for (i = 0; i < N; i++) {
#ifdef CGRA_COMPILER
please_map_me();
#endif 
        sum += a[i];
    }
    printf("sum = %d\n", sum);

    return sum;
}
