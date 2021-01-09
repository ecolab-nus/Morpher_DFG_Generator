#include <stdio.h>

#define SIZE 30

int A[SIZE];
int B[SIZE];

void init() {
	int i;
	for (i=0;i<SIZE;i++){
	
		A[i] = 0;
		B[i] = 0;
	}
}

int main() {
	int i;
	int j;

	A[i] = 2;
	B[i] = 3;

	for (i=1;i<SIZE;i++){
			#ifdef CGRA_COMPILER
           please_map_me();
           #endif
		A[i] = A[i-1] + 3;
		B[i] = i^2;
	}
}
