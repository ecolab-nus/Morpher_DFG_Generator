#include <string.h>
#include <stdio.h>


#define SIZE 8
int n = SIZE;
int A[SIZE*SIZE], B[SIZE*SIZE], C[SIZE*SIZE], temp[SIZE*SIZE];

void gemm(){



   int i0,j0,k0;

   for (i0=0;i0<SIZE; i0=i0+2){
      for (j0=0;j0<SIZE; j0=j0+2){
        for (k0=0;k0<SIZE; k0=k0+2){
                 #ifdef CGRA_COMPILER
                  please_map_me();
                  #endif
                  A[(i0+0)*SIZE+(j0+0)]=A[(i0+0)*SIZE+(j0+0)]+B[(i0+0)*SIZE+(k0+0)]*C[(k0+0)*SIZE+(j0+0)];
                  A[(i0+0)*SIZE+(j0+0)]=A[(i0+0)*SIZE+(j0+0)]+B[(i0+0)*SIZE+(k0+1)]*C[(k0+1)*SIZE+(j0+0)];
                  A[(i0+0)*SIZE+(j0+1)]=A[(i0+0)*SIZE+(j0+1)]+B[(i0+0)*SIZE+(k0+0)]*C[(k0+0)*SIZE+(j0+1)];
                  A[(i0+0)*SIZE+(j0+1)]=A[(i0+0)*SIZE+(j0+1)]+B[(i0+0)*SIZE+(k0+1)]*C[(k0+1)*SIZE+(j0+1)];
                  A[(i0+1)*SIZE+(j0+0)]=A[(i0+1)*SIZE+(j0+0)]+B[(i0+1)*SIZE+(k0+0)]*C[(k0+0)*SIZE+(j0+0)];
                  A[(i0+1)*SIZE+(j0+0)]=A[(i0+1)*SIZE+(j0+0)]+B[(i0+1)*SIZE+(k0+1)]*C[(k0+1)*SIZE+(j0+0)];
                  A[(i0+1)*SIZE+(j0+1)]=A[(i0+1)*SIZE+(j0+1)]+B[(i0+1)*SIZE+(k0+0)]*C[(k0+0)*SIZE+(j0+1)];
                  A[(i0+1)*SIZE+(j0+1)]=A[(i0+1)*SIZE+(j0+1)]+B[(i0+1)*SIZE+(k0+1)]*C[(k0+1)*SIZE+(j0+1)];
        }
     }
  }


}

void gemm_original(){



   int i,j,k;

   for (i=0;i<SIZE; i++)
      for (j=0;j<SIZE; j++)
        for (k=0;k<SIZE; k++){
           	A[(i)*SIZE+(j)] = A[(i)*SIZE+(j)] + B[(i)*SIZE+(k)]* C[(k)*SIZE+(j)];
            }


}

void main(){

int i,j;
for (i=0;i<SIZE; i++)
   for (j=0; j<SIZE; j++) {
      A[(i)*SIZE+(j)]= i;
      B[(i)*SIZE+(j)] = j+i;
      C[(i)*SIZE+(j)] = j*i;
    }
    
gemm();

for (i=0;i<SIZE; i++)
   for (j=0; j<SIZE; j++) {
      // printf("%d\n", A[i][j]);
      temp[(i)*SIZE+(j)] = A[(i)*SIZE+(j)];
    }

for (i=0;i<SIZE; i++)
   for (j=0; j<SIZE; j++) {
      A[(i)*SIZE+(j)]= i;
      B[(i)*SIZE+(j)] = j+i;
      C[(i)*SIZE+(j)] = j*i;
    }
    
gemm_original();

for (i=0;i<SIZE; i++)
   for (j=0; j<SIZE; j++) {
      if (A[(i)*SIZE+(j)]!=temp[(i)*SIZE+(j)])
      {
        printf("INCORRECT %d,%d\n",temp[(i)*SIZE+(j)],A[(i)*SIZE+(j)]);
      }else{
        printf("CORRECT %d,%d\n",temp[(i)*SIZE+(j)],A[(i)*SIZE+(j)]);
      }
    }

}


