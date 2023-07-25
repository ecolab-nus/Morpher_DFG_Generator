#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
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

    //input dimension I1 42
    #define I1  42
    //input dimension I2 42
    #define I2  42
    //input channels Ci 1
    #define Cin 1
    //output channels Co 35
    #define Co  1
//35
    //stride S 1
    #define S 1
    //kernel size K 3
    #define K 3
    //output dimension O1 40
    #define O1  40
    //output dimension O2 40
    #define O2  40

//############################################################################################################################################
//define weight parameters

//param1 conv1 size 315  dim(3, 3, 1, 35) elements 
const int param1[315];
//param2 bias1 size 35 elements
const int param2[35];

//param3 BN_mult size 35 elements
const int param3[35];
//param4 BN_add size 35 elements
const int param4[35];

//param5 conv1 size 15750  dim(3, 3, 35, 50) elements 
const int param5[15750];
//param6 bias1 size 50 elements
const int param6[50];

//param7 BN_mult size 50 elements
const int param7[50];
//param8 BN_add size 50 elements
const int param8[50];

//param9 fc_layer size (18, 1250) elements
const int param9[22500];
//param10 bias2 size 18 elements
const int param10[18];

//For layer 1
//input size (40, 40, 1) after padding --> (42, 42, 1)
int INPUT_MATRIX[I1*I2*1];
//weight matrix (3, 3, 1, 35)
int WEIGHT_MATRIX[K * K * Cin * Co];
//output matrix (40, 40, 35)
// int OUTPUT_MATRIX[O1*O2*Co];
int OUTPUT_MATRIX[O1*O2*Co];
int OUTPUT_MATRIX_EXP[O1*O2*Co];

//############################################################################################################################################
//GEMM kernel
    // for (i=0;i<R1; i++)
    //     for (j=0;j<C2/P; j++)
    //         for (k=0;k<C1; k++){
    //             O0[i*C2/P+j] += W0[i*C1+k]* I0[k*C2/P+j];
    //         }


    // // Flattened and Unrolled GEMM kernel
    // i=0;j=0;k=0;
    // for (ijk=0;ijk<R1C1C24P; ijk++){
    //     #ifdef CGRA_COMPILER
    //     please_map_me();
    //     #endif
    //     O0[i*C2P+j] = O0[i*C2P+j] + W0[i*C1+k]* I0[k*C2P+j]+ W0[i*C1+k+1]* I0[(k+1)*C2P+j]+ W0[i*C1+k+2]* I0[(k+2)*C2P+j]+W0[i*C1+k+3]* I0[(k+3)*C2P+j];
    //     k=k+4;
    //     if(k+1 >= C1){
    //       k=0;
    //       ++j;
    //     }
    //     if(j == C2P){
    //       j=0;
    //       ++i;
    //     }
    // }

void conv_layer1() {
    int i,j,cout;
    i=0; j=0; cout=0;

    for (int ijk=0;ijk<Co*O1*O2; ijk++){
        #ifdef CGRA_COMPILER
        please_map_me();
        #endif
        OUTPUT_MATRIX[i + j*O1+ cout*O1*O2] = INPUT_MATRIX[(j * S + 0) + (i * S + 0) * I2] * WEIGHT_MATRIX[cout * K * K + 0 + 0 * K]
        + INPUT_MATRIX[(j * S + 1) + (i * S + 0) * I2] * WEIGHT_MATRIX[cout * K * K + 1 + 0 * K]
        + INPUT_MATRIX[(j * S + 2) + (i * S + 0) * I2] * WEIGHT_MATRIX[cout * K * K + 2 + 0 * K]
        
        + INPUT_MATRIX[(j * S + 0) + (i * S + 1) * I2] * WEIGHT_MATRIX[cout * K * K + 0 + 1 * K]
        + INPUT_MATRIX[(j * S + 1) + (i * S + 1) * I2] * WEIGHT_MATRIX[cout * K * K + 1 + 1 * K]
        + INPUT_MATRIX[(j * S + 2) + (i * S + 1) * I2] * WEIGHT_MATRIX[cout * K * K + 2 + 1 * K]
        
        + INPUT_MATRIX[(j * S + 0) + (i * S + 2) * I2] * WEIGHT_MATRIX[cout * K * K + 0 + 2 * K]
        + INPUT_MATRIX[(j * S + 1) + (i * S + 2) * I2] * WEIGHT_MATRIX[cout * K * K + 1 + 2 * K]
        + INPUT_MATRIX[(j * S + 2) + (i * S + 2) * I2] * WEIGHT_MATRIX[cout * K * K + 2 + 2 * K];
        j = j + 1;
        if(j+1 > O2){
            j=0;
            ++i;
        }
        if(i == O1){
          i=0;
          ++cout;
        }
    }
}

void conv_layer1_orig() {


    for (int i=0; i<O1; i++) {
        for (int j=0; j<O2; j++) {
            for (int cout=0; cout<Co; cout++) {
                int temp = 0;
                for (int k1=0; k1<K; k1++) {
                    for (int k2=0; k2<K; k2++) {
                        temp += INPUT_MATRIX[(j * S + k2) + (i * S + k1) * I2] * WEIGHT_MATRIX[cout * K * K + k2 + k1 * K]; 
                    }
                }
                OUTPUT_MATRIX_EXP[i + j*O1 + cout*O1*O2] = temp;
            }
        }
    }
}

// #define SIZE  20
// int A[SIZE], B[SIZE], C[SIZE];

// __attribute__((noinline))
// void conv_layer1(){

   
//    for (int cout=0;cout<Co; cout++){
//       #ifdef CGRA_COMPILER
//       please_map_me();
//       #endif

//       OUTPUT_MATRIX[cout] += INPUT_MATRIX[cout]*WEIGHT_MATRIX[cout];
//    }


// }


//############################################################################################################################################
// void conv_layer2(int INPUT_MATRIX[21*21*35], int WEIGHT_MATRIX[3 * 3 * 35 * 50], int OUTPUT_MATRIX[10*10*50]) {

//     //input I1 21
//     int I1 = 21;
//     //input I2 21
//     int I2 = 21;
//     //input channels Ci 35
//     int Cin = 35;
//     //output channels Co 50
//     int Co = 50;
//     //stride S 2
//     int S = 2;
//     //kernel size K 3
//     int K = 3;
//     //output O1 10
//     int O1 = 10;
//     //output O2 10
//     int O2 = 10;

//     for (int i=0; i<O1; i++) {
//         for (int j=0; j<O2; j++) {
//             for (int cout=0; cout<Co; cout++) {
//                 int temp = 0;
//                 for (int cin=0; cin<Cin; cin++) {
//                     for (int k1=0; k1<K; k1++) {
//                         for (int k2=0; k2<K; k2++) {
//                             temp += INPUT_MATRIX[(j * S + k2) + (i * S + k1) * I2 + cin * I1 * I2] * WEIGHT_MATRIX[k2 + k1 * K + cin * K * K + cout * K * K * Cin]; 
//                         }
//                     }
//                 }
//                 OUTPUT_MATRIX[i + j*O1 + cout*O1*O2] = temp;
//             }
//         }
//     }
// }

void main() {

    // //For layer 1
    // //input size (40, 40, 1) after padding --> (42, 42, 1)
    // int INPUT_MATRIX[42*42*1];
    // //weight matrix (3, 3, 1, 35)
    // int WEIGHT_MATRIX[3 * 3 * 1 * 35];
    // //output matrix (40, 40, 35)
    // int OUTPUT_MATRIX[40*40*35];

    for (int i=0; i<I1*I2; i++) {
        INPUT_MATRIX[i] = 1;
    }

    for (int i=0; i<K*K*Cin*Co; i++) {
        WEIGHT_MATRIX[i] = i;
    }

    conv_layer1();//(INPUT_MATRIX, WEIGHT_MATRIX, OUTPUT_MATRIX);
    conv_layer1_orig();
    int error_count = 0;
    for (int k=0; k<Co; k++) {
        for (int i=0; i<O1; i++) {
            for (int j=0; j<O2; j++) {
                if(OUTPUT_MATRIX_EXP[k*O1*O2 + i*O2 + j] != OUTPUT_MATRIX[k*O1*O2 + i*O2 + j]){
                    error_count++;
                    // printf("%d %d %d exp: %d res: %d\n", k,i,j,OUTPUT_MATRIX_EXP[k*O1*O2 + i*O2 + j], OUTPUT_MATRIX[k*O1*O2 + i*O2 + j]);
                }  
            }
            // printf("\n");
        }
    }

    printf("Error count %d\n", error_count);

    //For layer 2
    //input size (20, 20, 1) after padding --> (21, 21, 1)
    // int INPUT_MATRIX2[21*21*35];
    // //weight matrix (3, 3, 35, 50)
    // int WEIGHT_MATRIX2[3 * 3 * 35 * 50];
    // //output matrix (10, 10, 50)
    // int OUTPUT_MATRIX2[10*10*50];

    // for (int i=0; i<21*21*35; i++) {
    //     INPUT_MATRIX2[i] = 2;
    // }

    // for (int i=0; i<3*3*35*50; i++) {
    //     WEIGHT_MATRIX2[i] = 1;
    // }

    // conv_layer2(INPUT_MATRIX2, WEIGHT_MATRIX2, OUTPUT_MATRIX2);

    // for (int k=0; k<50; k++) {
    //     for (int i=0; i<10; i++) {
    //         for (int j=0; j<10; j++) {
    //             printf("%d ", OUTPUT_MATRIX2[k*10*10 + i*10 + j]);
    //         }
    //         printf("\n");
    //     }
    // }

}
