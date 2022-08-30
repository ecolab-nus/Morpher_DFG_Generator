	#include <string.h>
	#include <stdio.h>


	#define SIZE  8
	#define SIZESIZESIZE4 128 //8*8*8/4
	int n = SIZE;
	int A[SIZE*SIZE], B[SIZE*SIZE], C[SIZE*SIZE];
	int i,j;

	__attribute__((noinline))
	void gemm(){


		int i,j,k,ijk;

	    i=0;j=0;k=0;
	    for (ijk=0;ijk<SIZESIZESIZE4; ijk++){
	       	#ifdef CGRA_COMPILER
	       	please_map_me();
	       	#endif
			A[i*SIZE+j] = A[i*SIZE+j] + B[i*SIZE+k]* C[k*SIZE+j]+ B[i*SIZE+k+1]* C[(k+1)*SIZE+j]+ B[i*SIZE+k+2]* C[(k+2)*SIZE+j]+B[i*SIZE+k+3]* C[(k+3)*SIZE+j];
			k=k+4;
			if(k+1 >= SIZE){
				k=0;
				++j;
			}
			if(j == SIZE){
	  			j=0;
				++i;
			}
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


