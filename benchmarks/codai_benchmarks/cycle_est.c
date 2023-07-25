#include <string.h>
#include <stdlib.h>
#include <unistd.h>  
#include <pthread.h>
#include <stdint.h>
#include <dirent.h>


#define SIZE  64
#define M 64
#define N 16
#define K 64
int16_t I[M*K];
int16_t W[K*N];
int16_t O[M*N];

    //input dimension I1 42
    #define I1  64
    //input dimension I2 42
    #define I2  64
    //input channels Ci 1
    #define Cin 1
    //output channels Co 35
    #define Co  1
//35
    //stride S 1
    #define S 1
    //kernel size K 3
    #define K1 3
    #define K2 3
    //output dimension O1 40
    #define O1  62
    //output dimension O2 40
    #define O2  62

//############################################################################################################################################

//For layer 1
//input size (40, 40, 1) after padding --> (42, 42, 1)
int8_t INPUT_MATRIX[I1*I2*1];
//weight matrix (3, 3, 1, 35)
int8_t WEIGHT_MATRIX[K * K * Cin * Co];
//output matrix (40, 40, 35)
int16_t OUTPUT_MATRIX[O1*O2*Co];


__attribute__((noinline))
void gemm(){
   int i,j,k;

   for (i=0;i<M; i++){
      for (j=0;j<N; j++){
        for (k=0;k<K; k++){
           #ifdef CGRA_COMPILER
           please_map_me();
           #endif
           O[i*N+j] =+ I[i*K+k]* W[k*N+j];
        }}}

}

void gemm1_cycle_est(){
   int i,j,k;
   int ops = 0;
   int ops_per_iter = 26;
   int cycles = 0;
   int overheadBytes_perInvo = 20;
   int overheadBytes = 0;
   int ii = 4;
   int latency = 10;

   for (i=0;i<M; i++){
      for (j=0;j<N; j++){
        overheadBytes += overheadBytes_perInvo;
        for (k=0;k<K; k++){
           O[i*N+j] += I[i*K+k]* W[k*N+j];
           cycles += ii;
           ops += ops_per_iter;
        }
        cycles += latency;
        }}
        
   printf("gemm 1 cycles: %d ops: %d overhead bytes: %d\n", cycles, ops, overheadBytes*4);

}

void gemm2_cycle_est(){
   int i,j,k;
   int ops = 0;
   int ops_per_iter = 58;
   int cycles = 0;
   int overheadBytes_perInvo = 20;
   int overheadBytes = 0;
   int ii = 6;
   int latency = 22;

   for (i=0;i<M; i++){
      for (j=0;j<N; j++){
        overheadBytes += overheadBytes_perInvo;
        for (k=0;k<K; k=k+4){
           cycles += ii;
           ops += ops_per_iter;
           
        }
        cycles += latency;
        }}
        
   printf("gemm 2 cycles: %d ops: %d overhead bytes: %d\n", cycles, ops, overheadBytes*4);

}

void gemm3_cycle_est(){

   int cycles = 0;
   int ops = 0;
   int ops_per_iter = 79;
   int ii = 8;
   int latency = 20;
   int i,j,k,ijk;
   i=0;j=0;k=0;
   int overheadBytes_perInvo = 20;
   int overheadBytes = 0;
   overheadBytes += overheadBytes_perInvo;
   for (ijk=0;ijk<M*N*K/4; ijk++){
           ops += ops_per_iter;
      O[i*N+j] = O[i*N+j] + I[i*K+k]* W[k*N+j] + I[i*K+k+1]* W[(k+1)*N+j] + I[i*K+k+2]* W[(k+2)*N+j] + I[i*K+k+3]* W[(k+3)*N+j];
      k=k+4;
      if(k+1 >= K){
	k=0;
	++j;
      }
      if(j == N){
	j=0;
	++i;
      }
      cycles += ii;
   }
   cycles += latency;
   
   printf("gemm 3 cycles: %d ops: %d overhead bytes: %d\n", cycles, ops, overheadBytes*4);

}

void conv1_cycle_est() {
   int cycles = 0;
   int overheadBytes_perInvo = 20;
   int overheadBytes = 0;
   int ops = 0;
   int ops_per_iter = 27;
   int ii = 4;
   int latency = 10;
   
    for (int i=0; i<O1; i++) {
        for (int j=0; j<O2; j++) {
            for (int cout=0; cout<Co; cout++) {
                int temp = 0;
                for (int k1=0; k1<K1; k1++) {
                    overheadBytes += overheadBytes_perInvo;
                    for (int k2=0; k2<K2; k2++) {
                        #ifdef CGRA_COMPILER
                	please_map_me();
                	#endif
                        temp += INPUT_MATRIX[(j * S + k2) + (i * S + k1) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + k2 + k1 * K2]; 
      			cycles += ii;
           		ops += ops_per_iter;
                    }
   		    cycles += latency;
                }
                OUTPUT_MATRIX[i + j*O1 + cout*O1*O2] = temp;
            }
        }
    }
    
    
   printf("conv 1 cycles: %d ops: %d overhead bytes: %d\n", cycles, ops*16, overheadBytes*64);
}

void conv2_cycle_est() {
   int cycles = 0;
   int overheadBytes_perInvo = 20;
   int overheadBytes = 0;
   int ops = 0;
   int ops_per_iter = 100;
   int ii = 12;
   int latency = 10;
   

    
    for (int cout=0; cout<Co; cout++) {
    for (int i=0; i<O1; i++) {
        overheadBytes += overheadBytes_perInvo;
        for (int j=0; j<O2; j++) {
                        #ifdef CGRA_COMPILER
                	please_map_me();
                	#endif
                
        OUTPUT_MATRIX[i + j*O1+ cout*O1*O2] = INPUT_MATRIX[(j * S + 0) + (i * S + 0) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 0 + 0 * K2]
        + INPUT_MATRIX[(j * S + 1) + (i * S + 0) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 1 + 0 * K2]
        + INPUT_MATRIX[(j * S + 2) + (i * S + 0) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 2 + 0 * K2]
        
        + INPUT_MATRIX[(j * S + 0) + (i * S + 1) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 0 + 1 * K2]
        + INPUT_MATRIX[(j * S + 1) + (i * S + 1) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 1 + 1 * K2]
        + INPUT_MATRIX[(j * S + 2) + (i * S + 1) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 2 + 1 * K2]
        
        + INPUT_MATRIX[(j * S + 0) + (i * S + 2) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 0 + 2 * K2]
        + INPUT_MATRIX[(j * S + 1) + (i * S + 2) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 1 + 2 * K2]
        + INPUT_MATRIX[(j * S + 2) + (i * S + 2) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 2 + 2 * K2];
      			cycles += ii;
           ops += ops_per_iter;
            }
   		    cycles += latency;
        }
    }
    
    
   printf("conv 2 cycles: %d ops: %d overhead bytes: %d\n", cycles, ops*16, overheadBytes*64);
}

void conv3_cycle_est() {
   int cycles = 0;
   int overheadBytes_perInvo = 20;
   int overheadBytes = 0;
   int ops = 0;
   int ops_per_iter = 153;
   int ii = 10;
   int latency = 10;

    
    
    int i,j,cout;
    i=0; j=0; cout=0;
    
                    overheadBytes += overheadBytes_perInvo;
    for (int ijk=0;ijk<Co*O1*O2; ijk++){
        #ifdef CGRA_COMPILER
        please_map_me();
        #endif
        OUTPUT_MATRIX[i + j*O1+ cout*O1*O2] = INPUT_MATRIX[(j * S + 0) + (i * S + 0) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 0 + 0 * K2]
        + INPUT_MATRIX[(j * S + 1) + (i * S + 0) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 1 + 0 * K2]
        + INPUT_MATRIX[(j * S + 2) + (i * S + 0) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 2 + 0 * K2]
        
        + INPUT_MATRIX[(j * S + 0) + (i * S + 1) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 0 + 1 * K2]
        + INPUT_MATRIX[(j * S + 1) + (i * S + 1) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 1 + 1 * K2]
        + INPUT_MATRIX[(j * S + 2) + (i * S + 1) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 2 + 1 * K2]
        
        + INPUT_MATRIX[(j * S + 0) + (i * S + 2) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 0 + 2 * K2]
        + INPUT_MATRIX[(j * S + 1) + (i * S + 2) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 1 + 2 * K2]
        + INPUT_MATRIX[(j * S + 2) + (i * S + 2) * I2] * WEIGHT_MATRIX[cout * K1 * K2 + 2 + 2 * K2];
        j = j + 1;
        if(j+1 > O2){
            j=0;
            ++i;
        }
        if(i == O1){
          i=0;
          ++cout;
        }
      			cycles += ii;
           ops += ops_per_iter;
    }
    
   		    cycles += latency;
    
   printf("conv 3 cycles: %d ops: %d overhead bytes: %d\n", cycles, ops*16, overheadBytes*64);
}


void main(){

int i,j;
for (i=0;i<M; i++)
   for (j=0;j<N; j++) {
      O[i*N+j]=  0;
    }
    
for (i=0;i<M; i++)
   for (j=0;j<K; j++) {
      I[i*K+j]=  i*j;
    }
    
for (i=0;i<K; i++)
   for (j=0;j<N; j++) {
      I[i*N+j]=  i+j;
    }
    
gemm();
gemm1_cycle_est();
gemm2_cycle_est();
gemm3_cycle_est();


for (int i=0; i<I1*I2; i++) {
  INPUT_MATRIX[i] = 1;
}

for (int i=0; i<K*K*Cin*Co; i++) {
  WEIGHT_MATRIX[i] = i;
}

conv1_cycle_est();
conv2_cycle_est();
conv3_cycle_est();


for (i=0;i<M; i++)
   for (j=0;j<N; j++) {
      //printf("%d\n", O[i*N+j]);
    }

}


