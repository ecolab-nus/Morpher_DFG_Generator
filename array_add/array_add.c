#include <string.h>
#include <stdio.h>


#define SIZE  10
int n = SIZE;
int A[SIZE], B[SIZE], C[SIZE];
int i,j;
void array_add(){



   int k;

   
        for (k=0;k<SIZE; k++){
          #ifdef CGRA_COMPILER
           please_map_me();
           #endif
           A[k] = B[k]+C[k];
		       //A[i][j] = A[i][j] + B[i][4*k]* C[4*k][j] + B[i][4*k+1]* C[4*k+1][j]
			     //   + B[i][4*k+2]* C[4*k+2][j] + B[i][4*k+3]* C[4*k+3][j];
            }


}

void main(){

int i,j;
for (i=0;i<n; i++){
      A[i]=0;
      B[i] = 2*i;
      C[i] = i*i;
    }
    
array_add();

for (i=0;i<n; i++){
      printf("%d\n", A[i]);
    }

}


