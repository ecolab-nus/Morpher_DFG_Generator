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


// __attribute__((noinline))
// void gemm(){
//    int i,j,k;

//    for (i=0;i<M; i++){
//       for (j=0;j<N; j++){
//         for (k=0;k<K; k=k+4){
//            #ifdef CGRA_COMPILER
//            please_map_me();
//            #endif
//            O[i*N+j] = O[i*N+j] + I[i*K+k]* W[k*N+j] + I[i*K+k+1]* W[(k+1)*N+j] + I[i*K+k+2]* W[(k+2)*N+j] + I[i*K+k+3]* W[(k+3)*N+j];
//         }}}

// }

__attribute__((noinline))
void gemm(){
   int i,j,k;

   for (i=0;i<M; i++){
      for (j=0;j<N; j++){
        for (k=0;k<K; k=k+8){
           #ifdef CGRA_COMPILER
           please_map_me();
           #endif
           O[i*N+j] = O[i*N+j] + I[i*K+k]* W[k*N+j] + I[i*K+k+1]* W[(k+1)*N+j] + I[i*K+k+2]* W[(k+2)*N+j] + I[i*K+k+3]* W[(k+3)*N+j]
                      + I[i*K+k+4]* W[(k+4)*N+j] + I[i*K+k+5]* W[(k+5)*N+j] + I[i*K+k+6]* W[(k+6)*N+j] + I[i*K+k+7]* W[(k+7)*N+j];
        }}}

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

for (i=0;i<M; i++)
   for (j=0;j<N; j++) {
      //printf("%d\n", O[i*N+j]);
    }

}



