#include <string.h>
#include <stdio.h>


#define SIZE  8
int A[SIZE*SIZE], B[SIZE*SIZE], C[SIZE*SIZE];
#define C1 16//80
#define R1 8//8
#define C2 16//500
#define R2 C1

int WEIGHT_MATRIX[R1*C1];
int INPUT_MATRIX[R2*C2];
int OUTPUT_MATRIX[R1*C2];
int OUTPUT_MATRIX_EXP[R1*C2];

void gemm(){
	int i,j,k,ijk;

        i=0;j=0;k=0;
        for (ijk=0;ijk<R1*C1*C2/4; ijk++){
           #ifdef CGRA_COMPILER
           please_map_me();
           #endif
//printf("i,j,k: %d,%d,%d\n",i,j,k);
	   OUTPUT_MATRIX[i*C2+j] = OUTPUT_MATRIX[i*C2+j] + WEIGHT_MATRIX[i*C1+k]* INPUT_MATRIX[k*C2+j]+ WEIGHT_MATRIX[i*C1+k+1]* INPUT_MATRIX[(k+1)*C2+j]+ WEIGHT_MATRIX[i*C1+k+2]* INPUT_MATRIX[(k+2)*C2+j]+WEIGHT_MATRIX[i*C1+k+3]* INPUT_MATRIX[(k+3)*C2+j];
	   k=k+4;
			if(k+1 >= C1){
				k=0;
				++j;
			}
			if(j == C2){
  				j=0;
				++i;
			}
	}

}

void microspeech_conv_layer(){
	int i,j,k;

	for (i=0;i<R1; i++)
		for (j=0;j<C2; j++)
			for (k=0;k<C1; k++){
				OUTPUT_MATRIX_EXP[i*C2+j] += WEIGHT_MATRIX[i*C1+k]* INPUT_MATRIX[k*C2+j];
			}

}

void main(){

	int i,j;
	for (i=0;i<R1; i++)
		for (j=0; j<C1; j++) {
			WEIGHT_MATRIX[(i)*C1+(j)]= 2*i+1;
		}

	for (i=0;i<R2; i++)
		for (j=0; j<C2; j++) {
			INPUT_MATRIX[(i)*C2+(j)]= i*j+3;
		}


	microspeech_conv_layer();
//microspeech_conv_layer_flattened();
gemm();


	for (i=0;i<R1; i++)
		for (j=0; j<C2; j++) {
			if (OUTPUT_MATRIX[(i)*C2+(j)]!=OUTPUT_MATRIX_EXP[(i)*C2+(j)])
			{
				printf("INCORRECT %d,%d\n",OUTPUT_MATRIX_EXP[(i)*C2+(j)],OUTPUT_MATRIX[(i)*C2+(j)]);
			}else{
				printf("CORRECT %d,%d\n",OUTPUT_MATRIX_EXP[(i)*C2+(j)],OUTPUT_MATRIX[(i)*C2+(j)]);
			}
		}

}


