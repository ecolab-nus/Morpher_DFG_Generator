#include <string.h>
#include <stdio.h>


#define SIZE  8
int A[SIZE*SIZE], B[SIZE*SIZE], C[SIZE*SIZE];

void gemm(){
   int i,j,k;

   for (i=0;i<SIZE; i++)
      for (j=0;j<SIZE; j++)
        for (k=0;k<SIZE; k++){
           #ifdef CGRA_COMPILER
           please_map_me();
           #endif
           A[i*SIZE+j] = A[i*SIZE+j] + B[i*SIZE+k]* C[k*SIZE+j];
        }

}

void main(){

int i,j;
for (i=0;i<SIZE; i++)
   for (j=0;j<SIZE; j++) {
      A[i*SIZE+j]=0;
      B[i*SIZE+j] = j+i;
      C[i*SIZE+j] = j*i;
    }
    
gemm();

for (i=0;i<SIZE; i++)
   for (j=0;j<SIZE; j++) {
      printf("%d\n", A[i*SIZE+j]);
    }

}


