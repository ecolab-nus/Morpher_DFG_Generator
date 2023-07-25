#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <pthread.h>
#include <stdint.h>
#include <dirent.h>



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

//For layer 1
//input size (40, 40, 1) after padding --> (42, 42, 1)
int8_t INPUT_MATRIX[I1*I2*1];
//weight matrix (3, 3, 1, 35)
int8_t WEIGHT_MATRIX[K * K * Cin * Co];
//output matrix (40, 40, 35)
int16_t OUTPUT_MATRIX[O1*O2*Co];

//############################################################################################################################################

// __attribute__((noinline))
void conv() {
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





void main() {



    for (int i=0; i<I1*I2; i++) {
        INPUT_MATRIX[i] = 1;
    }

    for (int i=0; i<K*K*Cin*Co; i++) {
        WEIGHT_MATRIX[i] = i;
    }

    conv();

    for (int k=0; k<Co; k++) {
        for (int i=0; i<O1; i++) {
            for (int j=0; j<O2; j++) {
                printf("%d ", OUTPUT_MATRIX[k*O1*O2 + i*O2 + j]);
            }
            printf("\n");
        }
    }


}
