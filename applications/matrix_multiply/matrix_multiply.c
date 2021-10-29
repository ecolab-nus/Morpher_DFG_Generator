#include <string.h>
#include <stdio.h>


//#define SIZE  8
//int A[SIZE*SIZE], B[SIZE*SIZE], C[SIZE*SIZE];
#define C1 16//80
#define R1 8//8
#define C2 16//500
#define R2 C1

int WEIGHT_MATRIX[R1*C1];
int INPUT_MATRIX[R2*C2];
int OUTPUT_MATRIX[R1*C2];
int OUTPUT_MATRIX_EXP[R1*C2];

#define SIZE 16
int n = SIZE;
int A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE], temp[SIZE][SIZE];
int i,j;

void matrix_multiply(){
	int i,j,k,ijk;

        i=0;j=0;k=0;
        for (ijk=0;ijk<R1*C1*C2/4; ijk++){
           #ifdef CGRA_COMPILER
           please_map_me();
           #endif
//printf("i,j,k: %d,%d,%d\n",i,j,k);
	   OUTPUT_MATRIX[i*C2+j] = WEIGHT_MATRIX[i*C1+k]* INPUT_MATRIX[k*C2+j]+ WEIGHT_MATRIX[i*C1+k+1]* INPUT_MATRIX[(k+1)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+2]* INPUT_MATRIX[(k+2)*C2+j]+WEIGHT_MATRIX[i*C1+k+3]* INPUT_MATRIX[(k+3)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+4]* INPUT_MATRIX[(k+4)*C2+j]+WEIGHT_MATRIX[i*C1+k+5]* INPUT_MATRIX[(k+5)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+6]* INPUT_MATRIX[(k+6)*C2+j]+WEIGHT_MATRIX[i*C1+k+7]* INPUT_MATRIX[(k+7)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+8]* INPUT_MATRIX[(k+8)*C2+j]+WEIGHT_MATRIX[i*C1+k+9]* INPUT_MATRIX[(k+9)*C2+j]+
	   						WEIGHT_MATRIX[i*C1+k+10]* INPUT_MATRIX[(k+10)*C2+j]+ WEIGHT_MATRIX[i*C1+k+11]* INPUT_MATRIX[(k+11)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+12]* INPUT_MATRIX[(k+12)*C2+j]+WEIGHT_MATRIX[i*C1+k+13]* INPUT_MATRIX[(k+13)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+14]* INPUT_MATRIX[(k+14)*C2+j]+WEIGHT_MATRIX[i*C1+k+15]* INPUT_MATRIX[(k+15)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+16]* INPUT_MATRIX[(k+16)*C2+j]+WEIGHT_MATRIX[i*C1+k+17]* INPUT_MATRIX[(k+17)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+18]* INPUT_MATRIX[(k+18)*C2+j];
	   k=k+19;
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


void matrix_multiply_unrolled_2(){
	int i,j,k,ijk;

        i=0;j=0;k=0;
        for (ijk=0;ijk<R1*C1*C2/4; ijk++){
           #ifdef CGRA_COMPILER
           please_map_me();
           #endif
//printf("i,j,k: %d,%d,%d\n",i,j,k);
	   OUTPUT_MATRIX[i*C2+j] = WEIGHT_MATRIX[i*C1+k]* INPUT_MATRIX[k*C2+j]+ WEIGHT_MATRIX[i*C1+k+1]* INPUT_MATRIX[(k+1)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+2]* INPUT_MATRIX[(k+2)*C2+j]+WEIGHT_MATRIX[i*C1+k+3]* INPUT_MATRIX[(k+3)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+4]* INPUT_MATRIX[(k+4)*C2+j]+WEIGHT_MATRIX[i*C1+k+5]* INPUT_MATRIX[(k+5)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+6]* INPUT_MATRIX[(k+6)*C2+j]+WEIGHT_MATRIX[i*C1+k+7]* INPUT_MATRIX[(k+7)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+8]* INPUT_MATRIX[(k+8)*C2+j]+WEIGHT_MATRIX[i*C1+k+9]* INPUT_MATRIX[(k+9)*C2+j]+
	   						WEIGHT_MATRIX[i*C1+k+10]* INPUT_MATRIX[(k+10)*C2+j]+ WEIGHT_MATRIX[i*C1+k+11]* INPUT_MATRIX[(k+11)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+12]* INPUT_MATRIX[(k+12)*C2+j]+WEIGHT_MATRIX[i*C1+k+13]* INPUT_MATRIX[(k+13)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+14]* INPUT_MATRIX[(k+14)*C2+j]+WEIGHT_MATRIX[i*C1+k+15]* INPUT_MATRIX[(k+15)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+16]* INPUT_MATRIX[(k+16)*C2+j]+WEIGHT_MATRIX[i*C1+k+17]* INPUT_MATRIX[(k+17)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+18]* INPUT_MATRIX[(k+18)*C2+j]+WEIGHT_MATRIX[i*C1+k+19]* INPUT_MATRIX[(k+19)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+20]* INPUT_MATRIX[(k+20)*C2+j]+WEIGHT_MATRIX[i*C1+k+21]* INPUT_MATRIX[(k+21)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+22]* INPUT_MATRIX[(k+22)*C2+j]+WEIGHT_MATRIX[i*C1+k+23]* INPUT_MATRIX[(k+23)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+24]* INPUT_MATRIX[(k+24)*C2+j]+WEIGHT_MATRIX[i*C1+k+25]* INPUT_MATRIX[(k+25)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+26]* INPUT_MATRIX[(k+26)*C2+j]+WEIGHT_MATRIX[i*C1+k+27]* INPUT_MATRIX[(k+27)*C2+j]+
	   						WEIGHT_MATRIX[i*C1+k+28]* INPUT_MATRIX[(k+28)*C2+j]+ WEIGHT_MATRIX[i*C1+k+29]* INPUT_MATRIX[(k+29)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+30]* INPUT_MATRIX[(k+30)*C2+j]+WEIGHT_MATRIX[i*C1+k+31]* INPUT_MATRIX[(k+31)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+32]* INPUT_MATRIX[(k+32)*C2+j]+WEIGHT_MATRIX[i*C1+k+33]* INPUT_MATRIX[(k+33)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+34]* INPUT_MATRIX[(k+34)*C2+j]+WEIGHT_MATRIX[i*C1+k+35]* INPUT_MATRIX[(k+35)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+36]* INPUT_MATRIX[(k+36)*C2+j]+WEIGHT_MATRIX[i*C1+k+37]* INPUT_MATRIX[(k+37)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+38]* INPUT_MATRIX[(k+38)*C2+j]+WEIGHT_MATRIX[i*C1+k+39]* INPUT_MATRIX[(k+39)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+40]* INPUT_MATRIX[(k+40)*C2+j]+WEIGHT_MATRIX[i*C1+k+41]* INPUT_MATRIX[(k+41)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+42]* INPUT_MATRIX[(k+42)*C2+j]+WEIGHT_MATRIX[i*C1+k+43]* INPUT_MATRIX[(k+43)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+44]* INPUT_MATRIX[(k+44)*C2+j]+WEIGHT_MATRIX[i*C1+k+45]* INPUT_MATRIX[(k+45)*C2+j]+ 
	   						WEIGHT_MATRIX[i*C1+k+46]* INPUT_MATRIX[(k+46)*C2+j]+WEIGHT_MATRIX[i*C1+k+47]* INPUT_MATRIX[(k+47)*C2+j];
	   k=k+48;
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

void general_matrix_multiply_block_4(){
	   int i0,i1,j0,j1,k0,k1;

	 for (i0=0;i0<SIZE; i0=i0+4)
      for (j0=0; j0<SIZE; j0=j0+4)
        for (k0=0;k0<SIZE; k0=k0+4){
          // printf("\n");
          //  for (i1=i0;i1<i0+3; i1++)
          //    for (j1=j0; j1<j0+3; j1++)
          //      for (k1=k0;k1<k0+3; k1++){
          //        #ifdef CGRA_COMPILER
          //         please_map_me();
          //         #endif
          //         A[i1][j1] = A[i1][j1] + B[i1][k1]* C[k1][j1];
          //         printf("A[i0+%d][j0+%d]=A[i0+%d][j0+%d]+B[i0+%d][k0+%d]*C[k0+%d][j0+%d];\n",i1,j1,i1,j1,i1,k1,k1,j1);
		        //       //A[i][j] = A[i][j] + B[i][4*k]* C[4*k][j] + B[i][4*k+1]* C[4*k+1][j]
			       //       //   + B[i][4*k+2]* C[4*k+2][j] + B[i][4*k+3]* C[4*k+3][j];
          //      }

                 #ifdef CGRA_COMPILER
                  please_map_me();
                  #endif
//           A[i0+0][j0+0]=A[i0+0][j0+0]+B[i0+0][k0+0]*C[k0+0][j0+0];
// A[i0+0][j0+0]=A[i0+0][j0+0]+B[i0+0][k0+1]*C[k0+1][j0+0];
// A[i0+0][j0+0]=A[i0+0][j0+0]+B[i0+0][k0+2]*C[k0+2][j0+0];
// A[i0+0][j0+1]=A[i0+0][j0+1]+B[i0+0][k0+0]*C[k0+0][j0+1];
// A[i0+0][j0+1]=A[i0+0][j0+1]+B[i0+0][k0+1]*C[k0+1][j0+1];
// A[i0+0][j0+1]=A[i0+0][j0+1]+B[i0+0][k0+2]*C[k0+2][j0+1];
// A[i0+0][j0+2]=A[i0+0][j0+2]+B[i0+0][k0+0]*C[k0+0][j0+2];
// A[i0+0][j0+2]=A[i0+0][j0+2]+B[i0+0][k0+1]*C[k0+1][j0+2];
// A[i0+0][j0+2]=A[i0+0][j0+2]+B[i0+0][k0+2]*C[k0+2][j0+2];
// A[i0+1][j0+0]=A[i0+1][j0+0]+B[i0+1][k0+0]*C[k0+0][j0+0];
// A[i0+1][j0+0]=A[i0+1][j0+0]+B[i0+1][k0+1]*C[k0+1][j0+0];
// A[i0+1][j0+0]=A[i0+1][j0+0]+B[i0+1][k0+2]*C[k0+2][j0+0];
// A[i0+1][j0+1]=A[i0+1][j0+1]+B[i0+1][k0+0]*C[k0+0][j0+1];
// A[i0+1][j0+1]=A[i0+1][j0+1]+B[i0+1][k0+1]*C[k0+1][j0+1];
// A[i0+1][j0+1]=A[i0+1][j0+1]+B[i0+1][k0+2]*C[k0+2][j0+1];
// A[i0+1][j0+2]=A[i0+1][j0+2]+B[i0+1][k0+0]*C[k0+0][j0+2];
// A[i0+1][j0+2]=A[i0+1][j0+2]+B[i0+1][k0+1]*C[k0+1][j0+2];
// A[i0+1][j0+2]=A[i0+1][j0+2]+B[i0+1][k0+2]*C[k0+2][j0+2];
// A[i0+2][j0+0]=A[i0+2][j0+0]+B[i0+2][k0+0]*C[k0+0][j0+0];
// A[i0+2][j0+0]=A[i0+2][j0+0]+B[i0+2][k0+1]*C[k0+1][j0+0];
// A[i0+2][j0+0]=A[i0+2][j0+0]+B[i0+2][k0+2]*C[k0+2][j0+0];
// A[i0+2][j0+1]=A[i0+2][j0+1]+B[i0+2][k0+0]*C[k0+0][j0+1];
// A[i0+2][j0+1]=A[i0+2][j0+1]+B[i0+2][k0+1]*C[k0+1][j0+1];
// A[i0+2][j0+1]=A[i0+2][j0+1]+B[i0+2][k0+2]*C[k0+2][j0+1];
// A[i0+2][j0+2]=A[i0+2][j0+2]+B[i0+2][k0+0]*C[k0+0][j0+2];
// A[i0+2][j0+2]=A[i0+2][j0+2]+B[i0+2][k0+1]*C[k0+1][j0+2];
// A[i0+2][j0+2]=A[i0+2][j0+2]+B[i0+2][k0+2]*C[k0+2][j0+2];

          A[i0+0][j0+0]=A[i0+0][j0+0]+B[i0+0][k0+0]*C[k0+0][j0+0];
          A[i0+0][j0+0]=A[i0+0][j0+0]+B[i0+0][k0+1]*C[k0+1][j0+0];
          A[i0+0][j0+0]=A[i0+0][j0+0]+B[i0+0][k0+2]*C[k0+2][j0+0];
          A[i0+0][j0+0]=A[i0+0][j0+0]+B[i0+0][k0+3]*C[k0+3][j0+0];
          A[i0+0][j0+1]=A[i0+0][j0+1]+B[i0+0][k0+0]*C[k0+0][j0+1];
          A[i0+0][j0+1]=A[i0+0][j0+1]+B[i0+0][k0+1]*C[k0+1][j0+1];
          A[i0+0][j0+1]=A[i0+0][j0+1]+B[i0+0][k0+2]*C[k0+2][j0+1];
          A[i0+0][j0+1]=A[i0+0][j0+1]+B[i0+0][k0+3]*C[k0+3][j0+1];
          A[i0+0][j0+2]=A[i0+0][j0+2]+B[i0+0][k0+0]*C[k0+0][j0+2];
          A[i0+0][j0+2]=A[i0+0][j0+2]+B[i0+0][k0+1]*C[k0+1][j0+2];
          A[i0+0][j0+2]=A[i0+0][j0+2]+B[i0+0][k0+2]*C[k0+2][j0+2];
          A[i0+0][j0+2]=A[i0+0][j0+2]+B[i0+0][k0+3]*C[k0+3][j0+2];
          A[i0+0][j0+3]=A[i0+0][j0+3]+B[i0+0][k0+0]*C[k0+0][j0+3];
          A[i0+0][j0+3]=A[i0+0][j0+3]+B[i0+0][k0+1]*C[k0+1][j0+3];
          A[i0+0][j0+3]=A[i0+0][j0+3]+B[i0+0][k0+2]*C[k0+2][j0+3];
          A[i0+0][j0+3]=A[i0+0][j0+3]+B[i0+0][k0+3]*C[k0+3][j0+3];
          A[i0+1][j0+0]=A[i0+1][j0+0]+B[i0+1][k0+0]*C[k0+0][j0+0];
          A[i0+1][j0+0]=A[i0+1][j0+0]+B[i0+1][k0+1]*C[k0+1][j0+0];
          A[i0+1][j0+0]=A[i0+1][j0+0]+B[i0+1][k0+2]*C[k0+2][j0+0];
          A[i0+1][j0+0]=A[i0+1][j0+0]+B[i0+1][k0+3]*C[k0+3][j0+0];
          A[i0+1][j0+1]=A[i0+1][j0+1]+B[i0+1][k0+0]*C[k0+0][j0+1];
          A[i0+1][j0+1]=A[i0+1][j0+1]+B[i0+1][k0+1]*C[k0+1][j0+1];
          A[i0+1][j0+1]=A[i0+1][j0+1]+B[i0+1][k0+2]*C[k0+2][j0+1];
          A[i0+1][j0+1]=A[i0+1][j0+1]+B[i0+1][k0+3]*C[k0+3][j0+1];
          A[i0+1][j0+2]=A[i0+1][j0+2]+B[i0+1][k0+0]*C[k0+0][j0+2];
          A[i0+1][j0+2]=A[i0+1][j0+2]+B[i0+1][k0+1]*C[k0+1][j0+2];
          A[i0+1][j0+2]=A[i0+1][j0+2]+B[i0+1][k0+2]*C[k0+2][j0+2];
          A[i0+1][j0+2]=A[i0+1][j0+2]+B[i0+1][k0+3]*C[k0+3][j0+2];
          A[i0+1][j0+3]=A[i0+1][j0+3]+B[i0+1][k0+0]*C[k0+0][j0+3];
          A[i0+1][j0+3]=A[i0+1][j0+3]+B[i0+1][k0+1]*C[k0+1][j0+3];
          A[i0+1][j0+3]=A[i0+1][j0+3]+B[i0+1][k0+2]*C[k0+2][j0+3];
          A[i0+1][j0+3]=A[i0+1][j0+3]+B[i0+1][k0+3]*C[k0+3][j0+3];
          A[i0+2][j0+0]=A[i0+2][j0+0]+B[i0+2][k0+0]*C[k0+0][j0+0];
          A[i0+2][j0+0]=A[i0+2][j0+0]+B[i0+2][k0+1]*C[k0+1][j0+0];
          A[i0+2][j0+0]=A[i0+2][j0+0]+B[i0+2][k0+2]*C[k0+2][j0+0];
          A[i0+2][j0+0]=A[i0+2][j0+0]+B[i0+2][k0+3]*C[k0+3][j0+0];
          A[i0+2][j0+1]=A[i0+2][j0+1]+B[i0+2][k0+0]*C[k0+0][j0+1];
          A[i0+2][j0+1]=A[i0+2][j0+1]+B[i0+2][k0+1]*C[k0+1][j0+1];
          A[i0+2][j0+1]=A[i0+2][j0+1]+B[i0+2][k0+2]*C[k0+2][j0+1];
          A[i0+2][j0+1]=A[i0+2][j0+1]+B[i0+2][k0+3]*C[k0+3][j0+1];
          A[i0+2][j0+2]=A[i0+2][j0+2]+B[i0+2][k0+0]*C[k0+0][j0+2];
          A[i0+2][j0+2]=A[i0+2][j0+2]+B[i0+2][k0+1]*C[k0+1][j0+2];
          A[i0+2][j0+2]=A[i0+2][j0+2]+B[i0+2][k0+2]*C[k0+2][j0+2];
          A[i0+2][j0+2]=A[i0+2][j0+2]+B[i0+2][k0+3]*C[k0+3][j0+2];
          A[i0+2][j0+3]=A[i0+2][j0+3]+B[i0+2][k0+0]*C[k0+0][j0+3];
          A[i0+2][j0+3]=A[i0+2][j0+3]+B[i0+2][k0+1]*C[k0+1][j0+3];
          A[i0+2][j0+3]=A[i0+2][j0+3]+B[i0+2][k0+2]*C[k0+2][j0+3];
          A[i0+2][j0+3]=A[i0+2][j0+3]+B[i0+2][k0+3]*C[k0+3][j0+3];
          A[i0+3][j0+0]=A[i0+3][j0+0]+B[i0+3][k0+0]*C[k0+0][j0+0];
          A[i0+3][j0+0]=A[i0+3][j0+0]+B[i0+3][k0+1]*C[k0+1][j0+0];
          A[i0+3][j0+0]=A[i0+3][j0+0]+B[i0+3][k0+2]*C[k0+2][j0+0];
          A[i0+3][j0+0]=A[i0+3][j0+0]+B[i0+3][k0+3]*C[k0+3][j0+0];
          A[i0+3][j0+1]=A[i0+3][j0+1]+B[i0+3][k0+0]*C[k0+0][j0+1];
          A[i0+3][j0+1]=A[i0+3][j0+1]+B[i0+3][k0+1]*C[k0+1][j0+1];
          A[i0+3][j0+1]=A[i0+3][j0+1]+B[i0+3][k0+2]*C[k0+2][j0+1];
          A[i0+3][j0+1]=A[i0+3][j0+1]+B[i0+3][k0+3]*C[k0+3][j0+1];
          A[i0+3][j0+2]=A[i0+3][j0+2]+B[i0+3][k0+0]*C[k0+0][j0+2];
          A[i0+3][j0+2]=A[i0+3][j0+2]+B[i0+3][k0+1]*C[k0+1][j0+2];
          A[i0+3][j0+2]=A[i0+3][j0+2]+B[i0+3][k0+2]*C[k0+2][j0+2];
          A[i0+3][j0+2]=A[i0+3][j0+2]+B[i0+3][k0+3]*C[k0+3][j0+2];
          A[i0+3][j0+3]=A[i0+3][j0+3]+B[i0+3][k0+0]*C[k0+0][j0+3];
          A[i0+3][j0+3]=A[i0+3][j0+3]+B[i0+3][k0+1]*C[k0+1][j0+3];
          A[i0+3][j0+3]=A[i0+3][j0+3]+B[i0+3][k0+2]*C[k0+2][j0+3];
          A[i0+3][j0+3]=A[i0+3][j0+3]+B[i0+3][k0+3]*C[k0+3][j0+3];



        }

}
void general_matrix_multiply_block_5(){
	   int i0,i1,j0,j1,k0,k1;

	 for (i0=0;i0<SIZE; i0=i0+5)
      for (j0=0; j0<SIZE; j0=j0+5)
        for (k0=0;k0<SIZE; k0=k0+5){

                 #ifdef CGRA_COMPILER
                  please_map_me();
                  #endif
                A[i0+0][j0+0] =  A[i0+0][j0+0] + B[i0+0][k0+0] * C[k0+0][j0+0];
				A[i0+0][j0+0] =  A[i0+0][j0+0] + B[i0+0][k0+1] * C[k0+1][j0+0];
				A[i0+0][j0+0] =  A[i0+0][j0+0] + B[i0+0][k0+2] * C[k0+2][j0+0];
				A[i0+0][j0+0] =  A[i0+0][j0+0] + B[i0+0][k0+3] * C[k0+3][j0+0];
				A[i0+0][j0+0] =  A[i0+0][j0+0] + B[i0+0][k0+4] * C[k0+4][j0+0];
				A[i0+0][j0+1] =  A[i0+0][j0+1] + B[i0+0][k0+0] * C[k0+0][j0+1];
				A[i0+0][j0+1] =  A[i0+0][j0+1] + B[i0+0][k0+1] * C[k0+1][j0+1];
				A[i0+0][j0+1] =  A[i0+0][j0+1] + B[i0+0][k0+2] * C[k0+2][j0+1];
				A[i0+0][j0+1] =  A[i0+0][j0+1] + B[i0+0][k0+3] * C[k0+3][j0+1];
				A[i0+0][j0+1] =  A[i0+0][j0+1] + B[i0+0][k0+4] * C[k0+4][j0+1];
				A[i0+0][j0+2] =  A[i0+0][j0+2] + B[i0+0][k0+0] * C[k0+0][j0+2];
				A[i0+0][j0+2] =  A[i0+0][j0+2] + B[i0+0][k0+1] * C[k0+1][j0+2];
				A[i0+0][j0+2] =  A[i0+0][j0+2] + B[i0+0][k0+2] * C[k0+2][j0+2];
				A[i0+0][j0+2] =  A[i0+0][j0+2] + B[i0+0][k0+3] * C[k0+3][j0+2];
				A[i0+0][j0+2] =  A[i0+0][j0+2] + B[i0+0][k0+4] * C[k0+4][j0+2];
				A[i0+0][j0+3] =  A[i0+0][j0+3] + B[i0+0][k0+0] * C[k0+0][j0+3];
				A[i0+0][j0+3] =  A[i0+0][j0+3] + B[i0+0][k0+1] * C[k0+1][j0+3];
				A[i0+0][j0+3] =  A[i0+0][j0+3] + B[i0+0][k0+2] * C[k0+2][j0+3];
				A[i0+0][j0+3] =  A[i0+0][j0+3] + B[i0+0][k0+3] * C[k0+3][j0+3];
				A[i0+0][j0+3] =  A[i0+0][j0+3] + B[i0+0][k0+4] * C[k0+4][j0+3];
				A[i0+0][j0+4] =  A[i0+0][j0+4] + B[i0+0][k0+0] * C[k0+0][j0+4];
				A[i0+0][j0+4] =  A[i0+0][j0+4] + B[i0+0][k0+1] * C[k0+1][j0+4];
				A[i0+0][j0+4] =  A[i0+0][j0+4] + B[i0+0][k0+2] * C[k0+2][j0+4];
				A[i0+0][j0+4] =  A[i0+0][j0+4] + B[i0+0][k0+3] * C[k0+3][j0+4];
				A[i0+0][j0+4] =  A[i0+0][j0+4] + B[i0+0][k0+4] * C[k0+4][j0+4];
				A[i0+1][j0+0] =  A[i0+1][j0+0] + B[i0+1][k0+0] * C[k0+0][j0+0];
				A[i0+1][j0+0] =  A[i0+1][j0+0] + B[i0+1][k0+1] * C[k0+1][j0+0];
				A[i0+1][j0+0] =  A[i0+1][j0+0] + B[i0+1][k0+2] * C[k0+2][j0+0];
				A[i0+1][j0+0] =  A[i0+1][j0+0] + B[i0+1][k0+3] * C[k0+3][j0+0];
				A[i0+1][j0+0] =  A[i0+1][j0+0] + B[i0+1][k0+4] * C[k0+4][j0+0];
				A[i0+1][j0+1] =  A[i0+1][j0+1] + B[i0+1][k0+0] * C[k0+0][j0+1];
				A[i0+1][j0+1] =  A[i0+1][j0+1] + B[i0+1][k0+1] * C[k0+1][j0+1];
				A[i0+1][j0+1] =  A[i0+1][j0+1] + B[i0+1][k0+2] * C[k0+2][j0+1];
				A[i0+1][j0+1] =  A[i0+1][j0+1] + B[i0+1][k0+3] * C[k0+3][j0+1];
				A[i0+1][j0+1] =  A[i0+1][j0+1] + B[i0+1][k0+4] * C[k0+4][j0+1];
				A[i0+1][j0+2] =  A[i0+1][j0+2] + B[i0+1][k0+0] * C[k0+0][j0+2];
				A[i0+1][j0+2] =  A[i0+1][j0+2] + B[i0+1][k0+1] * C[k0+1][j0+2];
				A[i0+1][j0+2] =  A[i0+1][j0+2] + B[i0+1][k0+2] * C[k0+2][j0+2];
				A[i0+1][j0+2] =  A[i0+1][j0+2] + B[i0+1][k0+3] * C[k0+3][j0+2];
				A[i0+1][j0+2] =  A[i0+1][j0+2] + B[i0+1][k0+4] * C[k0+4][j0+2];
				A[i0+1][j0+3] =  A[i0+1][j0+3] + B[i0+1][k0+0] * C[k0+0][j0+3];
				A[i0+1][j0+3] =  A[i0+1][j0+3] + B[i0+1][k0+1] * C[k0+1][j0+3];
				A[i0+1][j0+3] =  A[i0+1][j0+3] + B[i0+1][k0+2] * C[k0+2][j0+3];
				A[i0+1][j0+3] =  A[i0+1][j0+3] + B[i0+1][k0+3] * C[k0+3][j0+3];
				A[i0+1][j0+3] =  A[i0+1][j0+3] + B[i0+1][k0+4] * C[k0+4][j0+3];
				A[i0+1][j0+4] =  A[i0+1][j0+4] + B[i0+1][k0+0] * C[k0+0][j0+4];
				A[i0+1][j0+4] =  A[i0+1][j0+4] + B[i0+1][k0+1] * C[k0+1][j0+4];
				A[i0+1][j0+4] =  A[i0+1][j0+4] + B[i0+1][k0+2] * C[k0+2][j0+4];
				A[i0+1][j0+4] =  A[i0+1][j0+4] + B[i0+1][k0+3] * C[k0+3][j0+4];
				A[i0+1][j0+4] =  A[i0+1][j0+4] + B[i0+1][k0+4] * C[k0+4][j0+4];
				A[i0+2][j0+0] =  A[i0+2][j0+0] + B[i0+2][k0+0] * C[k0+0][j0+0];
				A[i0+2][j0+0] =  A[i0+2][j0+0] + B[i0+2][k0+1] * C[k0+1][j0+0];
				A[i0+2][j0+0] =  A[i0+2][j0+0] + B[i0+2][k0+2] * C[k0+2][j0+0];
				A[i0+2][j0+0] =  A[i0+2][j0+0] + B[i0+2][k0+3] * C[k0+3][j0+0];
				A[i0+2][j0+0] =  A[i0+2][j0+0] + B[i0+2][k0+4] * C[k0+4][j0+0];
				A[i0+2][j0+1] =  A[i0+2][j0+1] + B[i0+2][k0+0] * C[k0+0][j0+1];
				A[i0+2][j0+1] =  A[i0+2][j0+1] + B[i0+2][k0+1] * C[k0+1][j0+1];
				A[i0+2][j0+1] =  A[i0+2][j0+1] + B[i0+2][k0+2] * C[k0+2][j0+1];
				A[i0+2][j0+1] =  A[i0+2][j0+1] + B[i0+2][k0+3] * C[k0+3][j0+1];
				A[i0+2][j0+1] =  A[i0+2][j0+1] + B[i0+2][k0+4] * C[k0+4][j0+1];
				A[i0+2][j0+2] =  A[i0+2][j0+2] + B[i0+2][k0+0] * C[k0+0][j0+2];
				A[i0+2][j0+2] =  A[i0+2][j0+2] + B[i0+2][k0+1] * C[k0+1][j0+2];
				A[i0+2][j0+2] =  A[i0+2][j0+2] + B[i0+2][k0+2] * C[k0+2][j0+2];
				A[i0+2][j0+2] =  A[i0+2][j0+2] + B[i0+2][k0+3] * C[k0+3][j0+2];
				A[i0+2][j0+2] =  A[i0+2][j0+2] + B[i0+2][k0+4] * C[k0+4][j0+2];
				A[i0+2][j0+3] =  A[i0+2][j0+3] + B[i0+2][k0+0] * C[k0+0][j0+3];
				A[i0+2][j0+3] =  A[i0+2][j0+3] + B[i0+2][k0+1] * C[k0+1][j0+3];
				A[i0+2][j0+3] =  A[i0+2][j0+3] + B[i0+2][k0+2] * C[k0+2][j0+3];
				A[i0+2][j0+3] =  A[i0+2][j0+3] + B[i0+2][k0+3] * C[k0+3][j0+3];
				A[i0+2][j0+3] =  A[i0+2][j0+3] + B[i0+2][k0+4] * C[k0+4][j0+3];
				A[i0+2][j0+4] =  A[i0+2][j0+4] + B[i0+2][k0+0] * C[k0+0][j0+4];
				A[i0+2][j0+4] =  A[i0+2][j0+4] + B[i0+2][k0+1] * C[k0+1][j0+4];
				A[i0+2][j0+4] =  A[i0+2][j0+4] + B[i0+2][k0+2] * C[k0+2][j0+4];
				A[i0+2][j0+4] =  A[i0+2][j0+4] + B[i0+2][k0+3] * C[k0+3][j0+4];
				A[i0+2][j0+4] =  A[i0+2][j0+4] + B[i0+2][k0+4] * C[k0+4][j0+4];
				A[i0+3][j0+0] =  A[i0+3][j0+0] + B[i0+3][k0+0] * C[k0+0][j0+0];
				A[i0+3][j0+0] =  A[i0+3][j0+0] + B[i0+3][k0+1] * C[k0+1][j0+0];
				A[i0+3][j0+0] =  A[i0+3][j0+0] + B[i0+3][k0+2] * C[k0+2][j0+0];
				A[i0+3][j0+0] =  A[i0+3][j0+0] + B[i0+3][k0+3] * C[k0+3][j0+0];
				A[i0+3][j0+0] =  A[i0+3][j0+0] + B[i0+3][k0+4] * C[k0+4][j0+0];
				A[i0+3][j0+1] =  A[i0+3][j0+1] + B[i0+3][k0+0] * C[k0+0][j0+1];
				A[i0+3][j0+1] =  A[i0+3][j0+1] + B[i0+3][k0+1] * C[k0+1][j0+1];
				A[i0+3][j0+1] =  A[i0+3][j0+1] + B[i0+3][k0+2] * C[k0+2][j0+1];
				A[i0+3][j0+1] =  A[i0+3][j0+1] + B[i0+3][k0+3] * C[k0+3][j0+1];
				A[i0+3][j0+1] =  A[i0+3][j0+1] + B[i0+3][k0+4] * C[k0+4][j0+1];
				A[i0+3][j0+2] =  A[i0+3][j0+2] + B[i0+3][k0+0] * C[k0+0][j0+2];
				A[i0+3][j0+2] =  A[i0+3][j0+2] + B[i0+3][k0+1] * C[k0+1][j0+2];
				A[i0+3][j0+2] =  A[i0+3][j0+2] + B[i0+3][k0+2] * C[k0+2][j0+2];
				A[i0+3][j0+2] =  A[i0+3][j0+2] + B[i0+3][k0+3] * C[k0+3][j0+2];
				A[i0+3][j0+2] =  A[i0+3][j0+2] + B[i0+3][k0+4] * C[k0+4][j0+2];
				A[i0+3][j0+3] =  A[i0+3][j0+3] + B[i0+3][k0+0] * C[k0+0][j0+3];
				A[i0+3][j0+3] =  A[i0+3][j0+3] + B[i0+3][k0+1] * C[k0+1][j0+3];
				A[i0+3][j0+3] =  A[i0+3][j0+3] + B[i0+3][k0+2] * C[k0+2][j0+3];
				A[i0+3][j0+3] =  A[i0+3][j0+3] + B[i0+3][k0+3] * C[k0+3][j0+3];
				A[i0+3][j0+3] =  A[i0+3][j0+3] + B[i0+3][k0+4] * C[k0+4][j0+3];
				A[i0+3][j0+4] =  A[i0+3][j0+4] + B[i0+3][k0+0] * C[k0+0][j0+4];
				A[i0+3][j0+4] =  A[i0+3][j0+4] + B[i0+3][k0+1] * C[k0+1][j0+4];
				A[i0+3][j0+4] =  A[i0+3][j0+4] + B[i0+3][k0+2] * C[k0+2][j0+4];
				A[i0+3][j0+4] =  A[i0+3][j0+4] + B[i0+3][k0+3] * C[k0+3][j0+4];
				A[i0+3][j0+4] =  A[i0+3][j0+4] + B[i0+3][k0+4] * C[k0+4][j0+4];
				A[i0+4][j0+0] =  A[i0+4][j0+0] + B[i0+4][k0+0] * C[k0+0][j0+0];
				A[i0+4][j0+0] =  A[i0+4][j0+0] + B[i0+4][k0+1] * C[k0+1][j0+0];
				A[i0+4][j0+0] =  A[i0+4][j0+0] + B[i0+4][k0+2] * C[k0+2][j0+0];
				A[i0+4][j0+0] =  A[i0+4][j0+0] + B[i0+4][k0+3] * C[k0+3][j0+0];
				A[i0+4][j0+0] =  A[i0+4][j0+0] + B[i0+4][k0+4] * C[k0+4][j0+0];
				A[i0+4][j0+1] =  A[i0+4][j0+1] + B[i0+4][k0+0] * C[k0+0][j0+1];
				A[i0+4][j0+1] =  A[i0+4][j0+1] + B[i0+4][k0+1] * C[k0+1][j0+1];
				A[i0+4][j0+1] =  A[i0+4][j0+1] + B[i0+4][k0+2] * C[k0+2][j0+1];
				A[i0+4][j0+1] =  A[i0+4][j0+1] + B[i0+4][k0+3] * C[k0+3][j0+1];
				A[i0+4][j0+1] =  A[i0+4][j0+1] + B[i0+4][k0+4] * C[k0+4][j0+1];
				A[i0+4][j0+2] =  A[i0+4][j0+2] + B[i0+4][k0+0] * C[k0+0][j0+2];
				A[i0+4][j0+2] =  A[i0+4][j0+2] + B[i0+4][k0+1] * C[k0+1][j0+2];
				A[i0+4][j0+2] =  A[i0+4][j0+2] + B[i0+4][k0+2] * C[k0+2][j0+2];
				A[i0+4][j0+2] =  A[i0+4][j0+2] + B[i0+4][k0+3] * C[k0+3][j0+2];
				A[i0+4][j0+2] =  A[i0+4][j0+2] + B[i0+4][k0+4] * C[k0+4][j0+2];
				A[i0+4][j0+3] =  A[i0+4][j0+3] + B[i0+4][k0+0] * C[k0+0][j0+3];
				A[i0+4][j0+3] =  A[i0+4][j0+3] + B[i0+4][k0+1] * C[k0+1][j0+3];
				A[i0+4][j0+3] =  A[i0+4][j0+3] + B[i0+4][k0+2] * C[k0+2][j0+3];
				A[i0+4][j0+3] =  A[i0+4][j0+3] + B[i0+4][k0+3] * C[k0+3][j0+3];
				A[i0+4][j0+3] =  A[i0+4][j0+3] + B[i0+4][k0+4] * C[k0+4][j0+3];
				A[i0+4][j0+4] =  A[i0+4][j0+4] + B[i0+4][k0+0] * C[k0+0][j0+4];
				A[i0+4][j0+4] =  A[i0+4][j0+4] + B[i0+4][k0+1] * C[k0+1][j0+4];
				A[i0+4][j0+4] =  A[i0+4][j0+4] + B[i0+4][k0+2] * C[k0+2][j0+4];
				A[i0+4][j0+4] =  A[i0+4][j0+4] + B[i0+4][k0+3] * C[k0+3][j0+4];
				A[i0+4][j0+4] =  A[i0+4][j0+4] + B[i0+4][k0+4] * C[k0+4][j0+4];



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


