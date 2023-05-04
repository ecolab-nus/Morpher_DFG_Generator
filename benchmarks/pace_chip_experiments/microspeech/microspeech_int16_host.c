#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
//#include "ftd2xx.h"
//#include "libft4222.h"
#include <unistd.h>

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

#define INTEGRATION_WITH_FPGA_EMULATION

#ifdef INTEGRATION_WITH_FPGA_EMULATION

#include "ftd2xx.h"
#include "libft4222.h"
#include <unistd.h>
#define f_exit(x) {printf(x); return 0;}
#define file_len 36101
#define size 6


 //FT4222H handle and vars
    FT_HANDLE ftHandle = NULL;
    FT_STATUS ftStatus;
    FT4222_STATUS ft4222Status;

#endif


#define BYTE_OFFSET_BETWEEN_TWO_CLUSTERS 16384 //8192*2// check this with thilini

//https://github.com/ecolab-nus/HyCUBE_RTL/blob/main/HyCUBE_8x8_8bit_22nm/rtl/design/microspeech_single/mem_alloc.txt
#define CLUSTER0_BASE_ADDRESS_O0 2560 
#define CLUSTER0_BASE_ADDRESS_I0 8192
#define CLUSTER0_BASE_ADDRESS_W0 0

#define CLUSTER1_BASE_ADDRESS_O1 (CLUSTER0_BASE_ADDRESS_O0 + BYTE_OFFSET_BETWEEN_TWO_CLUSTERS)
#define CLUSTER1_BASE_ADDRESS_I1 (CLUSTER0_BASE_ADDRESS_I0 + BYTE_OFFSET_BETWEEN_TWO_CLUSTERS )
#define CLUSTER1_BASE_ADDRESS_W1 (CLUSTER0_BASE_ADDRESS_W0 + BYTE_OFFSET_BETWEEN_TWO_CLUSTERS)
#define CLUSTER2_BASE_ADDRESS_O2 (CLUSTER0_BASE_ADDRESS_O0 + 2*BYTE_OFFSET_BETWEEN_TWO_CLUSTERS)
#define CLUSTER2_BASE_ADDRESS_I2 (CLUSTER0_BASE_ADDRESS_I0 + 2*BYTE_OFFSET_BETWEEN_TWO_CLUSTERS)
#define CLUSTER2_BASE_ADDRESS_W2 (CLUSTER0_BASE_ADDRESS_W0 + 2*BYTE_OFFSET_BETWEEN_TWO_CLUSTERS)
#define CLUSTER3_BASE_ADDRESS_O3 (CLUSTER0_BASE_ADDRESS_O0 + 3*BYTE_OFFSET_BETWEEN_TWO_CLUSTERS)
#define CLUSTER3_BASE_ADDRESS_I3 (CLUSTER0_BASE_ADDRESS_I0 + 3*BYTE_OFFSET_BETWEEN_TWO_CLUSTERS)
#define CLUSTER3_BASE_ADDRESS_W3 (CLUSTER0_BASE_ADDRESS_W0 + 3*BYTE_OFFSET_BETWEEN_TWO_CLUSTERS)
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

bool alreadysent = false;
int test_data_counter =  1;

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

void send_loopstart_and_cluster_enable(){
    // Time functions
    clock_t start, end;
    double time_used;


    //SPI data vars
    uint32 sizeOfRead;
    //uint16 sizeOfRead;
    uint8 wdata[file_len*size], rdata[file_len*size];
 
    #include "../../../../invocation2/invocation2.h"
    

    start = clock();
    for (int i=3327; i<file_len; i++) {
    	// Write data
    	//write start address and cluster enables
    	if( (i > 3327+8190 && i < 3327+8194) || (i > 3327+8190+8192 && i < 3327+8194+8192) || (i > 3327+8190+2*8192 && i < 3327+8194+2*8192) || (i > 3327+8190+3*8192) ){
		// if( (i > 3327+1280 && i < 3327+4096) || (i > 3327+4096 && i < 3327+8194) || (i > 3327+1280+8192 && i < 3327+4096+8192)|| (i > 3327+4096+8192 && i < 3327+8194+8192) || (i > 3327+1280+2*8192 && i < 3327+4096+2*8192) ||  (i > 3327+4096+2*8192 && i < 3327+8194+2*8192)|| (i > 3327+1280+3*8192) ){
		
			ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[size*i], &wdata[size*i], size,  &sizeOfRead, true);
    		if (FT_OK != ftStatus) f_exit("Write failed!\n");
    	}
    }
    end = clock();
    time_used = (double)(end-start)/CLOCKS_PER_SEC;
    //printf("Unwanted delay %f seconds\n", time_used);

}
//send 1 2B data elements with one address
void send_data_1_spi(int * data, int address){

    uint32 sizeOfRead;
    uint8_t databyte1,databyte2,databyte3,databyte4,databyte5,databyte6,databyte7,databyte8;
    uint8_t wdata[6];
    uint8_t rdata[6];
    uint8_t opcode_addrmsb,addrbyte1,addrbyte2;

    opcode_addrmsb = 0x12;//0001 0010  [17] = 1 for DM

    databyte1 = (*data & 0xFF00) >> 8;
    databyte2 = *data & 0x00FF;

            
    addrbyte1 = (address & 0xFF00) >> 8;

    uint8_t select_dmemmsb = (addrbyte1 & 0x80)>>7;// [15]
    uint8_t select_dmem = (addrbyte1 & 0xe0);//[15 14 13 0 0 0 0 0]
    uint8_t rest = addrbyte1 & 0x1f;//[0 0 0 12 11 10 9 8]
    uint8_t convertedaddress = (select_dmem << 1)|rest;//[14 13 0 12 11 10 9 8]
    //printf(" 15 14 0 0 | 0 0 0 0 : %2x\n", (select_dmem << 1));
    //printf(" 0 0 0 0| 0 0 0 16 : %2x\n", (select_dmemmsb));

    opcode_addrmsb = opcode_addrmsb | select_dmemmsb;
    //printf(" 23 22 21 20| 19 18 17 16 : %2x\n", (opcode_addrmsb));


    addrbyte1 = convertedaddress;


    addrbyte2 = address &  0x00FF;
    //printf("I0[%d]:%d : addr: %d data B1: %x B2: %x addr B1:%x B2:%x  \n", i, I0[i], addr, databyte1, databyte2, addrbyte1, addrbyte2);
    // printf("addr: %x | %x\n",addrbyte1,addrbyte2);
    // printf("data: %x | %x\n",databyte1,databyte2);
    /*
    printf("addr: %x | %x\n",addrbyte1,addrbyte2);
    printf("data: %x | %x\n",databyte1,databyte2);
    printf("data: %x | %x\n",databyte3,databyte4);
    printf("data: %x | %x\n",databyte5,databyte6);
    printf("data: %x | %x\n",databyte7,databyte8);
    */
    #ifdef INTEGRATION_WITH_FPGA_EMULATION

    // Write data
    wdata[0] = opcode_addrmsb; // {Opcode, Addr(MSB)}
    wdata[1] = addrbyte1; // Addr // Check endianess with Rohan
    wdata[2] = addrbyte2; // Addr
    wdata[3] = 0x00; // Size
    wdata[4] = databyte1; // Data
    wdata[5] = databyte2; // Data
    //if(address <= 8500)
     	//printf("send_data_1_spi Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t address %d\n",wdata[0],wdata[1],wdata[2],wdata[3],wdata[4],wdata[5], address );

    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("send_data_1_spi Write failed!\n");

    #endif
        

}

//send 4 2B data elements with one address
void send_data_4_spi(int * data, int address){

    uint32 sizeOfRead;
    uint8_t databyte1,databyte2,databyte3,databyte4,databyte5,databyte6,databyte7,databyte8;
    uint8_t wdata[12];
    uint8_t rdata[12];
    uint8_t addrbyte1,addrbyte2;

    databyte1 = (*data & 0xFF00) >> 8;
    databyte2 = *data & 0x00FF;
    data++;
    databyte3 = (*data  & 0xFF00) >> 8;
    databyte4 = *data  & 0x00FF;
    data++;
    databyte5 = (*data  & 0xFF00) >> 8;
    databyte6 = *data  & 0x00FF;
    data++;
    databyte7 = (*data  & 0xFF00) >> 8;
    databyte8 = *data  & 0x00FF;

            
    addrbyte1 = (address & 0xFF00) >> 8;
    addrbyte2 = address &  0x00FF;
    //printf("I0[%d]:%d : addr: %d data B1: %x B2: %x addr B1:%x B2:%x  \n", i, I0[i], addr, databyte1, databyte2, addrbyte1, addrbyte2);
    /*
    printf("addr: %x | %x\n",addrbyte1,addrbyte2);
    printf("data: %x | %x\n",databyte1,databyte2);
    printf("data: %x | %x\n",databyte3,databyte4);
    printf("data: %x | %x\n",databyte5,databyte6);
    printf("data: %x | %x\n",databyte7,databyte8);
    */
    #ifdef INTEGRATION_WITH_FPGA_EMULATION

    // Write data
    wdata[0] = 0x12; // {Opcode, Addr(MSB)}
    wdata[1] = addrbyte1; // Addr // Check endianess with Rohan
    wdata[2] = addrbyte2; // Addr
    wdata[3] = 0x3; // Size
    wdata[4] = databyte1; // Data
    wdata[5] = databyte2; // Data
    wdata[6] = databyte3; // Data
    wdata[7] = databyte4; // Data
    wdata[8] = databyte5; // Data
    wdata[9] = databyte6; // Data
    wdata[10] = databyte7; // Data
    wdata[11] = databyte8; // Data
    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("send_data_4_spi Write failed!\n");

    #endif
        

}



void read_spi_data_range(int start_addr, int range, int * array){
    #ifdef INTEGRATION_WITH_FPGA_EMULATION
    //SPI data vars
    uint32 sizeOfRead;
    uint8 wdata[size], rdata[size];
    uint8_t addrbyte1,addrbyte2,opcode_addrmsb;

///////////////////////////////////////////////////////////////


    wdata[0] = 0x18;
    wdata[1] = 0x00;	
    wdata[2] = 0x00;
    wdata[3] = 0x00;
    wdata[4] = 0x01;
    wdata[5] = 0x10;
	
    // Write start execution
    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("Write failed!\n");
	
    // printf("Start execution bit is written\n\n");
    
 
    

    // printf("Reading the start execution bit\n");
    wdata[0] = 0x08;
    wdata[1] = 0x00;	
    wdata[2] = 0x00;
    wdata[3] = 0x00;
    wdata[4] = 0x01;
    wdata[5] = 0x10;

    // Read start execution
    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("Read failed!\n");

    // printf("Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t Rdata %2x %2x\n\n", wdata[0], wdata[1], wdata[2], wdata[3], wdata[4], wdata[5], rdata[4], rdata[5]);


    // printf("Reading config register now.\n");
    wdata[0] = 0x00;
    wdata[1] = 0x00;
    wdata[2] = 0x06;
    wdata[3] = 0x00;
    wdata[4] = 0x40;
    wdata[5] = 0x00;
    
    // Read config register
    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("Read failed!\n");
    
    // printf("Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t Rdata %2x %2x\n\n", wdata[0], wdata[1], wdata[2], wdata[3], wdata[4], wdata[5], rdata[4], rdata[5]);
    



    //#include "../../../../invocation1/invocation1.h"
    // printf("Reading the data from the memories now.\n");
    wdata[0] = 0x10;
    wdata[1] = 0x00;
    wdata[2] = 0x06;
    wdata[3] = 0x00;
    wdata[4] = 0x40;
    wdata[5] = 0x00;
    
    

    int address = start_addr;
            
    //addrbyte1 = (address & 0xFF00) >> 8;
    addrbyte2 = address &  0x00FF;




    opcode_addrmsb = 0x02;//0001 0010  [17] = 1 for DM


            
    addrbyte1 = (address & 0xFF00) >> 8;

    uint8_t select_dmemmsb = (addrbyte1 & 0x80)>>7;// [15]
    uint8_t select_dmem = (addrbyte1 & 0xe0);//[15 14 13 0 0 0 0 0]
    uint8_t rest = addrbyte1 & 0x1f;//[0 0 0 12 11 10 9 8]
    uint8_t convertedaddress = (select_dmem << 1)|rest;//[14 13 0 12 11 10 9 8]
    //printf(" 15 14 0 0 | 0 0 0 0 : %2x\n", (select_dmem << 1));
    //printf(" 0 0 0 0| 0 0 0 16 : %2x\n", (select_dmemmsb));

    opcode_addrmsb = opcode_addrmsb | select_dmemmsb;
    //printf(" 23 22 21 20| 19 18 17 16 : %2x\n", (opcode_addrmsb));


    addrbyte1 = convertedaddress;

    for (int i=0; i<range; i++) {//file_len


    	wdata[0] = opcode_addrmsb;//0x02;
    	wdata[1] = addrbyte1;
    	wdata[2] = addrbyte2;
    	wdata[3] = 0x00;
    	wdata[4] = 0x00;
    	wdata[5] = 0x00;



    	
    	
    	// printf("%d, %x\t", i*6, wdata[i*6]);
	
    	    //Read data
    	ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    	    if (FT_OK != ftStatus) f_exit("Read failed!\n");



        address = address + 2;    
    	//addrbyte1 = (address & 0xFF00) >> 8;
    	addrbyte2 = address &  0x00FF;

    	addrbyte1 = (address & 0xFF00) >> 8;

    	select_dmemmsb = (addrbyte1 & 0x80)>>7;// [15]
    	select_dmem = (addrbyte1 & 0xe0);//[15 14 13 0 0 0 0 0]
    	rest = addrbyte1 & 0x1f;//[0 0 0 12 11 10 9 8]
    	convertedaddress = (select_dmem << 1)|rest;//[14 13 0 12 11 10 9 8]
    	// printf(" 15 14 0 0 | 0 0 0 0 : %2x\n", (select_dmem << 1));
    	// printf(" 0 0 0 0| 0 0 0 16 : %2x\n", (select_dmemmsb));

    	opcode_addrmsb = opcode_addrmsb | select_dmemmsb;
   		//printf(" 23 22 21 20| 19 18 17 16 : %2x\n", (opcode_addrmsb));


    	addrbyte1 = convertedaddress;

    	uint32 data_msb = rdata[4] << 8;
    	uint16 data_before_signextend = data_msb + rdata[5];
    	//*array = (data_before_signextend & 0xFF) << 7 >> 7; // sign extension
    	*array = data_before_signextend | ((data_before_signextend & 0x8000) ? 0xFFFF0000 : 0); //sign extend

    	//*array = data_msb + rdata[5];
    	array++;
    }
    

    #endif
}


void start_execution_and_wait(){
#ifdef INTEGRATION_WITH_FPGA_EMULATION
	uint32 sizeOfRead;
	uint8_t wdata[12];
    uint8_t rdata[12];

// //WRITE LOOPSTART (16383 = 3fff) 1
// 	wdata[0] = 0x10; // {Opcode, Addr(MSB)}
// 	wdata[1] = 0x3f; // Addr
// 	wdata[2] = 0xff; // Addr
// 	wdata[3] = 0x00; // Size
// 	wdata[4] = 0x00; // Data
// 	wdata[5] = 0x01; // Data    
// 	ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
//     if (FT_OK != ftStatus) f_exit("Write failed!\n");

//     printf("LOOPSTART bit is written\n\n");
	

    // printf("Reading the start execution bit\n");
    wdata[0] = 0x08;
    wdata[1] = 0x00;	
    wdata[2] = 0x00;
    wdata[3] = 0x00;
    wdata[4] = 0x01;
    wdata[5] = 0x10;

    // Read start execution
    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("Read failed!\n");

    
    // printf("Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t Rdata %2x %2x\n\n", wdata[0], wdata[1], wdata[2], wdata[3], wdata[4], wdata[5], rdata[4], rdata[5]);





    wdata[0] = 0x18;
    wdata[1] = 0x00;	
    wdata[2] = 0x00;
    wdata[3] = 0x00;
    wdata[4] = 0x01;
    wdata[5] = 0x11;
	
    // Write start execution
    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("Write failed!\n");
	
    // printf("Start execution bit is written\n\n");
    
    
    

    // printf("Reading the start execution bit\n");
    wdata[0] = 0x08;
    wdata[1] = 0x00;	
    wdata[2] = 0x00;
    wdata[3] = 0x00;
    wdata[4] = 0x01;
    wdata[5] = 0x11;

    // Read start execution
    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("Read failed!\n");

    
    // printf("Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t Rdata %2x %2x\n\n", wdata[0], wdata[1], wdata[2], wdata[3], wdata[4], wdata[5], rdata[4], rdata[5]);

    //rdata[4] = 0x00;
    //rdata[5] = 0x00;
    
    wdata[0] = 0x08;
    wdata[1] = 0x00;
    wdata[2] = 0x00;
    wdata[3] = 0x00;
    wdata[4] = 0x01;
    wdata[5] = 0x11;
    uint8_t end_exec;

    int counter = 0;
    while(1) {
	
	// Read the execution end bit
	ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    	if (FT_OK != ftStatus) f_exit("Read failed!\n");


	if(rdata[5] == 0x03) {
		// printf("rdata[4] %x\t rdata[5] %x\n\n", rdata[4], rdata[5]);
		break;
	}
    }

    printf("Observed exec end.\n");
   

#endif

}

int32_t O0[R1*(C2/P)], W0[R1*C1], I0[R2*(C2/P)];
int32_t O1[R1*(C2/P)], W1[R1*C1], I1[R2*(C2/P)];
int32_t O2[R1*(C2/P)], W2[R1*C1], I2[R2*(C2/P)];
int32_t O3[R1*(C2/P)], W3[R1*C1], I3[R2*(C2/P)];


int32_t O0_PACE[R1*(C2/P)], W0_PACE[R1*C1], I0_PACE[R2*(C2/P)];
int32_t O1_PACE[R1*(C2/P)], W1_PACE[R1*C1], I1_PACE[R2*(C2/P)];
int32_t O2_PACE[R1*(C2/P)];
int32_t O3_PACE[R1*(C2/P)];
int32_t FIRST_TWO_MEMS[8192];
int32_t ALL_EIGHT_MEMS[4*8192];
//int32_t R1C1C24P=R1*C1*C2/(4*P);
//int32_t C2P=C2/P;
//12 partitions


void conv_layer_pace(){
	printf("# Calling PACE convolution #\n");
	int i,j,k,ijk;
        uint8_t wdata[210000], rdata[210000];
        uint8_t databyte1,databyte2,databyte3,databyte4,databyte5,databyte6,databyte7,databyte8;
        uint8_t addrbyte1,addrbyte2;
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
        int uint_weight = 0;
        int uint_input = 0;
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
			//printf("I0[%d]\n",h*(C2/P)+w);
		}


    //remove this after finding the start address
	send_loopstart_and_cluster_enable();
	


	//SEND Weights
	if(alreadysent == false){
		printf("Writing Weights\n");
		for (int i = 0; i < R1*C1; i=i+1)
		{		
        	int addr = (i*2+CLUSTER0_BASE_ADDRESS_W0);// 16 bit address
        	send_data_1_spi(&W0[i], addr);
        	addr = (i*2+CLUSTER1_BASE_ADDRESS_W1);// 16 bit address
        	send_data_1_spi(&W1[i], addr);
        	addr = (i*2+CLUSTER2_BASE_ADDRESS_W2);// 16 bit address
        	send_data_1_spi(&W2[i], addr);
        	addr = (i*2+CLUSTER3_BASE_ADDRESS_W3);// 16 bit address
        	send_data_1_spi(&W3[i], addr);
        }
		alreadysent = true;
	}

	//SEND Inputs
	for (int i = 0; i < R2*C2/P; i=i+1){
	    int addr = (i*2+CLUSTER0_BASE_ADDRESS_I0);// 16 bit address
            send_data_1_spi(&I0[i], addr);
            addr = (i*2+CLUSTER1_BASE_ADDRESS_I1);// 16 bit address
            send_data_1_spi(&I1[i], addr);
            addr = (i*2+CLUSTER2_BASE_ADDRESS_I2);// 16 bit address
            send_data_1_spi(&I2[i], addr);
            addr = (i*2+CLUSTER3_BASE_ADDRESS_I3);// 16 bit address
            send_data_1_spi(&I3[i], addr);

        }


    //Initialize Outputs
	for (int i = 0; i < R1*C2/P; i=i+1){
	    int addr = (i*2+CLUSTER0_BASE_ADDRESS_O0);// 16 bit address
            send_data_1_spi(&O0[i], addr);
            addr = (i*2+CLUSTER1_BASE_ADDRESS_O1);// 16 bit address
            send_data_1_spi(&O1[i], addr);
            addr = (i*2+CLUSTER2_BASE_ADDRESS_O2);// 16 bit address
            send_data_1_spi(&O2[i], addr);
            addr = (i*2+CLUSTER3_BASE_ADDRESS_O3);// 16 bit address
            send_data_1_spi(&O3[i], addr);
        }

        

	// execute

	// printf("Writing SPI live data done\n");

    start_execution_and_wait();
	
	
    //READ Outputs
	read_spi_data_range((int)CLUSTER0_BASE_ADDRESS_O0, R1*(C2/P), O0_PACE);
	read_spi_data_range((int)CLUSTER1_BASE_ADDRESS_O1, R1*(C2/P), O1_PACE);
	read_spi_data_range((int)CLUSTER2_BASE_ADDRESS_O2, R1*(C2/P), O2_PACE);
	read_spi_data_range((int)CLUSTER3_BASE_ADDRESS_O3, R1*(C2/P), O3_PACE);
    // printf("Reading SPI live data done\n\n");



	//copy data back from O
	for(int h=0; h<R1; h++)
		for(int w=0; w<C2/P; w++){
			OUTPUT_MATRIX_[h*(C2)+w] = O0_PACE[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (C2/P)] = O1_PACE[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (2*C2/P)] = O2_PACE[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (3*C2/P)] = O3_PACE[h*(C2/P)+w];
		}


	printf("First four blocks done\n");
	// 2nd four blocks
	//initialize O
	for(int h=0; h<R1; h++)
		for(int w=0; w<C2/P; w++){
			O0[h*(C2/P)+w] = 0;
			O1[h*(C2/P)+w] = 0;
			O2[h*(C2/P)+w] = 0;
			O3[h*(C2/P)+w] = 0;
		}


	//copy input_m to I
	for(int h=0; h<R2; h++)
		for(int w=0; w<C2/P; w++){
			I0[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (4*C2/P)];
			I1[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (5*C2/P)];
			I2[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (6*C2/P)];
			I3[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (7*C2/P)];
		}



    //remove this after finding the start address
	send_loopstart_and_cluster_enable();

	//SEND Inputs
	for (int i = 0; i < R2*C2/P; i=i+1){
	    int addr = (i*2+CLUSTER0_BASE_ADDRESS_I0);// 16 bit address
            send_data_1_spi(&I0[i], addr);
            addr = (i*2+CLUSTER1_BASE_ADDRESS_I1);// 16 bit address
            send_data_1_spi(&I1[i], addr);
            addr = (i*2+CLUSTER2_BASE_ADDRESS_I2);// 16 bit address
            send_data_1_spi(&I2[i], addr);
            addr = (i*2+CLUSTER3_BASE_ADDRESS_I3);// 16 bit address
            send_data_1_spi(&I3[i], addr);

        }


    	//Initialize Outputs
	for (int i = 0; i < R1*C2/P; i=i+1){
	    int addr = (i*2+CLUSTER0_BASE_ADDRESS_O0);// 16 bit address
            send_data_1_spi(&O0[i], addr);
            addr = (i*2+CLUSTER1_BASE_ADDRESS_O1);// 16 bit address
            send_data_1_spi(&O1[i], addr);
			addr = (i*2+CLUSTER2_BASE_ADDRESS_O2);// 16 bit address
            send_data_1_spi(&O2[i], addr);
            addr = (i*2+CLUSTER3_BASE_ADDRESS_O3);// 16 bit address
            send_data_1_spi(&O3[i], addr);

        }

        

	// execute
    start_execution_and_wait();


	
	
	read_spi_data_range((int)CLUSTER0_BASE_ADDRESS_O0, R1*(C2/P), O0_PACE);
	read_spi_data_range((int)CLUSTER1_BASE_ADDRESS_O1, R1*(C2/P), O1_PACE);
	read_spi_data_range((int)CLUSTER2_BASE_ADDRESS_O2, R1*(C2/P), O2_PACE);
	read_spi_data_range((int)CLUSTER3_BASE_ADDRESS_O3, R1*(C2/P), O3_PACE);
    // printf("Reading SPI live data done\n\n");


	//copy data back from O
	for(int h=0; h<R1; h++)
		for(int w=0; w<C2/P; w++){
			OUTPUT_MATRIX_[h*(C2)+w+ (4*C2/P)] = O0_PACE[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (5*C2/P)] = O1_PACE[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (6*C2/P)] = O2_PACE[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (7*C2/P)] = O3_PACE[h*(C2/P)+w];
		}


	printf("Second four blocks done\n");

    // 3rd four blocks

	//initialize O
	for(int h=0; h<R1; h++)
		for(int w=0; w<C2/P; w++){
			O0[h*(C2/P)+w] = 0;
			O1[h*(C2/P)+w] = 0;
			O2[h*(C2/P)+w] = 0;
			O3[h*(C2/P)+w] = 0;
		}

	//copy input_m to I
	for(int h=0; h<R2; h++)
		for(int w=0; w<C2/P; w++){
			I0[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (8*C2/P)];
			I1[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (9*C2/P)];
			I2[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (10*C2/P)];
			I3[h*(C2/P)+w] = INPUT_MATRIX[h*(C2)+w + (11*C2/P)];
		}



    //remove this after finding the start address
	send_loopstart_and_cluster_enable();

    //SEND Inputs
	for (int i = 0; i < R2*C2/P; i=i+1)
	{
		//
		int addr = (i*2+CLUSTER0_BASE_ADDRESS_I0);// 16 bit address
        send_data_1_spi(&I0[i], addr);
        addr = (i*2+CLUSTER1_BASE_ADDRESS_I1);// 16 bit address
        send_data_1_spi(&I1[i], addr);
        addr = (i*2+CLUSTER2_BASE_ADDRESS_I2);// 16 bit address
        send_data_1_spi(&I2[i], addr);
        addr = (i*2+CLUSTER3_BASE_ADDRESS_I3);// 16 bit address
        send_data_1_spi(&I3[i], addr);

    }


    //Initialize Outputs
	for (int i = 0; i < R1*C2/P; i=i+1)
	{
		//
		int addr = (i*2+CLUSTER0_BASE_ADDRESS_O0);// 16 bit address
        send_data_1_spi(&O0[i], addr);
        addr = (i*2+CLUSTER1_BASE_ADDRESS_O1);// 16 bit address
        send_data_1_spi(&O1[i], addr);
        addr = (i*2+CLUSTER2_BASE_ADDRESS_O2);// 16 bit address
        send_data_1_spi(&O2[i], addr);
        addr = (i*2+CLUSTER3_BASE_ADDRESS_O3);// 16 bit address
        send_data_1_spi(&O3[i], addr);

    }

        

	// execute

	// printf("Writing SPI live data done\n");

    start_execution_and_wait();

	
	read_spi_data_range((int)CLUSTER0_BASE_ADDRESS_O0, R1*(C2/P), O0_PACE);
	read_spi_data_range((int)CLUSTER1_BASE_ADDRESS_O1, R1*(C2/P), O1_PACE);
	read_spi_data_range((int)CLUSTER2_BASE_ADDRESS_O2, R1*(C2/P), O2_PACE);
	read_spi_data_range((int)CLUSTER3_BASE_ADDRESS_O3, R1*(C2/P), O3_PACE);
    // printf("Reading SPI live data done\n\n");

	//copy data back from O
	for(int h=0; h<R1; h++)
		for(int w=0; w<C2/P; w++){
			OUTPUT_MATRIX_[h*(C2)+w+ (8*C2/P)] = O0_PACE[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (9*C2/P)] = O1_PACE[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (10*C2/P)] = O2_PACE[h*(C2/P)+w];
			OUTPUT_MATRIX_[h*(C2)+w+ (11*C2/P)] = O3_PACE[h*(C2/P)+w];
		}


	printf("Third four blocks done\n");


	//printf("Total number of invocations: %d\n",id-1);
	//return;
	//pthread_exit(NULL);
}




int setup_fpga_emulation(){
    /*
Following code segment is copied from 
https://github.com/ecolab-nus/Hycube_RTL_FPGA/blob/main/oct_2021/snapshot/sw/microspeech_single/microspeech.c
*/
   #ifdef INTEGRATION_WITH_FPGA_EMULATION
    // Time functions
    clock_t start, end;
    double time_used;


    //FT4222H handle and vars
    // FT_HANDLE ftHandle = NULL;
    // FT_STATUS ftStatus;
    // FT4222_STATUS ft4222Status;

    //Variables with GPIO direction
    GPIO_Dir gpioDir[4];
    gpioDir[0] = GPIO_INPUT;
    gpioDir[1] = GPIO_INPUT;
    gpioDir[2] = GPIO_OUTPUT;
    gpioDir[3] = GPIO_INPUT;

    //SPI data vars
    uint32 sizeOfRead;
    //uint16 sizeOfRead;
    uint8 wdata[file_len*size], rdata[file_len*size];
    double error=0,cnt=0;

    //Open FT4222H device
    //ftStatus = FT_OpenEx((PVOID)"FT4222 B",FT_OPEN_BY_DESCRIPTION, &ftHandle);
    ftStatus = FT_Open(1, &ftHandle);
    if (FT_OK != ftStatus) f_exit("Open a FT4222 device failed!\n");
    
    //Get chip version info
    //FT4222_Version ver;
    //FT4222_GetVersion(ftHandle, &ver);
    //printf("%x  %x\n",ver.chipVersion,ver.dllVersion);

    //Initialize FT4222 as SPI master
    //ft4222Status = FT4222_SPIMaster_Init(ftHandle, SPI_IO_QUAD, CLK_DIV_8, CLK_IDLE_LOW, CLK_LEADING, 0x2);
    ft4222Status = FT4222_SPIMaster_Init(ftHandle, SPI_IO_SINGLE, CLK_DIV_8, CLK_IDLE_LOW, CLK_LEADING, 0x2);
    if (FT4222_OK != ft4222Status) f_exit("Init FT4222 as SPI master device failed!\n");

    //Set CS polarity
    //ft4222Status = FT4222_SPIMaster_SetCS(ftHandle, CS_ACTIVE_NEGTIVE); //default
    ft4222Status = FT4222_SPIMaster_SetCS(ftHandle, CS_ACTIVE_POSTIVE);
    if (FT4222_OK != ft4222Status) f_exit("Init FT4222 CS polarity failed!\n");

    //Set clock
    ft4222Status = FT4222_SetClock(ftHandle, SYS_CLK_80);
    if (FT4222_OK != ft4222Status) f_exit("Init FT4222 clock freq set failed!\n");

    //Set signal driving strength
    ft4222Status = FT4222_SPI_SetDrivingStrength(ftHandle, DS_4MA, DS_4MA, DS_4MA);
    if (FT4222_OK != ft4222Status) f_exit("Init FT4222 signal strength set failed!\n");

    ////Initilize FT4222 GPIO function
    //FT4222_GPIO_Init(ftHandle, gpioDir);
    //FT4222_SetSuspendOut(ftHandle, false);//disable suspend out , enable gpio 2
    //FT4222_SetWakeUpInterrupt(ftHandle, false);//disable interrupt , enable gpio 3

    ////GPIO RW
    //BOOL value;
    //FT4222_GPIO_Read(ftHandle, (GPIO_Port)GPIO_PORT3, &value);
    //FT4222_GPIO_Write(ftHandle, (GPIO_Port)GPIO_PORT2, 1);

    //Clear prev pending transasctions if any
    ft4222Status = FT4222_SPI_ResetTransaction(ftHandle, 0);
    if (FT4222_OK != ft4222Status) f_exit("Clear FT4222 old transactions failed!\n");

    //Set flush latency (lower=faster)
    ftStatus = FT_SetLatencyTimer(ftHandle, 2);
    if (FT_OK != ftStatus) f_exit("Flush latency set failed!\n");

    //Unlock packet
    wdata[0]=0xF0; //{Opcode,ID MSB)
    wdata[1]=0x00; //ID
    wdata[2]=0x00; //ID
    wdata[3]=0x00; //ID

    //SPI write
    // ftStatus=FT4222_SPIMaster_MultiReadWrite(ftHandle, rdata, &wdata[0], 0, 2, 0, &sizeOfRead);
    ft4222Status= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], 2,  &sizeOfRead, true);
    if (FT4222_OK != ftStatus) f_exit("Write failed!\n");

        // Chip enable and resetn set to 0
        wdata[0]=0x18; //{Opcode,Addr(MSB)}
        wdata[1]=0x00; //Addr
        wdata[2]=0x00; //Addr
        wdata[3]=0x00; //Size
        wdata[4]=0x00;
	wdata[5]=0x00;
        
        ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], 4+2,  &sizeOfRead, true);
        if (FT_OK != ftStatus) f_exit("Write failed!\n");
        
        wdata[0]=0x08; //{Opcode,Addr(MSB)}
        //SPI read
        ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], 4+2,  &sizeOfRead, true);
        if (FT_OK != ftStatus) f_exit("Read failed!\n");

        //Read
        printf("S1: : %2x%2x () != %2x%2x (read)\n",wdata[4],wdata[5],rdata[4],rdata[5]);



        // Resentn set to 1
        wdata[0]=0x18; //{Opcode,Addr(MSB)}
        wdata[1]=0x00; //Addr
        wdata[2]=0x00; //Addr
        wdata[3]=0x00; //Size
        wdata[4]=0x01;
	wdata[5]=0x00;
        
        ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], 4+2,  &sizeOfRead, true);
        if (FT_OK != ftStatus) f_exit("Write failed!\n");
        
        wdata[0]=0x08; //{Opcode,Addr(MSB)}
        //SPI read
        ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], 4+2,  &sizeOfRead, true);
        if (FT_OK != ftStatus) f_exit("Read failed!\n");

        //Read
        printf("S2: : %2x%2x () != %2x%2x (read)\n",wdata[4],wdata[5],rdata[4],rdata[5]);



        // Chip_en set to 1
        wdata[0]=0x18; //{Opcode,Addr(MSB)}
        wdata[1]=0x00; //Addr
        wdata[2]=0x00; //Addr
        wdata[3]=0x00; //Size
        wdata[4]=0x01;
	wdata[5]=0x10;
        
        ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], 4+2,  &sizeOfRead, true);
        if (FT_OK != ftStatus) f_exit("Write failed!\n");
        
        wdata[0]=0x08; //{Opcode,Addr(MSB)}
        //SPI read
        ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], 4+2,  &sizeOfRead, true);
        if (FT_OK != ftStatus) f_exit("Read failed!\n");

        //Read
        printf("S3: : %2x%2x () != %2x%2x (read)\n",wdata[4],wdata[5],rdata[4],rdata[5]);
   


    //RW loop
    int s;
    #include "../../../../invocation1/invocation1.h"

    // Start writing
    start = clock();
    for (int i=0; i<3329; i++) {
    	// Write data
	ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[size*i], &wdata[size*i], size,  &sizeOfRead, true);
    	if (FT_OK != ftStatus) f_exit("Write failed!\n");
	
    }
    end = clock();
    time_used = (double)(end-start)/CLOCKS_PER_SEC;
    printf("Configuration is completely written through spi in %f seconds\n\n", time_used);


/*

    printf("Reading the start execution bit\n");
    wdata[0] = 0x08;
    wdata[1] = 0x00;	
    wdata[2] = 0x00;
    wdata[3] = 0x00;
    wdata[4] = 0x01;
    wdata[5] = 0x10;

    // Read start execution
    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("Read failed!\n");

    
    printf("Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t Rdata %2x %2x\n\n", wdata[0], wdata[1], wdata[2], wdata[3], wdata[4], wdata[5], rdata[4], rdata[5]);





    wdata[0] = 0x18;
    wdata[1] = 0x00;	
    wdata[2] = 0x00;
    wdata[3] = 0x00;
    wdata[4] = 0x01;
    wdata[5] = 0x11;
	
    // Write start execution
    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("Write failed!\n");
	
    printf("Start execution bit is written\n\n");
    
    
    

    printf("Reading the start execution bit\n");
    wdata[0] = 0x08;
    wdata[1] = 0x00;	
    wdata[2] = 0x00;
    wdata[3] = 0x00;
    wdata[4] = 0x01;
    wdata[5] = 0x11;

    // Read start execution
    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("Read failed!\n");

    
    printf("Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t Rdata %2x %2x\n\n", wdata[0], wdata[1], wdata[2], wdata[3], wdata[4], wdata[5], rdata[4], rdata[5]);

    //rdata[4] = 0x00;
    //rdata[5] = 0x00;
    
    wdata[0] = 0x08;
    wdata[1] = 0x00;
    wdata[2] = 0x00;
    wdata[3] = 0x00;
    wdata[4] = 0x01;
    wdata[5] = 0x11;
    uint8_t end_exec;

    int counter = 0;
    while(1) {
	
	// Read the execution end bit
	ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    	if (FT_OK != ftStatus) f_exit("Read failed!\n");


	if(rdata[5] == 0x03) {
		printf("rdata[4] %x\t rdata[5] %x\n\n", rdata[4], rdata[5]);
		break;
	}
    }

    printf("Observed execution end bit.\n\n");
   

    wdata[0] = 0x18;
    wdata[1] = 0x00;	
    wdata[2] = 0x00;
    wdata[3] = 0x00;
    wdata[4] = 0x01;
    wdata[5] = 0x10;
	
    // Write start execution
    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("Write failed!\n");
	
    printf("Start execution bit is written\n\n");
    
    
    

    printf("Reading the start execution bit\n");
    wdata[0] = 0x08;
    wdata[1] = 0x00;	
    wdata[2] = 0x00;
    wdata[3] = 0x00;
    wdata[4] = 0x01;
    wdata[5] = 0x10;

    // Read start execution
    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("Read failed!\n");

    printf("Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t Rdata %2x %2x\n\n", wdata[0], wdata[1], wdata[2], wdata[3], wdata[4], wdata[5], rdata[4], rdata[5]);



    printf("Reading config register now.\n");
    wdata[0] = 0x00;
    wdata[1] = 0x00;
    wdata[2] = 0x06;
    wdata[3] = 0x00;
    wdata[4] = 0x40;
    wdata[5] = 0x00;
    
    // Read config register
    ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[0], &wdata[0], size,  &sizeOfRead, true);
    if (FT_OK != ftStatus) f_exit("Read failed!\n");
    
    printf("Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t Rdata %2x %2x\n\n", wdata[0], wdata[1], wdata[2], wdata[3], wdata[4], wdata[5], rdata[4], rdata[5]);
    

*/

    // #include "../../../../invocation1/invocation1.h"
    // printf("Reading the data from the memories now.\n");
    // wdata[0] = 0x10;
    // wdata[1] = 0x00;
    // wdata[2] = 0x06;
    // wdata[3] = 0x00;
    // wdata[4] = 0x40;
    // wdata[5] = 0x00;
    
    /*printf("Printing array A.\n");
    for (int i=7680/6; i<=7794/6; i++) {
	wdata[size*i] -= 0x10;
	// printf("%d, %x\t", i*6, wdata[i*6]);
	
    	//Read data
	ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[size*i], &wdata[size*i], size,  &sizeOfRead, true);
    	if (FT_OK != ftStatus) f_exit("Read failed!\n");

	printf("Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t Rdata %2x %2x\n", wdata[size*i], wdata[size*i + 1], wdata[size*i + 2], wdata[size*i+3], wdata[size*i + 4], wdata[size*i + 5], rdata[size*i + 4], rdata[size*i + 5]);
    }
    
    printf("\n\nPrinting array B.\n");
    for (int i=7800/6; i<=7914/6; i++) {
	wdata[size*i] -= 0x10;
	// printf("%d, %x\t", i*6, wdata[i*6]);
	
    	//Read data
	ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[size*i], &wdata[size*i], size,  &sizeOfRead, true);
    	if (FT_OK != ftStatus) f_exit("Read failed!\n");

	printf("Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t Rdata %2x %2x\n", wdata[size*i], wdata[size*i + 1], wdata[size*i + 2], wdata[size*i+3], wdata[size*i + 4], wdata[size*i + 5], rdata[size*i + 4], rdata[size*i + 5]);
    }

    printf("\n\nPrinting array C.\n");
    for (int i=32256/6; i<=32370/6; i++) {
	wdata[size*i] -= 0x10;
	// printf("%d, %x\t", i*6, wdata[i*6]);
	
    	//Read data
	ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[size*i], &wdata[size*i], size,  &sizeOfRead, true);
    	if (FT_OK != ftStatus) f_exit("Read failed!\n");

	printf("Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t Rdata %2x %2x\n", wdata[size*i], wdata[size*i + 1], wdata[size*i + 2], wdata[size*i+3], wdata[size*i + 4], wdata[size*i + 5], rdata[size*i + 4], rdata[size*i + 5]);
    }*/

    //printf("\n\ndata.\n");
    /*
    for (int i=4600; i<5000; i++) {//file_len
	wdata[size*i] -= 0x10;
	// printf("%d, %x\t", i*6, wdata[i*6]);
	printf("i %d ", i);	

    	//Read data
	ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[size*i], &wdata[size*i], size,  &sizeOfRead, true);
    	if (FT_OK != ftStatus) f_exit("Read failed!\n");

	printf("Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t Rdata %2x %2x\n", wdata[size*i], wdata[size*i + 1], wdata[size*i + 2], wdata[size*i+3], wdata[size*i + 4], wdata[size*i + 5], rdata[size*i + 4], rdata[size*i + 5]);
    
    uint32 address_msb = wdata[size*i + 1]<<8;
    uint32 address = address_msb + wdata[size*i + 2];
    printf("%d\n",address );
    }
    */

    /*for (int i=0; i<file_len; i++) {//file_len
    	wdata[size*i] -= 0x10;
    	// printf("%d, %x\t", i*6, wdata[i*6]);
	
    	    //Read data
    	// ftStatus= FT4222_SPIMaster_SingleReadWrite(ftHandle, &rdata[size*i], &wdata[size*i], size,  &sizeOfRead, true);
    	//     if (FT_OK != ftStatus) f_exit("Read failed!\n");
	
    	uint32 address_msb = wdata[size*i + 1]<<8;
    	uint32 address = address_msb + wdata[size*i + 2];
  //   	if(address ==8190 || address == 2560){// >= 2560 && address < 2560+100){
  //   		printf("i %d ", i); 
  //   		printf("Addr %2x %2x %2x\t Size %2x\t Wdata %2x %2x\t Rdata %2x %2x\n", wdata[size*i], wdata[size*i + 1], wdata[size*i + 2], wdata[size*i+3], wdata[size*i + 4], wdata[size*i + 5], rdata[size*i + 4], rdata[size*i + 5]);
   	
  //   		printf("%d\n",address );
		// }
    }*/

    

    //Close device
    //FT4222_UnInitialize(ftHandle);
    //FT_Close(ftHandle);


    #endif
    return 0;

}


void main() {


  setup_fpga_emulation();
  printf("SETUP DONE\n");
  printf("-----------MICROSPEECH APPLICATION START------\n");

   // FT4222_UnInitialize(ftHandle);
   //  FT_Close(ftHandle);
   //  exit(0);


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
    printf("\n------Test data %d--------\n", test_data_counter++);
    init_input();
    //read conv1 param
    read_conv_param();    

    //perform convolution
    microspeech_conv_layer();// executed on host
    conv_layer_pace();// offloaded to PACE

    for (int i=0;i<R1; i++)
        for (int j=0; j<C2; j++) {
            if (OUTPUT_MATRIX_[(i)*C2+(j)]!=OUTPUT_MATRIX_EXP[(i)*C2+(j)])
            {
               printf("i:%d j:%d INCORRECT %d,%d\n",i,j,OUTPUT_MATRIX_EXP[(i)*C2+(j)],OUTPUT_MATRIX_[(i)*C2+(j)]);
               incorrect++;
	    	}else{
        	   //printf("i:%d j:%d CORRECT %d,%d\n",i,j,OUTPUT_MATRIX_EXP[(i)*C2+(j)],OUTPUT_MATRIX_[(i)*C2+(j)]);
        	   correct++;
	    	}
        }

    for (int i=0;i<R1; i++)
		for (int j=0; j<500; j++){
			OUTPUT_MATRIX[(i)*500+(j)] =OUTPUT_MATRIX_[(i)*C2+(j)];// OUTPUT_MATRIX_[(i)*C2+(j)];
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
    	printf("Prediction: CORRECT \n");
    } else{
    	printf("Prediction: WRONG\n");
    }
    }
  }
  printf("\n\nFinal model accuracy: %f\n", (accuracy/c)*100);
  printf("Correct: %d , Incorrect: %d\n", correct,incorrect);

  closedir(dir);
    #ifdef INTEGRATION_WITH_FPGA_EMULATION
    //Close device
    FT4222_UnInitialize(ftHandle);
    FT_Close(ftHandle);
    #endif
}

