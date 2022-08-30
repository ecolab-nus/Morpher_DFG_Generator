#include <string.h>
#include <stdio.h>


#define SIZE  8
int n = SIZE;
int A[SIZE*SIZE], B[SIZE*SIZE], C[SIZE*SIZE];
int i,j;

__attribute__((noinline))
void gemm(){



   int i,j,k;

   for (i=0;i<SIZE; i++)
      for (j=0; j<SIZE; j++)
        for (k=0;k<SIZE; k=k+4){
          #ifdef CGRA_COMPILER
           please_map_me();
           #endif
		       A[i*SIZE+j] = A[i*SIZE+j] + B[i*SIZE+k]* C[k*SIZE+j] + B[i*SIZE+k+1]* C[(k+1)*SIZE+j]
			       + B[i*SIZE+k+2]* C[(k+2)*SIZE+j] + B[i*SIZE+k+3]* C[(k+3)*SIZE+j];
            }


}

void main(){

int i,j;
for (i=0;i<n; i++)
   for (j=0; j<n; j++) {
      A[i*SIZE+j]=0;
      B[i*SIZE+j] = j+i;
      C[i*SIZE+j] = j*i;
    }
    
gemm();

for (i=0;i<n; i++)
   for (j=0; j<n; j++) {
      printf("%d\n", A[i*SIZE+j]);
    }

}


