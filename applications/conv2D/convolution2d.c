//#include "convolution2d.h"#define
//typedef int DATA_TYPE;

/* Problem size */
#define NI 8
#define NJ 8
#define SIZE NI*NJ


int A[SIZE];
int B[SIZE];
#define c11 2
#define c12 -3
#define c21 5
#define c22 6

/*For 3x3 filter*/
#define c13 4
#define c23 7
#define c31 -8
#define c32 -9
#define c33 10


void convolution2d() {
	for (int i = 1; i < NI -1; i++) {
		for (int j = 1; j < NJ-1; j=j+17) {
#ifdef CGRA_COMPILER
please_map_me();
#endif
	/*For 3x3 filter*/
		B[i*NJ + j] = c11 * A[(i - 1)*NJ + (j - 1)]  +  c12 * A[(i + 0)*NJ + (j - 1)]  +  c13 * A[(i + 1)*NJ + (j - 1)]
				+ c21 * A[(i - 1)*NJ + (j + 0)]  +  c22 * A[(i + 0)*NJ + (j + 0)]  +  c23 * A[(i + 1)*NJ + (j + 0)]
				+ c31 * A[(i - 1)*NJ + (j + 1)]  +  c32 * A[(i + 0)*NJ + (j + 1)]  +  c33 * A[(i + 1)*NJ + (j + 1)];

		B[i*NJ + j+1] = c11 * A[(i - 1)*NJ + (j+1 - 1)]  +  c12 * A[(i + 0)*NJ + (j+1 - 1)]  +  c13 * A[(i + 1)*NJ + (j+1 - 1)]
				+ c21 * A[(i - 1)*NJ + (j+1 + 0)]  +  c22 * A[(i + 0)*NJ + (j+1 + 0)]  +  c23 * A[(i + 1)*NJ + (j+1 + 0)]
				+ c31 * A[(i - 1)*NJ + (j+1 + 1)]  +  c32 * A[(i + 0)*NJ + (j+1 + 1)]  +  c33 * A[(i + 1)*NJ + (j+1 + 1)];

		B[i*NJ + (j+2)] = c11 * A[(i - 1)*NJ + ((j+2) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+2) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+2) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+2) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+2) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+2) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+2) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+2) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+2) + 1)];

		B[i*NJ + (j+3)] = c11 * A[(i - 1)*NJ + ((j+3) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+3) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+3) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+3) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+3) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+3) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+3) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+3) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+3) + 1)];

		B[i*NJ + (j+4)] = c11 * A[(i - 1)*NJ + ((j+4) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+4) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+4) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+4) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+4) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+4) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+4) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+4) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+4) + 1)];

		B[i*NJ + (j+5)] = c11 * A[(i - 1)*NJ + ((j+5) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+5) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+5) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+5) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+5) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+5) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+5) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+5) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+5) + 1)];

		B[i*NJ + (j+6)] = c11 * A[(i - 1)*NJ + ((j+6) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+6) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+6) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+6) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+6) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+6) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+6) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+6) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+6) + 1)];

		B[i*NJ + (j+7)] = c11 * A[(i - 1)*NJ + ((j+7) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+7) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+7) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+7) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+7) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+7) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+7) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+7) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+7) + 1)];

		B[i*NJ + (j+8)] = c11 * A[(i - 1)*NJ + ((j+8) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+8) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+8) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+8) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+8) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+8) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+8) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+8) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+8) + 1)];

		B[i*NJ + (j+9)] = c11 * A[(i - 1)*NJ + ((j+9) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+9) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+9) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+9) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+9) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+9) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+9) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+9) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+9) + 1)];

		B[i*NJ + (j+10)] = c11 * A[(i - 1)*NJ + ((j+10) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+10) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+10) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+10) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+10) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+10) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+10) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+10) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+10) + 1)];

		B[i*NJ + (j+11)] = c11 * A[(i - 1)*NJ + ((j+11) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+11) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+11) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+11) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+11) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+11) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+11) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+11) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+11) + 1)];

		B[i*NJ + (j+12)] = c11 * A[(i - 1)*NJ + ((j+12) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+12) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+12) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+12) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+12) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+12) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+12) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+12) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+12) + 1)];

		B[i*NJ + (j+13)] = c11 * A[(i - 1)*NJ + ((j+13) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+13) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+13) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+13) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+13) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+13) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+13) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+13) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+13) + 1)];

		B[i*NJ + (j+14)] = c11 * A[(i - 1)*NJ + ((j+14) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+14) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+14) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+14) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+14) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+14) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+14) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+14) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+14) + 1)];

		B[i*NJ + (j+15)] = c11 * A[(i - 1)*NJ + ((j+15) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+15) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+15) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+15) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+15) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+15) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+15) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+15) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+15) + 1)];

		B[i*NJ + (j+16)] = c11 * A[(i - 1)*NJ + ((j+16) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+16) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+16) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+16) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+16) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+16) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+16) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+16) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+16) + 1)];

	// /*For 2x2 filter*/
	// 	B[i*NJ + j] = c11 * A[(i - 1)*NJ + (j - 1)]  +  c12 * A[(i + 0)*NJ + (j - 1)] 
	// 			+ c21 * A[(i - 1)*NJ + (j + 0)]  +  c22 * A[(i + 0)*NJ + (j + 0)];
		}
	}
}


void convolution2d_less_unroll() {
	for (int i = 1; i < NI -1; i++) {
		for (int j = 1; j < NJ-1; j=j+9) {
#ifdef CGRA_COMPILER
please_map_me();
#endif
	/*For 3x3 filter*/
		B[i*NJ + j] = c11 * A[(i - 1)*NJ + (j - 1)]  +  c12 * A[(i + 0)*NJ + (j - 1)]  +  c13 * A[(i + 1)*NJ + (j - 1)]
				+ c21 * A[(i - 1)*NJ + (j + 0)]  +  c22 * A[(i + 0)*NJ + (j + 0)]  +  c23 * A[(i + 1)*NJ + (j + 0)]
				+ c31 * A[(i - 1)*NJ + (j + 1)]  +  c32 * A[(i + 0)*NJ + (j + 1)]  +  c33 * A[(i + 1)*NJ + (j + 1)];

		B[i*NJ + j+1] = c11 * A[(i - 1)*NJ + (j+1 - 1)]  +  c12 * A[(i + 0)*NJ + (j+1 - 1)]  +  c13 * A[(i + 1)*NJ + (j+1 - 1)]
				+ c21 * A[(i - 1)*NJ + (j+1 + 0)]  +  c22 * A[(i + 0)*NJ + (j+1 + 0)]  +  c23 * A[(i + 1)*NJ + (j+1 + 0)]
				+ c31 * A[(i - 1)*NJ + (j+1 + 1)]  +  c32 * A[(i + 0)*NJ + (j+1 + 1)]  +  c33 * A[(i + 1)*NJ + (j+1 + 1)];

		B[i*NJ + (j+2)] = c11 * A[(i - 1)*NJ + ((j+2) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+2) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+2) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+2) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+2) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+2) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+2) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+2) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+2) + 1)];

		B[i*NJ + (j+3)] = c11 * A[(i - 1)*NJ + ((j+3) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+3) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+3) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+3) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+3) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+3) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+3) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+3) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+3) + 1)];

		B[i*NJ + (j+4)] = c11 * A[(i - 1)*NJ + ((j+4) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+4) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+4) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+4) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+4) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+4) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+4) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+4) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+4) + 1)];

		B[i*NJ + (j+5)] = c11 * A[(i - 1)*NJ + ((j+5) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+5) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+5) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+5) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+5) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+5) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+5) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+5) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+5) + 1)];

		B[i*NJ + (j+6)] = c11 * A[(i - 1)*NJ + ((j+6) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+6) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+6) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+6) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+6) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+6) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+6) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+6) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+6) + 1)];

		B[i*NJ + (j+7)] = c11 * A[(i - 1)*NJ + ((j+7) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+7) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+7) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+7) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+7) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+7) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+7) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+7) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+7) + 1)];

		B[i*NJ + (j+8)] = c11 * A[(i - 1)*NJ + ((j+8) - 1)]  +  c12 * A[(i + 0)*NJ + ((j+8) - 1)]  +  c13 * A[(i + 1)*NJ + ((j+8) - 1)]
				+ c21 * A[(i - 1)*NJ + ((j+8) + 0)]  +  c22 * A[(i + 0)*NJ + ((j+8) + 0)]  +  c23 * A[(i + 1)*NJ + ((j+8) + 0)]
				+ c31 * A[(i - 1)*NJ + ((j+8) + 1)]  +  c32 * A[(i + 0)*NJ + ((j+8) + 1)]  +  c33 * A[(i + 1)*NJ + ((j+8) + 1)];

	// /*For 2x2 filter*/
	// 	B[i*NJ + j] = c11 * A[(i - 1)*NJ + (j - 1)]  +  c12 * A[(i + 0)*NJ + (j - 1)] 
	// 			+ c21 * A[(i - 1)*NJ + (j + 0)]  +  c22 * A[(i + 0)*NJ + (j + 0)];
		}
	}
}


int main(){


for(int i=0;i<SIZE;i++){
    A[i] = i+1;
  }

convolution2d();
return 0;
}
