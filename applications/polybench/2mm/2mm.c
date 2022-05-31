#include <string.h>
#include <stdio.h>


#define SIZE  10
int n = SIZE;
int A[SIZE][SIZE], B[SIZE][SIZE], tmp[SIZE][SIZE];
int i = 0,j = 0 ;
void atax(){



   int i = 0,j = 0, k = 0, sum = 0, alpha = 10;

   
  for (i = 0; i < SIZE; i++)
    for (j = 0; j < SIZE; j++)
      
        for (k = 0; k < SIZE; ++k){
          #ifdef CGRA_COMPILER
           please_map_me();
           #endif
           sum += alpha * A[i][k] * B[k][j];
		       //A[i][j] = A[i][j] + B[i][4*k]* C[4*k][j] + B[i][4*k+1]* C[4*k+1][j]
			     //   + B[i][4*k+2]* C[4*k+2][j] + B[i][4*k+3]* C[4*k+3][j];
    }

    printf("%i\n", sum);
}

void main(){

int i,j;
for (i=0;i<n; i++)
   for (j=0; j<n; j++) {
      A[i][j]= i * j;
      B[i][j] = j+i;
      tmp[i][j] = j*i;
    }
    
atax();

for (i=0;i<n; i++)
   for (j=0; j<n; j++) {
      printf("%d\n", A[i][j]);
    }

}


