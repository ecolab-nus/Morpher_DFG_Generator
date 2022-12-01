#include <string.h>
#include <stdio.h>


#define SIZE  20
int A[SIZE], B[SIZE], C[SIZE];

void external_fun() {C[0] = 7;}

__attribute__((noinline))
void array_cond(){

   
   for (int i=0;i<SIZE; i++){
      #ifdef CGRA_COMPILER
      please_map_me();
      #endif

	if (A[i] > B[i])
		C[i] = A[i] - B[i];
	else
		C[i] = A[i] + B[i];

/*
	external_fun();
	if (A[i] > B[i])
		C[i] = 5;
	else
		C[i] = 7;
*/
/*
	if (B[i] > 4)
		C[i] = A[i] - B[i];
	else
		C[i] = 4;
*/
   }


}

int main(){

int i,j;

for (i=0;i<SIZE; i++){
      A[i] = i * 2 + 5;
      B[i] = i * 3;
      C[i] = 0;
    }
    
array_cond();

for (i=0;i<SIZE; i++) printf("%d\n", C[i]);
    

return 0;

}
