#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <stdint.h>
#include <dirent.h>

#define C1 80
#define R1 8
#define C2 504
#define R2 C1

#define P 12//P

#define R3 4000
#define C3 4
//#define R1C1C24P R1*C1*C2/(4*P)
//#define C2P C2/P
#define R1C1C24P 6720
#define C2P 42
int8_t WEIGHT_MATRIX[R1*C1];
int8_t INPUT_MATRIX[R2*C2];
int16_t OUTPUT_MATRIX_[R1*C2];
int32_t OUTPUT_MATRIX[R1*500];
int16_t OUTPUT_MATRIX_EXP[R1*C2];

int DENSE_MATRIX[R3*C3];
int64_t FINAL_OUT[4];
int TEST_DATA[49*40];
int16_t WEIGHT_TEST_F[640];
int64_t OUTPUT64[4000];

//param1 conv1 size 640  dim(8,80) elements 
const int param1[640];
//param2 bias1 size 8 elements
const int param2[8] = {-375,170,-48,208,83,6,-1205,-696};
//param3 fc_layer size 4*4000 elements
int param3[16000];
//param4 bias2 size 4 elements
const int param4[] = {433,-526,-96,189};

void read_test_data_param(char filename[]) {

    int num;
    FILE *fptr;
    int TEST_MATRIX[40*49];

    if ((fptr = fopen(filename,"r")) == NULL){
       printf("Error! opening file");

       // Program exits if the file pointer returns NULL.
       exit(1);
    }
    int i;
    for (i = 0; i < 49; i++) {
        for (int j=0; j<40; j++) {
            fscanf(fptr, "%d,", &TEST_DATA[i*40 + j]);
        }
    }
    fclose(fptr); 
}


void read_conv_param() {

   int num;
   FILE *fptr;
   int FILTERW[640];
   if ((fptr = fopen("param_conv.txt","r")) == NULL){
       printf("Error! opening file");

       // Program exits if the file pointer returns NULL.
       exit(1);
   }

    int i;
    for (i = 0; i < 80; i++) {
        for (int j=0; j<8; j++) { 
            fscanf(fptr, "%d,", &FILTERW[i*8 + j]);
        }
    }

   fclose(fptr); 

    int cnt = 0;
    
    for (int k=0; k<8; k++) {
        //int cnt = 0;
        for(int l=0; l<80; l++) {
            WEIGHT_MATRIX[cnt] = FILTERW[(l)*8 +(k)];
            cnt++;
        }
    }

   int max = 31;
   int min = -32;
   for (i=0; i<640; i++) {
       if (WEIGHT_MATRIX[i] > max) {
           WEIGHT_MATRIX[i] = max;
       }
       else if (WEIGHT_MATRIX[i] < min) {
           WEIGHT_MATRIX[i] = min;
       }
       
   }
}

void read_dense_param() {

   int num;
   FILE *fptr;
   if ((fptr = fopen("param_fc.txt","r")) == NULL){
       printf("Error! opening file");

       // Program exits if the file pointer returns NULL.
       exit(1);
   }
   int i;
    for (i = 0; i < 4000; i++) {
        for (int j=0; j<4; j++) 
            fscanf(fptr, "%d,", &DENSE_MATRIX[i*4 + j]);
    }
   fclose(fptr); 
}

void init_input() {

    int TEMP_MATRIX[49*46];
    int PADDED_MATRIX[58*46];
    int INPUT_MATRIX_COPY[80*500];

    //padding zeros along column
    for (int i=0; i<49; i++) {
        for (int j=0; j<3; j++) {
            TEMP_MATRIX[i*46 + j] = -128;
        }

        for (int j=3; j<43; j++) {
            TEMP_MATRIX[i*46 + j] = TEST_DATA[i*40 + j - 3];
        }

        for (int j=43; j<46; j++) {
            TEMP_MATRIX[i*46 + j] = -128;
        }
    }

    //padding zeros along row dim
    for (int i=0; i<4; i++) {
        for (int j=0; j<46; j++) {
            PADDED_MATRIX[i*46 + j] = -128;
        }
    }
    for (int i=4; i<53; i++) {
        for (int j=0; j<46; j++) {
            PADDED_MATRIX[i*46 + j] = TEMP_MATRIX[(i-4)*46 + j];
        }
    }
    for (int i=53; i<58; i++) {
        for (int j=0; j<46; j++) {
            PADDED_MATRIX[i*46 + j] = -128;
        }
    }

    //sanity check
    for (int i=4; i<53; i++) {
        for (int j=3; j<43; j++) {
            if (PADDED_MATRIX[i*46 + j] != TEST_DATA[(i-4)*40 + j - 3]) {
                printf("INCORRECT");
            }         
        }
    }
    //warping the original matrix into a 80*500 dim matrix
    int count = 0;
    for (int i = 0; i<=48; i+=2) {
        for (int j=0; j<=38; j+=2) {
            //int c = 0;
            for (int row = i; row<i+10; row++) {
                for (int col=j; col<j+8; col++) {
                    INPUT_MATRIX_COPY[count++] = PADDED_MATRIX[row*46 + col];
                }
            }
        }
    }


   
   //int INPUT_MATRIX_COPY[80*500];
   int i, j, k;
   int c1 = 0;
   for(i=0; i<504; i++) {
    for (int j=0; j<80; j++) {
        if (i < 500) {
            INPUT_MATRIX[i+j*504] = INPUT_MATRIX_COPY[c1++];    
        }
        else {
            INPUT_MATRIX[i+j*504] = 0;   
        }
    }
   }  
   int max = 31;
   int min = -32;
   for (i=0; i<40320; i++) {
       if (INPUT_MATRIX[i] > max) {
           INPUT_MATRIX[i] = max;
       }
       else if (INPUT_MATRIX[i] < min) {
           INPUT_MATRIX[i] = min;
       }
       
   }
}


void microspeech_conv_original() {

    int TEMP_MATRIX[49*46];
    int PADDED_MATRIX[58*46];

    int OUTPUT_TEST[500*8];

    //padding zeros along column
    for (int i=0; i<49; i++) {
        for (int j=0; j<3; j++) {
            TEMP_MATRIX[i*46 + j] = -128;
        }

        for (int j=3; j<43; j++) {
            TEMP_MATRIX[i*46 + j] = TEST_DATA[i*40 + j - 3];
        }

        for (int j=43; j<46; j++) {
            TEMP_MATRIX[i*46 + j] = -128;
        }
    }

    //padding zeros along row dim
    for (int i=0; i<4; i++) {
        for (int j=0; j<46; j++) {
            PADDED_MATRIX[i*46 + j] = -128;
        }
    }
    for (int i=4; i<53; i++) {
        for (int j=0; j<46; j++) {
            PADDED_MATRIX[i*46 + j] = TEMP_MATRIX[(i-4)*46 + j];
        }
    }
    for (int i=53; i<58; i++) {
        for (int j=0; j<46; j++) {
            PADDED_MATRIX[i*46 + j] = -128;
        }
    }

    // sanity check
    for (int i=4; i<53; i++) {
        for (int j=3; j<43; j++) {
            if (PADDED_MATRIX[i*46 + j] != TEST_DATA[(i-4)*40 + j - 3]) {
                printf("INCORRECT");
            }         
        }
    }
    
    int c = 0;
    //warping the original matrix into a 80*500 dim matrix
    int count = 0;
    for (int k=0; k<8; k++) {
        int cnt = 0;
        int FILTER[80];
        for(int l=0; l<80; l++) {
            FILTER[cnt++] = WEIGHT_TEST_F[(l)*8 +(k)];
        }
        for (int i = 0; i<=48; i+=2) {
            for (int j=0; j<=38; j+=2) {                    
                int temp = 0;
                //c++;
                for (int row = i; row<i+10; row++) {
                    for (int col=j; col<j+8; col++) {
                        temp += PADDED_MATRIX[row*46 + col]*FILTER[(row-i)*8 + (col-j)];
                        //printf("%d %d\n", row-i, col-j);
                    }
                }
                OUTPUT_MATRIX_EXP[c++] = temp;
            }    
        }
    }
}

void microspeech_conv_layer(){
    
    int i, j, k;
    int max = 32767;
    int min = -32767;

    for (i=0;i<R1; i++) {
        for (j=0;j<C2; j++) {
            int16_t temp=0;
            for (k=0;k<C1; k++){
                temp += WEIGHT_MATRIX[i*C1+k]*INPUT_MATRIX[k*C2+j];
            }
            OUTPUT_MATRIX_EXP[i*C2+j] = temp;
 	    //OUTPUT_MATRIX_[i*C2+j] = temp;
        }
    }
}

void microspeech_bias_ReLu() {
    for (int i=0; i<4000; i++) {
        int indx = i/500;
            OUTPUT_MATRIX[i] += param2[indx];
    }
}

void quantize_conv_layer() {
    int i, j, k;
    int sum_weight[8];
    for (int q=0; q<8; q++) {
        sum_weight[q] = 0;
    }
    for(i=0; i<R1; i++) {
        for (j=0; j<C1; j++) {
            sum_weight[i] += WEIGHT_MATRIX[i*C1 + j];
        }
    }

    for (i=0; i<4000; i++) {
        int ind = (int) i/500;
        OUTPUT_MATRIX[i] = OUTPUT_MATRIX[i] + (128*sum_weight[ind]);
    }
}

void quantize_fc_layer() {
    int i, j, k;    
    int sum_weight[8];
    for (int q=0; q<8; q++) {
        sum_weight[q] = 0;
    }
    for(i=0; i<4000; i++) {
        for (j=0; j<4; j++) {
            sum_weight[j] += DENSE_MATRIX[i*4 + j];
        }
    }

    for (i=0; i<4; i++) {
        FINAL_OUT[i] = FINAL_OUT[i] + (128*sum_weight[i]);
    }
}

void requantize_conv() {

    int64_t factor[8] = {1674350931,1535919972,2026360758,1174747100,1517546873,1302070198,1086796819,1779030479};
    int64_t factor2[8] = {1099511627776,4398046511104,1099511627776,1099511627776,1099511627776,1099511627776,549755813888,1099511627776};
    int64_t factor3[8] = {41,43,41,41,41,41,40,41};

    for (int i=0; i<4000; i++) {
        int ind = i/500;
        OUTPUT64[i] = OUTPUT_MATRIX[i]*factor[ind] + factor2[ind];
    }

    for (int i=0; i<4000; i++) {
        int ind = i/500;
        OUTPUT64[i] = OUTPUT64[i]>>factor3[ind];
        OUTPUT64[i] +=  -128;
        if (OUTPUT64[i] < -128) {
            OUTPUT_MATRIX[i] = -128;
        }
        else if (OUTPUT64[i] > 127) {
            OUTPUT_MATRIX[i] = 127;
        }
        else {
            OUTPUT_MATRIX[i] = OUTPUT64[i];
        }
    }
 
    int OUTPUT_TEST_F[4000];
    int count = 0;
    for (int j=0; j<500; j++) {
    for (int i=0; i<8; i++) {
        OUTPUT_TEST_F[count++] = OUTPUT_MATRIX[i*500 + j];
        }
    }   
}

int requantize_fc() {

    int FINAL[4];
    for(int i=0; i<4; i++) {
        FINAL[i] = FINAL_OUT[i] + param4[i];
    }

    long long int multiplier=1780495384;
    long long int shift=-11;
    int total_right_shift = shift + 31;
    long long int FINAL64[4];
    int pos_rounding_val = 1 << (total_right_shift);

    // fixed_point_multiply(x, fixed_point_multiplier, right_shift) 
    // x = cast(x,int64) * fixed_point_multiplier 
    // total_right_shift = right_shift + 31 
    // pos_rounding_value = 1 << (total_right_shift -1) 
    // x = x + pos_rounding_value 
    // x = x >> total_right_shift return cast(x, int32)

    int FINAL8[4];
    int FIN[4];
    for (int i=0; i<4; i++) {
        FINAL8[i] = (FINAL[i] * multiplier + pos_rounding_val) >> (total_right_shift + 1);
        FINAL8[i] += pos_rounding_val;       
        FINAL8[i] = FINAL8[i] >> (total_right_shift + 1);
    }

    for (int i=0; i<4; i++) {
        FINAL_OUT[i] = FINAL8[i] + 19;
        if (FINAL_OUT[i] < -128) {
            FINAL_OUT[i] = -128;
        }
        else if (FINAL_OUT[i] > 127) {
            FINAL_OUT[i] = 127;
        }
        else {
            FINAL_OUT[i] = FINAL_OUT[i];
        }
    }

    for (int i=0; i<4; i++) {
        FINAL_OUT[i] -= 19;
    }

    float FINAL_FLOAT[4];
    float max_val = -13456;
    int max_idx = -1;

    for (int i=0; i<4; i++) {
        FINAL_FLOAT[i] = FINAL_OUT[i]*0.0979961; 
        if (max_val < FINAL_FLOAT[i]) {
            max_val = FINAL_FLOAT[i];
            max_idx = i;
        }
    }
    // printf("Obtained label:%d\n", max_idx); 
    return max_idx;   
}


void reshape_conv_output() {

   int OUTPUT_MATRIX_COPY[4000];
   int i;
   for(i=0; i<4000; i++) {
    OUTPUT_MATRIX_COPY[i] = OUTPUT_MATRIX[i];
   }
   int c1 = 0;
   for(i=0; i<500; i++) {
    for (int j=0; j<8; j++) {
        OUTPUT_MATRIX[c1++] = OUTPUT_MATRIX_COPY[i + j*500];
    }
   }  
}


void microspeech_fc_layer(){
    int i,j,k;

    for (i=0; i<4; i++) {
        FINAL_OUT[i] = 0;
    }

    for (i=0;i<1; i++) {
        for (j=0;j<C3; j++) {
            for (k=0;k<R3; k++){
                FINAL_OUT[i*C3+j] += OUTPUT_MATRIX[i*R3+k]*DENSE_MATRIX[k*C3+j];
            }
        }
    }
}




int32_t O0[R1*(C2/P)], W0[R1*C1], I0[R2*(C2/P)];
int32_t O1[R1*(C2/P)], W1[R1*C1], I1[R2*(C2/P)];
int32_t O2[R1*(C2/P)], W2[R1*C1], I2[R2*(C2/P)];
int32_t O3[R1*(C2/P)], W3[R1*C1], I3[R2*(C2/P)];
//int32_t R1C1C24P=R1*C1*C2/(4*P);
//int32_t C2P=C2/P;
//12 partitions
void microspeech_conv_layer_hycube(){
	int i,j,k,ijk;

  //printf("invoke\n");

	//First four blocks
	//initialize O
	for(int h=0; h<R1; h++)
		for(int w=0; w<C2/P; w++){
			O0[h*(C2/P)+w] = 0;
			O1[h*(C2/P)+w] = 0;
			O2[h*(C2/P)+w] = 0;
			O3[h*(C2/P)+w] = 0;
		}
	//copy weight_m to W
	for(int h=0; h<R1; h++)
		for(int w=0; w<C1; w++){
			W0[h*C1+w] = WEIGHT_MATRIX[h*C1+w];
			W1[h*C1+w] = WEIGHT_MATRIX[h*C1+w];
			W2[h*C1+w] = WEIGHT_MATRIX[h*C1+w];
			W3[h*C1+w] = WEIGHT_MATRIX[h*C1+w];
		}

	//copy input_m to I
	for(int h=0; h<R2; h++)
		for(int w=0; w<C2/P; w++){
			I0[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w];
			I1[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (C2/P)];
			I2[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (2*C2/P)];
			I3[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (3*C2/P)];
		}


	// execute
    /*
    //GEMM kernel
	for (i=0;i<R1; i++)
		for (j=0;j<C2/P; j++)
			for (k=0;k<C1; k++){
				O0[i*C2/P+j] += W0[i*C1+k]* I0[k*C2/P+j];
			}
    */

    // Flattened and Unrolled GEMM kernel
    i=0;j=0;k=0;
    for (ijk=0;ijk<R1C1C24P; ijk++){
        #ifdef CGRA_COMPILER
        please_map_me();
        #endif
	    O0[i*C2P+j] = O0[i*C2P+j] + W0[i*C1+k]* I0[k*C2P+j]+ W0[i*C1+k+1]* I0[(k+1)*C2P+j]+ W0[i*C1+k+2]* I0[(k+2)*C2P+j]+W0[i*C1+k+3]* I0[(k+3)*C2P+j];
	    k=k+4;
	    if(k+1 >= C1){
		  k=0;
		  ++j;
	    }
	    if(j == C2P){
  		  j=0;
	      ++i;
	    }
	}


    i=0;j=0;k=0;
    for (ijk=0;ijk<R1*C1*C2/(4*P); ijk++){
       //#ifdef CGRA_COMPILER
       //please_map_me();
       //#endif
	    O1[i*C2/P+j] = O1[i*C2/P+j] + W1[i*C1+k]* I1[k*C2/P+j]+ W1[i*C1+k+1]* I1[(k+1)*C2/P+j]+ W1[i*C1+k+2]* I1[(k+2)*C2/P+j]+W1[i*C1+k+3]* I1[(k+3)*C2/P+j];
	    k=k+4;
        if(k+1 >= C1){
          k=0;
          ++j;
        }
        if(j == C2/P){
          j=0;
          ++i;
        }
	}

    i=0;j=0;k=0;
    for (ijk=0;ijk<R1*C1*C2/(4*P); ijk++){
       //#ifdef CGRA_COMPILER
       //please_map_me();
       //#endif
	    O2[i*C2/P+j] = O2[i*C2/P+j] + W2[i*C1+k]* I2[k*C2/P+j]+ W2[i*C1+k+1]* I2[(k+1)*C2/P+j]+ W2[i*C1+k+2]* I2[(k+2)*C2/P+j]+W2[i*C1+k+3]* I2[(k+3)*C2/P+j];
	    k=k+4;
        if(k+1 >= C1){
          k=0;
          ++j;
        }
        if(j == C2/P){
          j=0;
          ++i;
        }
	}

    i=0;j=0;k=0;
    for (ijk=0;ijk<R1*C1*C2/(4*P); ijk++){
       //#ifdef CGRA_COMPILER
       //please_map_me();
       //#endif
	    O3[i*C2/P+j] = O3[i*C2/P+j] + W3[i*C1+k]* I3[k*C2/P+j]+ W3[i*C1+k+1]* I3[(k+1)*C2/P+j]+ W3[i*C1+k+2]* I3[(k+2)*C2/P+j]+W3[i*C1+k+3]* I3[(k+3)*C2/P+j];
	    k=k+4;
        if(k+1 >= C1){
          k=0;
          ++j;
        }
        if(j == C2/P){
          j=0;
          ++i;
        }
	}






	//copy data back from O
	for(int h=0; h<R1; h++)
		for(int w=0; w<C2/P; w++){
			OUTPUT_MATRIX_[h*(C2)+w] = O0[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (C2/P)] = O1[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (2*C2/P)] = O2[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (3*C2/P)] = O3[h*(C2/P)+w];
		}


	//printf("First four blocks done\n");
	// 2nd four blocks
	//First four blocks
	//initialize O
	for(int h=0; h<R1; h++)
		for(int w=0; w<C2/P; w++){
			O0[h*(C2/P)+w] = 0;
			O1[h*(C2/P)+w] = 0;
			O2[h*(C2/P)+w] = 0;
			O3[h*(C2/P)+w] = 0;
		}
	//copy weight_m to W
	for(int h=0; h<R1; h++)
		for(int w=0; w<C1; w++){
			W0[h*C1+w] = WEIGHT_MATRIX[h*C1+w];
			W1[h*C1+w] = WEIGHT_MATRIX[h*C1+w];
			W2[h*C1+w] = WEIGHT_MATRIX[h*C1+w];
			W3[h*C1+w] = WEIGHT_MATRIX[h*C1+w];
		}

	//copy input_m to I
	for(int h=0; h<R2; h++)
		for(int w=0; w<C2/P; w++){
			I0[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (4*C2/P)];
			I1[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (5*C2/P)];
			I2[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (6*C2/P)];
			I3[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (7*C2/P)];
		}

	// execute

    i=0;j=0;k=0;
    for (ijk=0;ijk<R1*C1*C2/(4*P); ijk++){
       //#ifdef CGRA_COMPILER
       //please_map_me();
       //#endif
	   O0[i*C2/P+j] = O0[i*C2/P+j] + W0[i*C1+k]* I0[k*C2/P+j]+ W0[i*C1+k+1]* I0[(k+1)*C2/P+j]+ W0[i*C1+k+2]* I0[(k+2)*C2/P+j]+W0[i*C1+k+3]* I0[(k+3)*C2/P+j];
	   k=k+4;
	   if(k+1 >= C1){
	   	k=0;
	   	++j;
	   }
	   if(j == C2/P){
  	   	j=0;
	   	++i;
	   }
	}

    i=0;j=0;k=0;
    for (ijk=0;ijk<R1*C1*C2/(4*P); ijk++){
       //#ifdef CGRA_COMPILER
       //please_map_me();
       //#endif
	   O1[i*C2/P+j] = O1[i*C2/P+j] + W1[i*C1+k]* I1[k*C2/P+j]+ W1[i*C1+k+1]* I1[(k+1)*C2/P+j]+ W1[i*C1+k+2]* I1[(k+2)*C2/P+j]+W1[i*C1+k+3]* I1[(k+3)*C2/P+j];
	   k=k+4;
	   if(k+1 >= C1){
	   	k=0;
	   	++j;
	   }
	   if(j == C2/P){
  	   	j=0;
	   	++i;
	   }
	}

    i=0;j=0;k=0;
    for (ijk=0;ijk<R1*C1*C2/(4*P); ijk++){
       //#ifdef CGRA_COMPILER
       //please_map_me();
       //#endif
	   O2[i*C2/P+j] = O2[i*C2/P+j] + W2[i*C1+k]* I2[k*C2/P+j]+ W2[i*C1+k+1]* I2[(k+1)*C2/P+j]+ W2[i*C1+k+2]* I2[(k+2)*C2/P+j]+W2[i*C1+k+3]* I2[(k+3)*C2/P+j];
	   k=k+4;
	   if(k+1 >= C1){
	   	k=0;
	   	++j;
	   }
	   if(j == C2/P){
  	   	j=0;
	   	++i;
	   }
	}

    i=0;j=0;k=0;
    for (ijk=0;ijk<R1*C1*C2/(4*P); ijk++){
       //#ifdef CGRA_COMPILER
       //please_map_me();
       //#endif
	   O3[i*C2/P+j] = O3[i*C2/P+j] + W3[i*C1+k]* I3[k*C2/P+j]+ W3[i*C1+k+1]* I3[(k+1)*C2/P+j]+ W3[i*C1+k+2]* I3[(k+2)*C2/P+j]+W3[i*C1+k+3]* I3[(k+3)*C2/P+j];
	   k=k+4;
	   if(k+1 >= C1){
	   	k=0;
	   	++j;
	   }
	   if(j == C2/P){
  	   	j=0;
	   	++i;
	   }
	}




	//copy data back from O
	for(int h=0; h<R1; h++)
		for(int w=0; w<C2/P; w++){
			OUTPUT_MATRIX_[h*(C2)+w+ (4*C2/P)] = O0[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (5*C2/P)] = O1[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (6*C2/P)] = O2[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (7*C2/P)] = O3[h*(C2/P)+w];
		}


	//printf("Second four blocks done\n");

    // 3rd four blocks
    //First four blocks
	//initialize O
	for(int h=0; h<R1; h++)
		for(int w=0; w<C2/P; w++){
			O0[h*(C2/P)+w] = 0;
			O1[h*(C2/P)+w] = 0;
			O2[h*(C2/P)+w] = 0;
			O3[h*(C2/P)+w] = 0;
		}
	//copy weight_m to W
	for(int h=0; h<R1; h++)
		for(int w=0; w<C1; w++){
			W0[h*C1+w] = WEIGHT_MATRIX[h*C1+w];
			W1[h*C1+w] = WEIGHT_MATRIX[h*C1+w];
			W2[h*C1+w] = WEIGHT_MATRIX[h*C1+w];
			W3[h*C1+w] = WEIGHT_MATRIX[h*C1+w];
		}

	//copy input_m to I
	for(int h=0; h<R2; h++)
		for(int w=0; w<C2/P; w++){
			I0[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (8*C2/P)];
			I1[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (9*C2/P)];
			I2[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (10*C2/P)];
			I3[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (11*C2/P)];
		}

	// execute
    i=0;j=0;k=0;
    for (ijk=0;ijk<R1*C1*C2/(4*P); ijk++){
       //#ifdef CGRA_COMPILER
       //please_map_me();
       //#endif
	   O0[i*C2/P+j] = O0[i*C2/P+j] + W0[i*C1+k]* I0[k*C2/P+j]+ W0[i*C1+k+1]* I0[(k+1)*C2/P+j]+ W0[i*C1+k+2]* I0[(k+2)*C2/P+j]+W0[i*C1+k+3]* I0[(k+3)*C2/P+j];
	   k=k+4;
	   if(k+1 >= C1){
	   	k=0;
	   	++j;
	   }
	   if(j == C2/P){
  	   	j=0;
	   	++i;
	   }
	}

    i=0;j=0;k=0;
    for (ijk=0;ijk<R1*C1*C2/(4*P); ijk++){
       //#ifdef CGRA_COMPILER
       //please_map_me();
       //#endif
	   O1[i*C2/P+j] = O1[i*C2/P+j] + W1[i*C1+k]* I1[k*C2/P+j]+ W1[i*C1+k+1]* I1[(k+1)*C2/P+j]+ W1[i*C1+k+2]* I1[(k+2)*C2/P+j]+W1[i*C1+k+3]* I1[(k+3)*C2/P+j];
	   k=k+4;
	   if(k+1 >= C1){
	   	k=0;
	   	++j;
	   }
	   if(j == C2/P){
  	   	j=0;
	   	++i;
	   }
	}


    i=0;j=0;k=0;
    for (ijk=0;ijk<R1*C1*C2/(4*P); ijk++){
       //#ifdef CGRA_COMPILER
       //please_map_me();
       //#endif
	   O2[i*C2/P+j] = O2[i*C2/P+j] + W2[i*C1+k]* I2[k*C2/P+j]+ W2[i*C1+k+1]* I2[(k+1)*C2/P+j]+ W2[i*C1+k+2]* I2[(k+2)*C2/P+j]+W2[i*C1+k+3]* I2[(k+3)*C2/P+j];
	   k=k+4;
	   if(k+1 >= C1){
	   	k=0;
	   	++j;
	   }
	   if(j == C2/P){
  	   	j=0;
	   	++i;
	   }
	}

    i=0;j=0;k=0;
    for (ijk=0;ijk<R1*C1*C2/(4*P); ijk++){
       //#ifdef CGRA_COMPILER
       //please_map_me();
       //#endif
	   O3[i*C2/P+j] = O3[i*C2/P+j] + W3[i*C1+k]* I3[k*C2/P+j]+ W3[i*C1+k+1]* I3[(k+1)*C2/P+j]+ W3[i*C1+k+2]* I3[(k+2)*C2/P+j]+W3[i*C1+k+3]* I3[(k+3)*C2/P+j];
	   k=k+4;
	   if(k+1 >= C1){
	   	k=0;
	   	++j;
	   }
	   if(j == C2/P){
  	   	j=0;
	   	++i;
	   }
	}




	//copy data back from O
	for(int h=0; h<R1; h++)
		for(int w=0; w<C2/P; w++){
			OUTPUT_MATRIX_[h*(C2)+w+ (8*C2/P)] = O0[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (9*C2/P)] = O1[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (10*C2/P)] = O2[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (11*C2/P)] = O3[h*(C2/P)+w];
		}


	//printf("Third four blocks done\n");


	//printf("Total number of invocations: %d\n",id-1);
	//return;
	//pthread_exit(NULL);
}



void main() {

 int correct = 0;
 int incorrect = 0;
  float accuracy = 0;
  DIR *dir;
  struct dirent *entry;

  //specify directory path here
  //path_to_dir
  dir = opendir("/home/angela/Desktop/prototype/Morpher_DFG_Generator/applications/hycube_v3_design_app_test/Microspeech16/test_data/");
  
  if (dir == NULL) {
    printf("Failed to open directory.\n");
    return;
  }
  int c = 0;
  while ((entry = readdir(dir)) != NULL) {
    if(entry->d_type != DT_DIR) {
      char string[1000];
      
      //specify directory path here
      char full_file_name[] = "/home/angela/Desktop/prototype/Morpher_DFG_Generator/applications/hycube_v3_design_app_test/Microspeech16/test_data/";
      int ctr_fl = sizeof(full_file_name);

      //printf("%s\n", entry->d_name);
      int ctr = 0;
      while(entry->d_name[ctr] != '\0') {
          string[ctr] = entry->d_name[ctr];
          full_file_name[ctr_fl-1] = entry->d_name[ctr];
          ctr++;
          ctr_fl++;
      }
      string[ctr] = '\0';
      full_file_name[ctr_fl-1] = '\0';
      int correct_label = string[ctr-6] - '0';
    //   printf("Correct label: %d\n", correct_label);
      c++;

    //read test data 
    //full_file_name
    read_test_data_param(full_file_name);
    //initialize input
    init_input();
    //read conv1 param
    read_conv_param();    

    // //perform convolution
    //microspeech_conv_original();
    microspeech_conv_layer();

    microspeech_conv_layer_hycube();

    for (int i=0;i<R1; i++)
        for (int j=0; j<C2; j++) {
            if (OUTPUT_MATRIX_[(i)*C2+(j)]!=OUTPUT_MATRIX_EXP[(i)*C2+(j)])
            {
                //printf("i:%d j:%d INCORRECT %d,%d\n",i,j,OUTPUT_MATRIX_EXP[(i)*C2+(j)],OUTPUT_MATRIX_[(i)*C2+(j)]);
               incorrect++;
	    }else{
              //  printf("i:%d j:%d CORRECT %d,%d\n",i,j,OUTPUT_MATRIX_EXP[(i)*C2+(j)],OUTPUT_MATRIX_[(i)*C2+(j)]);
              correct++;
	    }
        }

    for (int i=0;i<R1; i++)
	for (int j=0; j<500; j++){
		OUTPUT_MATRIX[(i)*500+(j)] =OUTPUT_MATRIX_EXP[(i)*C2+(j)];// OUTPUT_MATRIX_[(i)*C2+(j)];
	}
    quantize_conv_layer();

    microspeech_bias_ReLu();
    requantize_conv();
    reshape_conv_output();
    read_dense_param();
    microspeech_fc_layer();
    quantize_fc_layer();
    int obtained_label = requantize_fc();
    if (obtained_label == correct_label) {
        accuracy += 1;
        }
    }
  }
  printf("Final model accuracy: %f\n", (accuracy/c)*100);
  printf("Correct: %d , Incorrect: %d\n", correct,incorrect);

  closedir(dir);
}

