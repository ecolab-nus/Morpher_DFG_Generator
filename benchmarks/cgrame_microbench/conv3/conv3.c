#include <stdio.h>

volatile int * N;
int a[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
int b[20];
int *m;

int main() {
    int i, j;
    int n = 10;//*N;

    int sum = 0;

//    int a_2 = a[i];
//    int a_1 = a[i + 1];
    for (i = 1; i < n-1; i++) {
        //DFGLoop: loop
#ifdef CGRA_COMPILER
please_map_me();
#endif 
        b[i] = a[i] * 10 + a[i + 1] * 20 + a[i+ 2] * 3;
        /*int a_0 = a[i+2];
        b[i] = a_2 * 1 + a_1 * 2 + a_0 * 3;
        a_2 = a_1;
        a_1 = a_0;
        */
    }

    return 0;
}
