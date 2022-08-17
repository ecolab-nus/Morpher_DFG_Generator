/**
 * trmm.c: This file is part of the PolyBench 3.0 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
// #include "polybench.h"

# define MINI_DATASET

/* Default to STANDARD_DATASET. */
# if !defined(MINI_DATASET) && !defined(SMALL_DATASET) && !defined(LARGE_DATASET) && !defined(EXTRALARGE_DATASET)
#  define STANDARD_DATASET
# endif

/* Do not define anything if the user manually defines the size. */
# if !defined(NI)
/* Define the possible dataset sizes. */
#  ifdef MINI_DATASET
#   define NI 32
#  endif

#  ifdef SMALL_DATASET
#   define NI 128
#  endif

#  ifdef STANDARD_DATASET /* Default if unspecified. */
#   define NI 1024
#  endif

#  ifdef LARGE_DATASET
#   define NI 2000
#  endif

#  ifdef EXTRALARGE_DATASET
#   define NI 4000
#  endif
# endif /* !N */


# ifndef DATA_TYPE
#  define DATA_TYPE double
// #  define DATA_TYPE int
#  define DATA_PRINTF_MODIFIER "%0.2lf "
// #  define DATA_PRINTF_MODIFIER "%d"
# endif

DATA_TYPE A[NI*NI];
DATA_TYPE B[NI*NI];
DATA_TYPE alpha;
int ni = NI;

/* Array initialization. */
static
void init_array()
{
  int i, j;

  alpha = 32412;
  for (i = 0; i < ni; i++)
    for (j = 0; j < ni; j++) {
      A[i*NI+j] = ((DATA_TYPE) i*j) / ni;
      B[i*NI+j] = ((DATA_TYPE) i*j) / ni;
    }
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array()
{
  int i, j;

  for (i = 0; i < ni; i++)
    for (j = 0; j < ni; j++) {
	fprintf (stderr, DATA_PRINTF_MODIFIER, B[i*NI+j]);
	if ((i * ni + j) % 20 == 0) fprintf (stderr, "\n");
    }
  fprintf (stderr, "\n");
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
__attribute__((noinline))
static
void trmm()
{
  int i, j, k;

// #pragma scop
  /*  B := alpha*A'*B, A triangular */
  for (i = 1; i < ni; i++)
    for (j = 0; j < ni; j++)
      for (k = 0; k < i; k++)
#ifdef CGRA_COMPILER
please_map_me();
#endif 
        B[i*NI+j] += alpha * A[i*NI+k] * B[j*NI+k];
// #pragma endscop
}


int main(int argc, char** argv)
{

  /* Initialize array(s). */
  init_array ();

  /* Run kernel. */
  trmm();

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  print_array();

  return 0;
}
