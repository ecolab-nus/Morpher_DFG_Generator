#include <string.h>
#include <stdlib.h>
#include <unistd.h>  
#include <pthread.h>
#include <stdint.h>
#include <dirent.h>


#define SIZE  64
#define M 63
#define N 15
#define K 63
int16_t I[M*K];
int16_t W[K*N];
int16_t O[M*N];


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

void gemm_cycle_est(){
   int i,j,k;
   int cycles = 0;
   int ii = 4;
   int latency = 10;

   for (i=0;i<M; i++){
      for (j=0;j<N; j++){
        for (k=0;k<K; k++){
           O[i*N+j] += I[i*K+k]* W[k*N+j];
           cycles += ii;
        }
        cycles += latency;
        }}
        
   printf("cycles: %d \n", cycles);

}

void main(){

int i,j;
for (i=0;i<M; i++)
   for (j=0;j<N; j++) {
      O[i*N+j]=  0;
    }
    
for (i=0;i<M; i++)
   for (j=0;j<K; j++) {
      I[i*K+j]=  0;
    }
    
for (i=0;i<K; i++)
   for (j=0;j<N; j++) {
      I[i*N+j]=  0;
    }
    
gemm();
gemm_cycle_est();

for (i=0;i<M; i++)
   for (j=0;j<N; j++) {
      //printf("%d\n", O[i*N+j]);
    }

}


