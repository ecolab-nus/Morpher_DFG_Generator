#include <string.h>
#include <stdio.h>


#define SIZE  32
int n = SIZE;
int A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE], temp[SIZE][SIZE];
int i,j;
void gemm(){



   int i,j,k;

   for (i=0;i<SIZE; i++)
      for (j=0; j<SIZE; j++)
        for (k=0;k<SIZE/16; k++){
          #ifdef CGRA_COMPILER
           please_map_me();
           #endif
		       A[i][j] = A[i][j] + B[i][16*k]* C[16*k][j] + B[i][16*k+1]* C[16*k+1][j]
			       + B[i][16*k+2]* C[16*k+2][j] + B[i][16*k+3]* C[16*k+3][j] + B[i][16*k+4]* C[16*k+4][j] + B[i][16*k+5]* C[16*k+5][j]
			       + B[i][16*k+6]* C[16*k+6][j] + B[i][16*k+7]* C[16*k+7][j] + B[i][16*k+8]* C[16*k+8][j] + B[i][16*k+9]* C[16*k+9][j]
             + B[i][16*k+10]* C[16*k+10][j] + B[i][16*k+11]* C[16*k+11][j] + B[i][16*k+12]* C[16*k+12][j] + B[i][16*k+13]* C[16*k+13][j]
             + B[i][16*k+14]* C[16*k+14][j] + B[i][16*k+15]* C[16*k+15][j];
            }


}

void gemm_original(){



   int i,j,k;

   for (i=0;i<SIZE; i++)
      for (j=0; j<SIZE; j++)
        for (k=0;k<SIZE; k++){
           A[i][j] = A[i][j] + B[i][k]* C[k][j];
           //A[i][j] = A[i][j] + B[i][4*k]* C[4*k][j] + B[i][4*k+1]* C[4*k+1][j]
           //   + B[i][4*k+2]* C[4*k+2][j] + B[i][4*k+3]* C[4*k+3][j];
            }


}

void main(){

int i,j;
for (i=0;i<n; i++)
   for (j=0; j<n; j++) {
      A[i][j]=0;
      B[i][j] = j+i;
      C[i][j] = j*i;
    }
    
gemm();

for (i=0;i<n; i++)
   for (j=0; j<n; j++) {
      // printf("%d\n", A[i][j]);
      temp[i][j] = A[i][j];
    }

for (i=0;i<n; i++)
   for (j=0; j<n; j++) {
      A[i][j]=0;
      B[i][j] = j+i;
      C[i][j] = j*i;
    }
    
gemm_original();

for (i=0;i<n; i++)
   for (j=0; j<n; j++) {
      if (A[i][j]!=temp[i][j])
      {
        printf("INCORRECT %d,%d\n",temp[i][j],A[i][j]);
      }
    }

}


