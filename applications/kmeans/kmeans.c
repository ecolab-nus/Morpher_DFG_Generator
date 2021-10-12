# include <math.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>

# include "kmeans.h"

/******************************************************************************/

char ch_cap ( char ch )

/******************************************************************************/
/*
  Purpose:

    CH_CAP capitalizes a single character.

  Discussion:

    This routine should be equivalent to the library "toupper" function.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    19 July 1998

  Author:

    John Burkardt

  Parameters:

    Input, char CH, the character to capitalize.

    Output, char CH_CAP, the capitalized character.
*/
{
  if ( 97 <= ch && ch <= 122 )
  {
    ch = ch - 32;
  }

  return ch;
}
/******************************************************************************/

int ch_eqi ( char ch1, char ch2 )

/******************************************************************************/
/*
  Purpose:

    CH_EQI is TRUE (1) if two characters are equal, disregarding case.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    13 June 2003

  Author:

    John Burkardt

  Parameters:

    Input, char CH1, CH2, the characters to compare.

    Output, int CH_EQI, is TRUE (1) if the two characters are equal,
    disregarding case and FALSE (0) otherwise.
*/
{
  int value;

  if ( 97 <= ch1 && ch1 <= 122 )
  {
    ch1 = ch1 - 32;
  }
  if ( 97 <= ch2 && ch2 <= 122 )
  {
    ch2 = ch2 - 32;
  }
  if ( ch1 == ch2 )
  {
    value = 1;
  }
  else
  {
    value = 0;
  }
  return value;
}
/******************************************************************************/

int ch_to_digit ( char ch )

/******************************************************************************/
/*
  Purpose:

    CH_TO_DIGIT returns the integer value of a base 10 digit.

  Example:

     CH  DIGIT
    ---  -----
    '0'    0
    '1'    1
    ...  ...
    '9'    9
    ' '   -1
    'X'   -1

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    13 June 2003

  Author:

    John Burkardt

  Parameters:

    Input, char CH, the decimal digit, '0' through '9'.

    Output, int CH_TO_DIGIT, the corresponding integer value.  If the
    character was 'illegal', then DIGIT is -1.
*/
{
  int digit;

  if ( '0' <= ch && ch <= '9' )
  {
    digit = ch - '0';
  }
  else
  {
    digit = -1;
  }

  return digit;
}
/******************************************************************************/

double *cluster_energy_compute ( int dim_num, int point_num, int cluster_num,
  double point[], int cluster[], double cluster_center[] )

/******************************************************************************/
/*
  Purpose:

    CLUSTER_ENERGY_COMPUTE computes the energy of the clusters.

  Discussion:

    The cluster energy is defined as the sum of the distance
    squared from each point to its cluster center.  It is the goal
    of the H-means and K-means algorithms to find, for a fixed number
    of clusters, a clustering that minimizes this energy

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    07 October 2011

  Author:

    John Burkardt

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of data points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, double POINT[DIM_NUM*POINT_NUM], the data points.

    Input, int CLUSTER[POINT_NUM], the cluster to which each
    data point belongs.  These values are 0-based.

    Input, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM], the
    centers associated with the minimal energy clustering.

    Output, double CLUSTER_ENERGY_COMPUTE[CLUSTER_NUM], the energy
    associated with each cluster.
*/
{
  double *cluster_energy;
  int i;
  int j;
  int k;
  double point_energy;

  cluster_energy = r8vec_zeros_new ( cluster_num );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[i];
    point_energy = 0.0;
    for ( i = 0; i < dim_num; i++ )
    {
      point_energy = point_energy
        + pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
    }
    cluster_energy[k] = cluster_energy[k] + point_energy;
  }

  return cluster_energy;
}
/******************************************************************************/

double *cluster_initialize_1 ( int dim_num, int point_num, int cluster_num,
  double point[] )

/******************************************************************************/
/*
  Purpose:

    CLUSTER_INITIALIZE_1 initializes the clusters to data points.

  Discussion:

    The cluster centers are simply chosen to be the first data points.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    08 October 2011

  Author:

    John Burkardt

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, double POINT[DIM_NUM*POINT_NUM], the coordinates
    of the points.

    Output, double CLUSTER_INITIALIZE_1[DIM_NUM*CLUSTER_NUM],
    the coordinates of the cluster centers.
*/
{
  double *cluster_center;
  int i;
  int j;

  cluster_center = ( double * ) malloc ( dim_num * cluster_num * sizeof ( double ) );

  for ( j = 0; j < cluster_num; j++ )
  {
    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+j*dim_num] = point[i+j*dim_num];
    }
  }

  return cluster_center;
}
/******************************************************************************/

double *cluster_initialize_2 ( int dim_num, int point_num, int cluster_num,
  double point[], int *seed )

/******************************************************************************/
/*
  Purpose:

    CLUSTER_INITIALIZE_2 initializes the cluster centers to random values.

  Discussion:

    In this case, the hyperbox containing the data is computed.

    Then the cluster centers are chosen uniformly at random within
    this hyperbox.

    Of course, if the data is not smoothly distributed throughout
    the box, many cluster centers will be isolated.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    08 October 2011

  Author:

    John Burkardt

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, double POINT[DIM_NUM*POINT_NUM], the coordinates
    of the points.

    Input/output, int *SEED, a seed for the random
    number generator.

    Output, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM],
    the coordinates of the cluster centers.
*/
{
  double *cluster_center;
  int i;
  int j;
  double *r;
  double *r_max;
  double *r_min;

  cluster_center = ( double * ) malloc ( dim_num * cluster_num * sizeof ( double ) );

  r = ( double * ) malloc ( dim_num * sizeof ( double ) );
  r_min = ( double * ) malloc ( dim_num * sizeof ( double ) );
  r_max = ( double * ) malloc ( dim_num * sizeof ( double ) );

  j = 0;
  for ( i = 0; i < dim_num; i++ )
  {
    r_max[i] = point[i+j*dim_num];
    r_min[i] = point[i+j*dim_num];
  }

  for ( j = 1; j < point_num; j++ )
  {
    for ( i = 0; i < dim_num; i++ )
    {
      r_max[i] = r8_max ( r_max[i], point[i+j*dim_num] );
      r_min[i] = r8_min ( r_min[i], point[i+j*dim_num] );
    }
  }

  for ( j = 0; j < cluster_num; j++ )
  {
    r8vec_uniform_01 ( dim_num, seed, r );
    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+j*dim_num] = ( 1.0 - r[i] ) * r_min[i] + r[i] * r_max[i];
    }
  }
  free ( r );
  free ( r_max );
  free ( r_min );

  return cluster_center;
}
/******************************************************************************/

double *cluster_initialize_3 ( int dim_num, int point_num, int cluster_num,
  double point[], int *seed )

/******************************************************************************/
/*
  Purpose:

    CLUSTER_INITIALIZE_3 initializes the cluster centers to random values.

  Discussion:

    In this case, each point is randomly assigned to a cluster, and
    the cluster centers are then computed as the centroids of the points
    in the cluster.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    08 October 2011

  Author:

    John Burkardt

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, double POINT[DIM_NUM*POINT_NUM], the coordinates
    of the points.

    Input/output, int *SEED, a seed for the random
    number generator.

    Output, double CLUSTER_INITIALIZE_3[DIM_NUM*CLUSTER_NUM],
    the coordinates of the cluster centers.
*/
{
  double *cluster_center;
  int *cluster_population;
  int i;
  int j;
  int k;
/*
  Assign one point to each cluster center.
*/
  cluster_center = ( double * ) malloc ( dim_num * cluster_num * sizeof ( double ) );

  for ( k = 0; k < cluster_num; k++ )
  {
    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+k*dim_num] = point[i+k*dim_num];
    }
  }

  cluster_population = ( int * ) malloc ( cluster_num * sizeof ( int ) );

  for ( k = 0; k < cluster_num; k++ )
  {
    cluster_population[k] = 1;
  }
/*
  The rest of the points get assigned randomly.
*/
  for ( j = cluster_num; j < point_num; j++ )
  {
    k = i4_uniform_ab ( 1, cluster_num, seed );
    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
        + point[i+j*dim_num];
    }
    cluster_population[k] = cluster_population[k] + 1;
  }
/*
  Now average the points to get the centroid.
*/
  for ( k = 0; k < cluster_num; k++ )
  {
    if ( cluster_population[k] != 0 )
    {
      for ( i = 0; i < dim_num; i++ )
      {
        cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num] /
          ( double ) ( cluster_population[k] );
      }
    }
  }

  free ( cluster_population );

  return cluster_center;
}
/******************************************************************************/

double *cluster_initialize_4 ( int dim_num, int point_num, int cluster_num,
  double point[], int *seed )

/******************************************************************************/
/*
  Purpose:

    CLUSTER_INITIALIZE_4 initializes the cluster centers to random values.

  Discussion:

    In this case, each data point is divided randomly among the
    the cluster centers.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    08 October 2011

  Author:

    John Burkardt

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, double POINT[DIM_NUM*POINT_NUM], the coordinates
    of the points.

    Input/output, int *SEED, a seed for the random
    number generator.

    Output, double CLUSTER_INITIALIZE_4[DIM_NUM*CLUSTER_NUM],
    the coordinates of the cluster centers.
*/
{
  double *cluster_center;
  double *cluster_factor;
  double *cluster_weight;
  double divisor;
  int i;
  int j;
  int k;

  cluster_center = r8vec_zeros_new ( dim_num * cluster_num );

  cluster_factor = ( double * ) malloc ( cluster_num * sizeof ( double ) );

  cluster_weight = r8vec_zeros_new ( cluster_num );

  for ( j = 0; j < point_num; j++ )
  {
    r8vec_uniform_01 ( cluster_num, seed, cluster_factor );

    divisor = r8vec_sum ( cluster_num, cluster_factor );

    for ( k = 0; k < cluster_num; k++ )
    {
      cluster_factor[k] = cluster_factor[k] / divisor;
    }

    for ( k = 0; k < cluster_num; k++ )
    {
      for ( i = 0; i < dim_num; i++ )
      {
        cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
          + cluster_factor[k] * point[i+j*dim_num];
      }
    }

    for ( k = 0; k < cluster_num; k++ )
    {
      cluster_weight[k] = cluster_weight[k] + cluster_factor[k];
    }
  }
/*
  Now normalize,  so that each cluster center is now a convex
  combination of the points.
*/
  for ( k = 0; k < cluster_num; k++ )
  {
    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
        / cluster_weight[k];
    }
  }

  free ( cluster_factor );
  free ( cluster_weight );

  return cluster_center;
}
/******************************************************************************/

double *cluster_initialize_5 ( int dim_num, int point_num, int cluster_num,
  double point[], int *seed )

/******************************************************************************/
/*
  Purpose:

    CLUSTER_INITIALIZE_5 initializes the cluster centers to random values.

  Discussion:

    In this case, each cluster center is a random convex combination
    of the data points.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    08 October 2011

  Author:

    John Burkardt

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, double POINT[DIM_NUM*POINT_NUM], the coordinates
    of the points.

    Input/output, int *SEED, a seed for the random
    number generator.

    Output, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM],
    the coordinates of the cluster centers.
*/
{
  double *cluster_center;
  double column_sum;
  double *factor;
  int j;
  int k;
/*
  Get a PxC block of random factors.
*/
  factor = r8mat_uniform_01_new ( point_num, cluster_num, seed );
/*
  Make each column of factors have unit sum.
*/
  for ( k = 0; k < cluster_num; k++ )
  {
    column_sum = 0.0;
    for ( j = 0; j < point_num; j++ )
    {
      column_sum = column_sum + factor[j+k*point_num];
    }
    for ( j = 0; j < point_num; j++ )
    {
      factor[j+k*point_num] = factor[j+k*point_num] / column_sum;
    }
  }
/*
  Set centers = points * factors.
*/
  cluster_center = r8mat_mm_new ( dim_num, point_num, cluster_num, point,
    factor );

  free ( factor );

  return cluster_center;
}
/******************************************************************************/

void cluster_print_summary ( int point_num, int cluster_num,
  int cluster_population[], double cluster_energy[], double cluster_variance[] )

/******************************************************************************/
/*
  Purpose:

    CLUSTER_PRINT_SUMMARY prints a summary of data about a clustering.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    08 October 2011

  Author:

    John Burkardt

  Parameters:

    Input, int POINT_NUM, the number of points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, int CLUSTER_POPULATION[CLUSTER_NUM], the number of
    points assigned to each cluster.

    Input, double CLUSTER_ENERGY[CLUSTER_NUM], the energy of
    the clusters.

    Input, double CLUSTER_VARIANCE[CLUSTER_NUM], the variance of
    the clusters.
*/
{
  double ce;
  int cep;
  double ce_total;
  int cp;
  int cpp;
  double cv;
  int k;

  ce_total = r8vec_sum ( cluster_num, cluster_energy );

  printf ( "\n" );
  printf ( "  Clustering statistics:\n" );
  printf ( "\n" );
  printf ( "    Number of clusters is %d\n", cluster_num );
  printf ( "    Number of points is   %d\n", point_num );
  printf ( "    Total energy is       %g\n", ce_total );
  printf ( "\n" );
  printf ( "    Cluster   Population        Energy          Variance\n" );
  printf ( "    -------  -----------  -----------------  --------------\n" );
  printf ( "                  #          value         \n" );
  printf ( "\n" );

  for ( k = 0; k < cluster_num; k++ )
  {
    cp = cluster_population[k];
    cpp = ( int ) ( ( double ) ( 100 * cp ) / ( double ) ( point_num ) );
    ce = cluster_energy[k];
    cep = ( int ) ( ( ce * 100.0 ) / ce_total );
    cv = cluster_variance[k];
    printf ( "  %7d  %7d  %3d  %12g  %3d  %12g\n", k, cp, cpp, ce, cep, cv );
  }

  cp = i4vec_sum ( cluster_num, cluster_population );
  cpp = 100;
  ce = r8vec_sum ( cluster_num, cluster_energy );
  cep = 100;
  cv = r8vec_i4vec_dot_product ( cluster_num, cluster_variance,
    cluster_population ) / cp;

  printf ( "\n" );
  printf ( "  Total  %7d  %3d  %12g  %3d  %12g\n", cp, cpp, ce, cep, cv );

  return;
}
/******************************************************************************/

double *cluster_variance_compute ( int dim_num, int point_num, int cluster_num,
  double point[], int cluster[], double cluster_center[] )

/******************************************************************************/
/*
  Purpose:

    CLUSTER_VARIANCE_COMPUTE computes the variance of the clusters.

  Discussion:

    The cluster variance (from the cluster center) is the average of the
    sum of the squares of the distances of each point in the cluster to the
    cluster center.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    06 October 2009

  Author:

    John Burkardt

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of data points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, double POINT[DIM_NUM*POINT_NUM], the data points.

    Input, int CLUSTER[POINT_NUM], the cluster to which each
    data point belongs.

    Input, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM], the
    centers associated with the minimal energy clustering.

    Output, double CLUSTER_VARIANCE_COMPUTE[CLUSTER_NUM], the variance
    associated with each cluster.
*/
{
  int *cluster_population;
  double *cluster_variance;
  int i;
  int j;
  int k;
  double point_variance;

  cluster_population = i4vec_zeros_new ( cluster_num );
  cluster_variance = r8vec_zeros_new ( cluster_num );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];

    point_variance = 0.0;
    for ( i = 0; i < dim_num; i++ )
    {
      point_variance = point_variance +
        pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
    }
    cluster_variance[k] = cluster_variance[k] + point_variance;
    cluster_population[k] = cluster_population[k] + 1;
  }

  for ( k = 0; k < cluster_num; k++ )
  {
    if ( 0 < cluster_population[k] )
    {
      cluster_variance[k] = cluster_variance[k] / cluster_population[k];
    }
  }

  free ( cluster_population );

  return cluster_variance;
}
/******************************************************************************/

int file_column_count ( char *input_filename )

/******************************************************************************/
/*
  Purpose:

    FILE_COLUMN_COUNT counts the number of columns in the first line of a file.

  Discussion:

    The file is assumed to be a simple text file.

    Most lines of the file is presumed to consist of COLUMN_NUM words, separated
    by spaces.  There may also be some blank lines, and some comment lines,
    which have a "#" in column 1.

    The routine tries to find the first non-comment non-blank line and
    counts the number of words in that line.

    If all lines are blanks or comments, it goes back and tries to analyze
    a comment line.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    13 June 2003

  Author:

    John Burkardt

  Parameters:

    Input, char *INPUT_FILENAME, the name of the file.

    Output, int FILE_COLUMN_COUNT, the number of columns assumed 
    to be in the file.
*/
{
# define MY_LINE_MAX 256

  int column_num;
  char *error;
  FILE *input;
  int got_one;
  char line[MY_LINE_MAX];
/*
  Open the file.
*/
  input = fopen ( input_filename, "r" );

  if ( !input )
  {
    column_num = -1;
    printf ( "\n" );
    printf ( "FILE_COLUMN_COUNT - Fatal error!\n" );
    printf ( "  Could not open the input file: \"%s\"\n", input_filename );
    return column_num;
  }
/*
  Read one line, but skip blank lines and comment lines.
*/
  got_one = 0;

  for ( ; ; )
  {
    error = fgets ( line, MY_LINE_MAX, input );

    if ( !error )
    {
      break;
    }

    if ( s_len_trim ( line ) == 0 )
    {
      continue;
    }

    if ( line[0] == '#' )
    {
      continue;
    }

    got_one = 1;
    break;

  }

  if ( got_one == 0 )
  {
    fclose ( input );

    input = fopen ( input_filename, "r" );

    for ( ; ; )
    {
      error = fgets ( line, MY_LINE_MAX, input );

      if ( !error )
      {
        break;
      }

      if ( s_len_trim ( line ) == 0 )
      {
        continue;
      }

      got_one = 1;
      break;
    }
  }

  fclose ( input );

  if ( got_one == 0 )
  {
    printf ( "\n" );
    printf ( "FILE_COLUMN_COUNT - Warning!\n" );
    printf ( "  The file does not seem to contain any data.\n" );
    return -1;
  }

  column_num = s_word_count ( line );

  return column_num;

# undef MY_LINE_MAX
}
/******************************************************************************/

int file_row_count ( char *input_filename )

/******************************************************************************/
/*
  Purpose:

    FILE_ROW_COUNT counts the number of row records in a file.

  Discussion:

    It does not count lines that are blank, or that begin with a
    comment symbol '#'.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    13 June 2003

  Author:

    John Burkardt

  Parameters:

    Input, char *INPUT_FILENAME, the name of the input file.

    Output, int FILE_ROW_COUNT, the number of rows found.
*/
{
# define MY_LINE_MAX 256

  int comment_num;
  char *error;
  FILE *input;
  char line[MY_LINE_MAX];
  int record_num;
  int row_num;

  row_num = 0;
  comment_num = 0;
  record_num = 0;

  input = fopen ( input_filename, "r" );

  if ( !input )
  {
    printf ( "\n" );
    printf ( "FILE_ROW_COUNT - Fatal error!\n" );
    printf ( "  Could not open the input file: \"%s\"\n", input_filename );
    return (-1);
  }

  for ( ; ; )
  {
    error = fgets ( line, MY_LINE_MAX, input );

    if ( !error )
    {
      break;
    }

    record_num = record_num + 1;

    if ( line[0] == '#' )
    {
      comment_num = comment_num + 1;
      continue;
    }

    if ( s_len_trim ( line ) == 0 )
    {
      comment_num = comment_num + 1;
      continue;
    }

    row_num = row_num + 1;
  }

  fclose ( input );

  return row_num;

# undef MY_LINE_MAX
}
/******************************************************************************/

void hmeans_01 ( int dim_num, int point_num, int cluster_num, int it_max,
  int *it_num, double point[], int cluster[], double cluster_center[],
  int cluster_population[], double cluster_energy[] )

/******************************************************************************/
/*
  Purpose:

    HMEANS_01 applies the H-Means algorithm.

  Discussion:

    The data for the H-Means problem is a set of N points X in
    M-dimensions, and a desired number of clusters K.

    The goal is to determine K points Z, called cluster centers, so that
    if we associate each point X with its nearest Z value, we minimize
    the standard deviation or cluster energy.  Writing CLUSTER(I) to
    indicate the index of the nearest cluster center to point X(I), the
    energy can be written as:

      Energy = Sum ( 1 <= I <= N ) || X(I) - Z(CLUSTER(I)) ||^2

    where

      || X - Z ||^2 = Sum ( 1 <= J <= M ) ( X(J) - Z(J) )^2

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    08 October 2011

  Author:

    John Burkardt

  Reference:

    Wendy Martinez, Angel Martinez,
    Computational Statistics Handbook with MATLAB,
    pages 373-376,
    Chapman and Hall / CRC, 2002.

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of data points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, int IT_MAX, the maximum number of iterations.

    Output, int *IT_NUM, the number of iterations taken.

    Input, double POINT[DIM_NUM*POINT_NUM], the data points.

    Input/output, int CLUSTER[POINT_NUM].  On input, the user
    may specify an initial cluster for each point, or leave all entrie of
    CLUSTER set to 0.  On output, CLUSTER contains the index of the
    cluster to which each data point belongs.

    Input/output, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM], the
    centers associated with the minimal energy clustering.

    Output, int CLUSTER_POPULATION[CLUSTER_NUM],
    the populuation of each cluster.

    Output, double CLUSTER_ENERGY[CLUSTER_NUM], the energy
    associated with each cluster.
*/
{
  double *centroid;
  int i;
  int j;
  int k;
  int k2;
  int missed;
  double point_energy;
  double point_energy_min;
  int swap;
/*
  Data checks.
*/
  if ( cluster_num < 1 )
  {
    printf ( "\n" );
    printf ( "HMEANS_01 - Fatal error!\n" );
    printf ( "  CLUSTER_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( dim_num < 1 )
  {
    printf ( "\n" );
    printf ( "HMEANS_01 - Fatal error!\n" );
    printf ( "  DIM_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( point_num < 1 )
  {
    printf ( "\n" );
    printf ( "HMEANS_01 - Fatal error!\n" );
    printf ( "  POINT_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( it_max < 0 )
  {
    printf ( "\n" );
    printf ( "HMEANS_01 - Fatal error!\n" );
    printf ( "  IT_MAX < 0.\n" );
    exit ( 1 );
  }
/*
  On input, legal entries in CLUSTER are preserved, but
  otherwise, each point is assigned to its nearest cluster.
*/
  for ( j = 0; j < point_num; j++ )
  {
    if ( cluster[j] < 0 || cluster_num <= cluster[j] )
    {
      point_energy_min = r8_huge ( );

      for ( k = 0; k < cluster_num; k++ )
      {
        point_energy = 0.0;
        for ( i = 0; i < dim_num; i++ )
        {
          point_energy = point_energy +
          pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
        }

        if ( point_energy < point_energy_min )
        {
          point_energy_min = point_energy;
          cluster[j] = k;
        }
      }
    }
  }
  *it_num = 0;

  while ( *it_num < it_max )
  {
    *it_num = *it_num + 1;
/*
  #1:
  Assign each point to the cluster of its nearest center.
*/
    swap = 0;

    for ( j = 0; j < point_num; j++ )
    {
      point_energy_min = r8_huge ( );
      k = cluster[j];

      for ( k2 = 0; k2 < cluster_num; k2++ )
      {
        point_energy = 0.0;
        for ( i = 0; i < dim_num; i++ )
        {
          point_energy = point_energy +
          pow ( point[i+j*dim_num] - cluster_center[i+k2*dim_num], 2 );
        }

        if ( point_energy < point_energy_min )
        {
          point_energy_min = point_energy;
          cluster[j] = k2;
        }
      }

      if ( k != cluster[j] )
      {
        swap = swap + 1;
      }
    }
/*
  Terminate if no points were swapped.
*/
    if ( 1 < *it_num )
    {
      if ( swap == 0 )
      {
        break;
      }
    }
/*
  #2:
  Determine the total energy of the new clustering with current centroids.
*/
    r8vec_zeros ( cluster_num, cluster_energy );

    for ( j = 0; j < point_num; j++ )
    {
      k = cluster[j];

      point_energy = 0.0;
      for ( i = 0; i < dim_num; i++ )
      {
        point_energy = point_energy +
        pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
      }
      cluster_energy[k] = cluster_energy[k] + point_energy;
    }
/*
  #3:
  Determine the centroids of the clusters.
*/
    centroid = r8vec_zeros_new ( dim_num * cluster_num );
    i4vec_zeros ( cluster_num, cluster_population );

    for ( j = 0; j < point_num; j++ )
    {
      k = cluster[j];
      cluster_population[k] = cluster_population[k] + 1;
      for ( i = 0; i < dim_num; i++ )
      {
        centroid[i+k*dim_num] = centroid[i+k*dim_num] + point[i+j*dim_num];
      }
    }
/*
  Now divide by the population to get the centroid.
  But if a center has no population, pick a point at random.
*/
    missed = 0;

    for ( k = 0; k < cluster_num; k++ )
    {
      if ( cluster_population[k] != 0 )
      {
        for ( i = 0; i < dim_num; i++ )
        {
          centroid[i+k*dim_num] = centroid[i+k*dim_num]
          / ( double ) ( cluster_population[k] );
        }
      }
      else
      {
        for ( i = 0; i < dim_num; i++ )
        {
          centroid[i+k*dim_num] = point[i+missed*dim_num];
        }
        missed = missed + 1;
      }
    }

    for ( k = 0; k < cluster_num; k++ )
    {
      for ( i = 0; i < dim_num; i++ )
      {
        cluster_center[i+k*dim_num] = centroid[i+k*dim_num];
      }
    }

    free ( centroid );
/*
  #4:
  Determine the total energy of the current clustering with new centroids.
*/
    r8vec_zeros ( cluster_num, cluster_energy );

    for ( j = 0; j < point_num; j++ )
    {
      k = cluster[j];

      point_energy = 0.0;
      for ( i = 0; i < dim_num; i++ )
      {
        point_energy = point_energy +
        pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
      }
      cluster_energy[k] = cluster_energy[k] + point_energy;
    }
  }
  return;
}
/******************************************************************************/

void hmeans_02 ( int dim_num, int point_num, int cluster_num, int it_max,
  int *it_num, double point[], int cluster[], double cluster_center[],
  int cluster_population[], double cluster_energy[], int *seed )

/******************************************************************************/
/*
  Purpose:

    HMEANS_02 applies the H-Means algorithm.

  Discussion:

    This is a simple routine to group a set of points into K clusters,
    each with a center point, in such a way that the total cluster
    energy is minimized.  The total cluster energy is the sum of the
    squares of the distances of each point to the center of its cluster.

    The algorithm begins with an initial estimate for the cluster centers:

    1. The points are assigned to the nearest cluster centers.

    2. The iteration exit ( 1 );s if the total energy has not changed
        significantly, or we have reached the maximum number of iterations.

    3. Each cluster center is replaced by the centroid of the points
       in the cluster.

    4. Return to step 1.

    The algorithm may fail to find the best solution.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    08 October 2011

  Author:

    John Burkardt

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, int IT_MAX, the maximum number of iterations.

    Output, int *IT_NUM, the number of iterations taken.

    Input, double POINT[DIM_NUM*POINT_NUM], the coordinates
    of the points.

    Input/output, int CLUSTER[POINT_NUM].  On input, the user
    may specify an initial cluster for each point, or leave all entrie of
    CLUSTER set to 0.  On output, CLUSTER contains the index of the
    cluster to which each data point belongs.

    Input/output, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM],
    the coordinates of the cluster centers.

    Output, int CLUSTER_POPULATION[CLUSTER_NUM], the number of
    points assigned to each cluster.

    Output, double CLUSTER_ENERGY[CLUSTER_NUM], the energy of
    the clusters.

    Input/output, int *SEED, a seed for the random
    number generator.
*/
{
  int debug = 0;
  int i;
  int j;
  int k;
  int k2;
  double point_energy;
  double point_energy_min;
  int swap;
/*
  Data checks.
*/
  if ( cluster_num < 1 )
  {
    printf ( "\n" );
    printf ( "HMEANS_02 - Fatal error!\n" );
    printf ( "  CLUSTER_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( dim_num < 1 )
  {
    printf ( "\n" );
    printf ( "HMEANS_02 - Fatal error!\n" );
    printf ( "  DIM_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( point_num < 1 )
  {
    printf ( "\n" );
    printf ( "HMEANS_02 - Fatal error!\n" );
    printf ( "  POINT_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( it_max < 0 )
  {
    printf ( "\n" );
    printf ( "HMEANS_02 - Fatal error!\n" );
    printf ( "  IT_MAX < 0.\n" );
    exit ( 1 );
  }
/*
  On input, legal entries in CLUSTER are preserved, but
  otherwise, each point is assigned to its nearest cluster.
*/
  for ( j = 0; j < point_num; j++ )
  {
    if ( cluster[j] < 0 || cluster_num <= cluster[j] )
    {
      point_energy_min = r8_huge ( );
      for ( k = 0; k < cluster_num; k++ )
      {
        point_energy = 0.0;
        for ( i = 0; i < dim_num; i++ )
        {
          point_energy = point_energy +
            pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
        }
        if ( point_energy < point_energy_min )
        {
          point_energy_min = point_energy;
          cluster[j] = k;
        }
      }
    }
  }

  *it_num = 0;

  for ( ; ; )
  {
/*
  Given centers, assign points to nearest center.
*/
    i4vec_zeros ( cluster_num, cluster_population );
    r8vec_zeros ( cluster_num, cluster_energy );

    swap = 0;

    for ( j = 0; j < point_num; j++ )
    {
      point_energy_min = r8_huge ( );
      k = cluster[j];

      for ( k2 = 0; k2 < cluster_num; k2++ )
      {
        point_energy = 0.0;
        for ( i = 0; i < dim_num; i++ )
        {
          point_energy = point_energy +
            pow ( point[i+j*dim_num] - cluster_center[i+k2*dim_num], 2 );
        }

        if ( point_energy < point_energy_min )
        {
          point_energy_min = point_energy;
          cluster[j] = k2;
        }
      }
      if ( k != cluster[j] )
      {
        swap = swap + 1;
      }
      k = cluster[j];
      cluster_energy[k] = cluster_energy[k] + point_energy_min;
      cluster_population[k] = cluster_population[k] + 1;
    }

    if ( debug )
    {
      printf ( "  %3d  %14g\n", *it_num, r8vec_sum ( cluster_num, cluster_energy ) );
    }

    if ( 0 < *it_num )
    {
      if ( swap == 0 )
      {
        break;
      }
    }

    if ( it_max <= *it_num )
    {
      break;
    }

    *it_num = *it_num + 1;
/*
  Given points in cluster, replace center by centroid.
*/
    r8vec_zeros ( dim_num * cluster_num, cluster_center );

    for ( j = 0; j < point_num; j++ )
    {
      k = cluster[j];
      for ( i = 0; i < dim_num; i++ )
      {
        cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
          + point[i+j*dim_num];
      }
    }

    for ( k = 0; k < cluster_num; k++ )
    {
      if ( cluster_population[k] != 0 )
      {
        for ( i = 0; i < dim_num; i++ )
        {
          cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num] /
            ( double ) ( cluster_population[k] );
        }
      }
      else
      {
        j = i4_uniform_ab ( 0, point_num - 1, seed );
        for ( i = 0; i < dim_num; i++ )
        {
          cluster_center[i+k*dim_num] = point[i+j*dim_num];
        }
      }
    }
  }
/*
  Compute the energy based on the final value of the cluster centers.
*/
  r8vec_zeros ( cluster_num, cluster_energy );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];

    point_energy = 0.0;
    for ( i = 0; i < dim_num; i++ )
    {
      point_energy = point_energy +
      pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
    }
    cluster_energy[k] = cluster_energy[k] + point_energy;
  }
  return;
}
/******************************************************************************/

void hmeans_w_01 ( int dim_num, int point_num, int cluster_num, int it_max,
  int *it_num, double point[], double weight[], int cluster[],
  double cluster_center[], int cluster_population[], double cluster_energy[] )

/******************************************************************************/
/*
  Purpose:

    HMEANS_W_01 applies the weighted H-Means algorithm.

  Discussion:

    The input data for the weight H-Means problem includes:
    * a set of N data points X in M dimensions,
    * a set of N nonnegative weights W,
    * a desired number of clusters K.
    * an initial set of cluster centers Z,
    * an (optional) initial set of cluster assignments.

    The goal is to determine K points Z, called cluster centers, and
    to assign each point X(I) to some cluster Z(J), so that we minimize
    the weighted standard deviation of the distance of each data point
    to the center of its cluster.  Writing J = CLUSTER(I) to
    indicate the index of the nearest cluster center Z(J) to the
    point X(I), the quantity we are trying to minimize is the sum
    of the weighted cluster energies E(J), where:

      E(J) = Sum ( 1 <= I <= N ) W(I) * || X(I) - Z(J) ||^2

    Here, we assume that we are using the Euclidean norm, so that

      || X(I) - Z(J) ||^2 = Sum ( 1 <= K <= M )
        ( X(I)(K) - Z(J)(K) )^2

    In this notation, X(I)(K) is the K-th spatial component of the
    I-th data point.

    Note that this routine should give the same results as HMEANS_01
    in any case in which all the entries of the WEIGHT vector are equal.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    08 October 2011

  Author:

    John Burkardt

  Reference:

    Wendy Martinez, Angel Martinez,
    Computational Statistics Handbook with MATLAB,
    pages 373-376,
    Chapman and Hall / CRC, 2002.

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of data points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, int IT_MAX, the maximum number of iterations.

    Output, int *IT_NUM, the number of iterations taken.

    Input, double POINT[DIM_NUM*POINT_NUM], the data points.

    Input, double WEIGHT[POINT_NUM], the weights
    assigned to the data points.  These must be nonnegative, and
    at least one must be strictly positive.

    Input/output, int CLUSTER[POINT_NUM].  On input, the user
    may specify an initial cluster for each point, or leave all entrie of
    CLUSTER set to 0.  On output, CLUSTER contains the index of the
    cluster to which each data point belongs.

    Input/output, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM], the
    centers associated with the minimal energy clustering.

    Output, double CLUSTER_ENERGY[CLUSTER_NUM], the energy
    associated with each cluster.
*/
{
  double *centroid;
  double *cluster_weight;
  double energy;
  int i;
  int j;
  int k;
  int k2;
  int missed;
  double point_energy;
  double point_energy_min;
  int swap;
/*
  Data checks.
*/
  if ( cluster_num < 1 )
  {
    printf ( "\n" );
    printf ( "HMEANS_W_01 - Fatal error!\n" );
    printf ( "  CLUSTER_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( dim_num < 1 )
  {
    printf ( "\n" );
    printf ( "HMEANS_W_01 - Fatal error!\n" );
    printf ( "  DIM_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( point_num < 1 )
  {
    printf ( "\n" );
    printf ( "HMEANS_W_01 - Fatal error!\n" );
    printf ( "  POINT_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( it_max < 0 )
  {
    printf ( "\n" );
    printf ( "HMEANS_W_01 - Fatal error!\n" );
    printf ( "  IT_MAX < 0.\n" );
    exit ( 1 );
  }

  if ( r8vec_any_negative ( point_num, weight ) )
  {
    printf ( "\n" );
    printf ( "HMEANS_W_01 - Fatal error!\n" );
    printf ( "  Some weight entry is negative.\n" );
    exit ( 1 );
  }

  if ( r8vec_all_nonpositive ( point_num, weight ) )
  {
    printf ( "\n" );
    printf ( "HMEANS_W_01 - Fatal error!\n" );
    printf ( "  No weight entry is positive.\n" );
    exit ( 1 );
  }
/*
  On input, legal entries in CLUSTER are preserved, but
  otherwise, each point is assigned to its nearest cluster.
*/
  for ( j = 0; j < point_num; j++ )
  {
    if ( cluster[j] < 0 || cluster_num <= cluster[j] )
    {
      point_energy_min = r8_huge ( );

      for ( k = 0; k < cluster_num; k++ )
      {
        point_energy = 0.0;
        for ( i = 0; i < dim_num; i++ )
        {
          point_energy = point_energy +
            pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
        }

        if ( point_energy < point_energy_min )
        {
          point_energy_min = point_energy;
          cluster[j] = k;
        }
      }
    }
  }
  *it_num = 0;

  while ( *it_num < it_max )
  {
    *it_num = *it_num + 1;
/*
  #1:
  Reassign points to clusters:
  Assign each point to the cluster whose center is nearest;
  Count the number of points whose cluster assignment is changed.
*/
    swap = 0;

    for ( j = 0; j < point_num; j++ )
    {
      point_energy_min = r8_huge ( );
      k = cluster[j];

      for ( k2 = 0; k2 < cluster_num; k2++ )
      {
        point_energy = 0.0;
        for ( i = 0; i < dim_num; i++ )
        {
          point_energy = point_energy +
            pow ( point[i+j*dim_num] - cluster_center[i+k2*dim_num], 2 );
        }

        if ( point_energy < point_energy_min )
        {
          point_energy_min = point_energy;
          cluster[j] = k2;
        }
      }

      if ( k != cluster[j] )
      {
        swap = swap + 1;
      }
    }
/*
  If no point changed its cluster assignment, the algorithm can make no
  more improvements, so terminate.
*/
    if ( 1 < *it_num )
    {
      if ( swap == 0 )
      {
        break;
      }
    }
/*
  Determine the current energy.
*/
    energy = 0.0;
    for ( j = 0; j < point_num; j++ )
    {
      point_energy = 0.0;
      for ( i = 0; i < dim_num; i++ )
      {
        point_energy = point_energy +
          pow ( point[i+j*dim_num] - cluster_center[i+k2*dim_num], 2 );
      }
      energy = energy + weight[j] * point_energy;
    }
    printf ( "  %4d  %14g\n", *it_num, energy );
/*
  #2:
  Determine the centroids of the clusters, and set the
  cluster center to the cluster centroid.
*/
    centroid = r8vec_zeros_new ( dim_num * cluster_num );
    cluster_weight = r8vec_zeros_new ( cluster_num );

    for ( j = 0; j < point_num; j++ )
    {
      k = cluster[j];
      cluster_population[k] = cluster_population[k] + 1;
      cluster_weight[k] = cluster_weight[k] + weight[j];
      for ( i = 0; i < dim_num; i++ )
      {
        centroid[i+k*dim_num] = centroid[i+k*dim_num]
          + weight[j] * point[i+j*dim_num];
      }
    }

    missed = 0;

    for ( k = 0; k < cluster_num; k++ )
    {
      if ( cluster_weight[k] != 0.0 )
      {
        for ( i = 0; i < dim_num; i++ )
        {
          centroid[i+k*dim_num] = centroid[i+k*dim_num] / cluster_weight[k];
        }
      }
      else
      {
        for ( i = 0; i < dim_num; i++ )
        {
          centroid[i+k*dim_num] = point[i+missed*dim_num];
        }
        missed = missed + 1;
      }
    }

    for ( k = 0; k < cluster_num; k++ )
    {
      for ( i = 0; i < dim_num; i++ )
      {
        cluster_center[i+k*dim_num] = centroid[i+k*dim_num];
      }
    }

    free ( centroid );
    free ( cluster_weight );
  }
/*
  Compute the energy based on the final value of the cluster centers.
*/
  r8vec_zeros ( cluster_num, cluster_energy );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];

    point_energy = 0.0;
    for ( i = 0; i < dim_num; i++ )
    {
      point_energy = point_energy +
      pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
    }
    cluster_energy[k] = cluster_energy[k] + weight[j] * point_energy;
  }
  return;
}
/******************************************************************************/

void hmeans_w_02 ( int dim_num, int point_num, int cluster_num, int it_max,
  int *it_num, double point[], double weight[], int cluster[],
  double cluster_center[], int cluster_population[], double cluster_energy[],
  int *seed )

/******************************************************************************/
/*
  Purpose:

    HMEANS_W_02 applies the weighted H-Means algorithm.

  Discussion:

    The input data for the weight H-Means problem includes:
    * a set of N data points X in M dimensions,
    * a set of N nonnegative weights W,
    * a desired number of clusters K.
    * an initial set of cluster centers Z,
    * an (optional) initial set of cluster assignments.

    The goal is to determine K points Z, called cluster centers, and
    to assign each point X(I) to some cluster Z(J), so that we minimize
    the weighted standard deviation of the distance of each data point
    to the center of its cluster.  Writing J = CLUSTER(I) to
    indicate the index of the nearest cluster center Z(J) to the
    point X(I), the quantity we are trying to minimize is the sum
    of the weighted cluster energies E(J), where:

      E(J) = Sum ( 1 <= I <= N ) W(I) * || X(I) - Z(J) ||^2

    Here, we assume that we are using the Euclidean norm, so that

      || X(I) - Z(J) ||^2 = Sum ( 1 <= K <= M )
        ( X(I)(K) - Z(J)(K) )^2

    In this notation, X(I)(K) is the K-th spatial component of the
    I-th data point.

    Note that this routine should give the same results as HMEANS_02
    in any case in which all the entries of the WEIGHT vector are equal.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    09 October 2011

  Author:

    John Burkardt

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, int IT_MAX, the maximum number of iterations.

    Output, int *IT_NUM, the number of iterations taken.

    Input, double POINT[DIM_NUM*POINT_NUM], the coordinates
    of the points.

    Input, double WEIGHT[POINT_NUM], the weights
    assigned to the data points.  These must be nonnegative, and
    at least one must be strictly positive.

    Input/output, int CLUSTER[POINT_NUM].  On input, the user
    may specify an initial cluster for each point, or leave all entrie of
    CLUSTER set to 0.  On output, CLUSTER contains the index of the
    cluster to which each data point belongs.

    Input/output, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM],
    the coordinates of the cluster centers.

    Output, int CLUSTER_POPULATION[CLUSTER_NUM], the number of
    points assigned to each cluster.

    Output, double CLUSTER_ENERGY[CLUSTER_NUM], the energy of
    the clusters.

    Input/output, int *SEED, a seed for the random
    number generator.
*/
{
  double *cluster_weight;
  int i;
  int j;
  int k;
  int k2;
  double point_energy;
  double point_energy_min;
  int swap;
/*
  Data checks.
*/
  if ( cluster_num < 1 )
  {
    printf ( "\n" );
    printf ( "HMEANS_W_02 - Fatal error!\n" );
    printf ( "  CLUSTER_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( dim_num < 1 )
  {
    printf ( "\n" );
    printf ( "HMEANS_W_02 - Fatal error!\n" );
    printf ( "  DIM_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( point_num < 1 )
  {
    printf ( "\n" );
    printf ( "HMEANS_W_02 - Fatal error!\n" );
    printf ( "  POINT_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( it_max < 0 )
  {
    printf ( "\n" );
    printf ( "HMEANS_W_02 - Fatal error!\n" );
    printf ( "  IT_MAX < 0.\n" );
    exit ( 1 );
  }

  if ( r8vec_any_negative ( point_num, weight ) )
  {
    printf ( "\n" );
    printf ( "HMEANS_W_02 - Fatal error!\n" );
    printf ( "  Some weight entry is negative.\n" );
    exit ( 1 );
  }

  if ( r8vec_all_nonpositive ( point_num, weight ) )
  {
    printf ( "\n" );
    printf ( "HMEANS_W_02 - Fatal error!\n" );
    printf ( "  No weight entry is positive.\n" );
    exit ( 1 );
  }
/*
  On input, legal entries in CLUSTER are preserved, but
  otherwise, each point is assigned to its nearest cluster.
*/
  for ( j = 0; j < point_num; j++ )
  {
    if ( cluster[j] < 0 || cluster_num <= cluster[j] )
    {
      point_energy_min = r8_huge ( );
      for ( k = 0; k < cluster_num; k++ )
      {
        point_energy = 0.0;
        for ( i = 0; i < dim_num; i++ )
        {
          point_energy = point_energy +
            pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
        }
        if ( point_energy < point_energy_min )
        {
          point_energy_min = point_energy;
          cluster[j] = k;
        }
      }
    }
  }
  *it_num = 0;

  for ( ; ; )
  {
/*
  Given centers, assign points to nearest center.
*/
    i4vec_zeros ( cluster_num, cluster_population );
    r8vec_zeros ( cluster_num, cluster_energy );
    cluster_weight = r8vec_zeros_new ( cluster_num );

    swap = 0;

    for ( j = 0; j < point_num; j++ )
    {
      point_energy_min = r8_huge ( );
      k = cluster[j];

      for ( k2 = 0; k2 < cluster_num; k2++ )
      {
        point_energy = 0.0;
        for ( i = 0; i < dim_num; i++ )
        {
          point_energy = point_energy +
            pow ( point[i+j*dim_num] - cluster_center[i+k2*dim_num], 2 );
        }

        if ( point_energy < point_energy_min )
        {
          point_energy_min = point_energy;
          cluster[j] = k2;
        }
      }

      if ( k != cluster[j] )
      {
        swap = swap + 1;
      }
      k = cluster[j];
      cluster_energy[k] = cluster_energy[k] + weight[j] * point_energy_min;
      cluster_population[k] = cluster_population[k] + 1;
      cluster_weight[k] = cluster_weight[k] + weight[j];
    }

    if ( 0 < *it_num )
    {
      if ( swap == 0 )
      {
        break;
      }
    }

    if ( it_max <= *it_num )
    {
      break;
    }

    *it_num = *it_num + 1;
/*
  Given points in cluster, replace center by weighted centroid.
*/
    r8vec_zeros ( dim_num * cluster_num, cluster_center );

    for ( j = 0; j < point_num; j++ )
    {
      k = cluster[j];
      for ( i = 0; i < dim_num; i++ )
      {
        cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
          + weight[j] * point[i+j*dim_num];
      }
    }

    for ( k = 0; k < cluster_num; k++ )
    {
      if ( cluster_weight[k] != 0.0 )
      {
        for ( i = 0; i < dim_num; i++ )
        {
          cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num] /
            cluster_weight[k];
        }
      }
      else
      {
        j = i4_uniform_ab ( 0, point_num - 1, seed );
        for ( i = 0; i < dim_num; i++ )
        {
          cluster_center[i+k*dim_num] = point[i+j*dim_num];
        }
      }
    }
    free ( cluster_weight );
  }
/*
  Compute the energy based on the final value of the cluster centers.
*/
  r8vec_zeros ( cluster_num, cluster_energy );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];

    point_energy = 0.0;
    for ( i = 0; i < dim_num; i++ )
    {
      point_energy = point_energy +
        pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
    }
    cluster_energy[k] = cluster_energy[k] + weight[j] * point_energy;
  }

  return;
}
/******************************************************************************/

int i4_max ( int i1, int i2 )

/******************************************************************************/
/*
  Purpose:

    I4_MAX returns the maximum of two I4's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    29 August 2006

  Author:

    John Burkardt

  Parameters:

    Input, int I1, I2, are two integers to be compared.

    Output, int I4_MAX, the larger of I1 and I2.
*/
{
  int value;

  if ( i2 < i1 )
  {
    value = i1;
  }
  else
  {
    value = i2;
  }
  return value;
}
/******************************************************************************/

int i4_min ( int i1, int i2 )

/******************************************************************************/
/*
  Purpose:

    I4_MIN returns the smaller of two I4's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    29 August 2006

  Author:

    John Burkardt

  Parameters:

    Input, int I1, I2, two integers to be compared.

    Output, int I4_MIN, the smaller of I1 and I2.
*/
{
  int value;

  if ( i1 < i2 )
  {
    value = i1;
  }
  else
  {
    value = i2;
  }
  return value;
}
/******************************************************************************/

int i4_uniform_ab ( int a, int b, int *seed )

/******************************************************************************/
/*
  Purpose:

    I4_UNIFORM_AB returns a scaled pseudorandom I4 between A and B.

  Discussion:

    The pseudorandom number should be uniformly distributed
    between A and B.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    24 May 2012

  Author:

    John Burkardt

  Reference:

    Paul Bratley, Bennett Fox, Linus Schrage,
    A Guide to Simulation,
    Second Edition,
    Springer, 1987,
    ISBN: 0387964673,
    LC: QA76.9.C65.B73.

    Bennett Fox,
    Algorithm 647:
    Implementation and Relative Efficiency of Quasirandom
    Sequence Generators,
    ACM Transactions on Mathematical Software,
    Volume 12, Number 4, December 1986, pages 362-376.

    Pierre L'Ecuyer,
    Random Number Generation,
    in Handbook of Simulation,
    edited by Jerry Banks,
    Wiley, 1998,
    ISBN: 0471134031,
    LC: T57.62.H37.

    Peter Lewis, Allen Goodman, James Miller,
    A Pseudo-Random Number Generator for the System/360,
    IBM Systems Journal,
    Volume 8, Number 2, 1969, pages 136-143.

  Parameters:

    Input, int A, B, the limits of the interval.

    Input/output, int *SEED, the "seed" value, which should NOT be 0.
    On output, SEED has been updated.

    Output, int I4_UNIFORM_AB, a number between A and B.
*/
{
  int c;
  const int i4_huge = 2147483647;
  int k;
  float r;
  int value;

  if ( *seed == 0 )
  {
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "I4_UNIFORM_AB - Fatal error!\n" );
    fprintf ( stderr, "  Input value of SEED = 0.\n" );
    exit ( 1 );
  }
/*
  Guaranteee A <= B.
*/
  if ( b < a )
  {
    c = a;
    a = b;
    b = c;
  }

  k = *seed / 127773;

  *seed = 16807 * ( *seed - k * 127773 ) - k * 2836;

  if ( *seed < 0 )
  {
    *seed = *seed + i4_huge;
  }

  r = ( float ) ( *seed ) * 4.656612875E-10;
/*
  Scale R to lie between A-0.5 and B+0.5.
*/
  r = ( 1.0 - r ) * ( ( float ) ( a ) - 0.5 ) 
    +         r   * ( ( float ) ( b ) + 0.5 );
/*
  Round R to the nearest integer.
*/
  value = round ( r );
/*
  Guarantee that A <= VALUE <= B.
*/
  if ( value < a )
  {
    value = a;
  }
  if ( b < value )
  {
    value = b;
  }

  return value;
}
/******************************************************************************/

void i4mat_write ( char *output_filename, int m, int n, int table[] )

/******************************************************************************/
/*
  Purpose:

    I4MAT_WRITE writes an I4MAT file.

  Discussion:

    An I4MAT is an array of I4's.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    01 June 2009

  Author:

    John Burkardt

  Parameters:

    Input, char *OUTPUT_FILENAME, the output filename.

    Input, int M, the spatial dimension.

    Input, int N, the number of points.

    Input, int TABLE[M*N], the data.
*/
{
  int i;
  int j;
  FILE *output;
/*
  Open the file.
*/
  output = fopen ( output_filename, "wt" );

  if ( !output )
  {
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "I4MAT_WRITE - Fatal error!\n" );
    fprintf ( stderr, "  Could not open the output file '%s'\n", output_filename );
    exit ( 1 );
  }
/*
  Write the data.
*/
  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      fprintf ( output, "  %d", table[i+j*m] );
    }
    fprintf ( output, "\n" );
  }
/*
  Close the file.
*/
  fclose ( output );

  return;
}
/******************************************************************************/

void i4vec_negone ( int n, int a[] )

/******************************************************************************/
/*
  Purpose:

    I4VEC_NEGONE sets an I4VEC to -1.

  Discussion:

    An I4VEC is a vector of I4's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    12 October 2011

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vector.

    Output, int A[N], a vector of -1's.
*/
{
  int i;

  for ( i = 0; i < n; i++ )
  {
    a[i] = -1;
  }
  return;
}
/******************************************************************************/

int *i4vec_negone_new ( int n )

/******************************************************************************/
/*
  Purpose:

    I4VEC_NEGONE_NEW creates an I4VEC and sets it to -1.

  Discussion:

    An I4VEC is a vector of I4's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    12 October 2011

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vector.

    Output, int I4VEC_NEGONE_NEW[N], a vector of -1's.
*/
{
  int *a;
  int i;

  a = ( int * ) malloc ( n * sizeof ( int ) );

  for ( i = 0; i < n; i++ )
  {
    a[i] = -1;
  }
  return a;
}
/******************************************************************************/

int i4vec_sum ( int n, int a[] )

/******************************************************************************/
/*
  Purpose:

    I4VEC_SUM sums the entries of an I4VEC.

  Discussion:

    An I4VEC is a vector of I4's.

  Example:

    Input:

      A = ( 1, 2, 3, 4 )

    Output:

      I4VEC_SUM = 10

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    29 May 2003

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vector.

    Input, int A[N], the vector to be summed.

    Output, int I4VEC_SUM, the sum of the entries of A.
*/
{
  int i;
  int sum;

  sum = 0;
  for ( i = 0; i < n; i++ )
  {
    sum = sum + a[i];
  }

  return sum;
}
/******************************************************************************/

void i4vec_zeros ( int n, int a[] )

/******************************************************************************/
/*
  Purpose:

    I4VEC_ZEROS zeroes an I4VEC.

  Discussion:

    An I4VEC is a vector of I4's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    05 September 2008

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vector.

    Output, int A[N], a vector of zeroes.
*/
{
  int i;

  for ( i = 0; i < n; i++ )
  {
    a[i] = 0;
  }
  return;
}
/******************************************************************************/

int *i4vec_zeros_new ( int n )

/******************************************************************************/
/*
  Purpose:

    I4VEC_ZEROS_NEW creates and zeroes an I4VEC.

  Discussion:

    An I4VEC is a vector of I4's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    05 September 2008

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vector.

    Output, int I4VEC_ZEROS_NEW[N], a vector of zeroes.
*/
{
  int *a;
  int i;

  a = ( int * ) malloc ( n * sizeof ( int ) );

  for ( i = 0; i < n; i++ )
  {
    a[i] = 0;
  }
  return a;
}
/******************************************************************************/

void kmeans_01 ( int dim_num, int point_num, int cluster_num, int it_max,
  int *it_num, double point[], int cluster[], double cluster_center[],
  int cluster_population[], double cluster_energy[] )

/******************************************************************************/
/*
  Purpose:

    KMEANS_01 applies the K-Means algorithm.

  Discussion:

    Given a matrix of POINT_NUM observations on DIM_NUM variables, the
    observations are to be allocated to CLUSTER_NUM clusters in such
    a way that the within-cluster sum of squares is minimized.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    09 October 2011

  Author:

    Original FORTRAN77 version by David Sparks.
    C++ version by John Burkardt.

  Reference:

    David Sparks,
    Algorithm AS 58:
    Euclidean Cluster Analysis,
    Applied Statistics,
    Volume 22, Number 1, 1973, pages 126-130.

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, int IT_MAX, the maximum number of iterations.

    Output, int *IT_NUM, the number of iterations taken.

    Input, double POINT[DIM_NUM*POINT_NUM], the points.

    Output, int CLUSTER[POINT_NUM], indicates which cluster
    each point belongs to.

    Input/output, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM],
    the cluster centers.

    Output, int CLUSTER_POPULATION[CLUSTER_NUM], the number
    of points in each cluster.

    Output, double CLUSTER_ENERGY[CLUSTER_NUM], the
    cluster energies.
*/
{
  double dc;
  double de;
  double *f;
  int i;
  int il;
  int ir;
  int j;
  int j2;
  int k;
  double point_energy;
  double point_energy_min;
  int swap;

  *it_num = 0;
/*
  Idiot checks.
*/
  if ( cluster_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_01 - Fatal error!\n" );
    printf ( "  CLUSTER_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( dim_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_01 - Fatal error!\n" );
    printf ( "  DIM_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( point_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_01 - Fatal error!\n" );
    printf ( "  POINT_NUM < 1.\n" );
    exit ( 1 );
  }
/*
  For each observation, calculate the distance from each cluster
  center, and assign to the nearest.
*/
  for ( j = 0; j < point_num; j++ )
  {
    point_energy_min = r8_huge ( );
    cluster[j] = -1;

    for ( k = 0; k < cluster_num; k++ )
    {
      point_energy = 0.0;
      for ( i = 0; i < dim_num; i++ )
      {
        point_energy = point_energy +
          pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
      }

      if ( point_energy < point_energy_min )
      {
        point_energy_min = point_energy;
        cluster[j] = k;
      }
    }
  }
/*
  Determine the cluster population counts.
*/
  i4vec_zeros ( cluster_num, cluster_population );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    cluster_population[k] = cluster_population[k] + 1;
  }
/*
  Calculate the mean and sum of squares for each cluster.
*/
  r8vec_zeros ( dim_num * cluster_num, cluster_center );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
        + point[i+j*dim_num];
    }
  }

  for ( k = 0; k < cluster_num; k++ )
  {
    if ( 0 < cluster_population[k] )
    {
      for ( i = 0; i < dim_num; i++ )
      {
        cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num] /
         ( double ) cluster_population[k];
      }
    }
  }
/*
  Set the point energies.
*/
  f = r8vec_zeros_new ( point_num );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    for ( i = 0; i < dim_num; i=i+40 )
    {
    #ifdef CGRA_COMPILER
           please_map_me();
           #endif
      //f[j] = f[j] + pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
       f[j] = f[j] + (point[i+j*dim_num] - cluster_center[i+k*dim_num])*(point[i+j*dim_num] - cluster_center[i+k*dim_num]);
       f[j] = f[j] + (point[i+1+j*dim_num] - cluster_center[i+1+k*dim_num])*(point[i+1+j*dim_num] - cluster_center[i+1+k*dim_num]);
       f[j] = f[j] + (point[i+2+j*dim_num] - cluster_center[i+2+k*dim_num])*(point[i+2+j*dim_num] - cluster_center[i+2+k*dim_num]);
       f[j] = f[j] + (point[i+3+j*dim_num] - cluster_center[i+3+k*dim_num])*(point[i+3+j*dim_num] - cluster_center[i+3+k*dim_num]);
       f[j] = f[j] + (point[i+4+j*dim_num] - cluster_center[i+4+k*dim_num])*(point[i+4+j*dim_num] - cluster_center[i+4+k*dim_num]);
       f[j] = f[j] + (point[i+5+j*dim_num] - cluster_center[i+5+k*dim_num])*(point[i+5+j*dim_num] - cluster_center[i+5+k*dim_num]);
       f[j] = f[j] + (point[i+6+j*dim_num] - cluster_center[i+6+k*dim_num])*(point[i+6+j*dim_num] - cluster_center[i+6+k*dim_num]);
       f[j] = f[j] + (point[i+7+j*dim_num] - cluster_center[i+7+k*dim_num])*(point[i+7+j*dim_num] - cluster_center[i+7+k*dim_num]);
       f[j] = f[j] + (point[i+8+j*dim_num] - cluster_center[i+8+k*dim_num])*(point[i+8+j*dim_num] - cluster_center[i+8+k*dim_num]);
       f[j] = f[j] + (point[i+9+j*dim_num] - cluster_center[i+9+k*dim_num])*(point[i+9+j*dim_num] - cluster_center[i+9+k*dim_num]);
       f[j] = f[j] + (point[i+10+j*dim_num] - cluster_center[i+10+k*dim_num])*(point[i+10+j*dim_num] - cluster_center[i+10+k*dim_num]);
       f[j] = f[j] + (point[i+11+j*dim_num] - cluster_center[i+11+k*dim_num])*(point[i+11+j*dim_num] - cluster_center[i+11+k*dim_num]);
       f[j] = f[j] + (point[i+12+j*dim_num] - cluster_center[i+12+k*dim_num])*(point[i+12+j*dim_num] - cluster_center[i+12+k*dim_num]);
       f[j] = f[j] + (point[i+13+j*dim_num] - cluster_center[i+13+k*dim_num])*(point[i+13+j*dim_num] - cluster_center[i+13+k*dim_num]);
       f[j] = f[j] + (point[i+14+j*dim_num] - cluster_center[i+14+k*dim_num])*(point[i+14+j*dim_num] - cluster_center[i+14+k*dim_num]);
       f[j] = f[j] + (point[i+15+j*dim_num] - cluster_center[i+15+k*dim_num])*(point[i+15+j*dim_num] - cluster_center[i+15+k*dim_num]);
       f[j] = f[j] + (point[i+16+j*dim_num] - cluster_center[i+16+k*dim_num])*(point[i+16+j*dim_num] - cluster_center[i+16+k*dim_num]);
       f[j] = f[j] + (point[i+17+j*dim_num] - cluster_center[i+17+k*dim_num])*(point[i+17+j*dim_num] - cluster_center[i+17+k*dim_num]);
       f[j] = f[j] + (point[i+18+j*dim_num] - cluster_center[i+18+k*dim_num])*(point[i+18+j*dim_num] - cluster_center[i+18+k*dim_num]);
       f[j] = f[j] + (point[i+19+j*dim_num] - cluster_center[i+19+k*dim_num])*(point[i+19+j*dim_num] - cluster_center[i+19+k*dim_num]);
       f[j] = f[j] + (point[i+20+j*dim_num] - cluster_center[i+20+k*dim_num])*(point[i+20+j*dim_num] - cluster_center[i+20+k*dim_num]);
       f[j] = f[j] + (point[i+21+j*dim_num] - cluster_center[i+21+k*dim_num])*(point[i+21+j*dim_num] - cluster_center[i+21+k*dim_num]);
       f[j] = f[j] + (point[i+22+j*dim_num] - cluster_center[i+22+k*dim_num])*(point[i+22+j*dim_num] - cluster_center[i+22+k*dim_num]);
       f[j] = f[j] + (point[i+23+j*dim_num] - cluster_center[i+23+k*dim_num])*(point[i+23+j*dim_num] - cluster_center[i+23+k*dim_num]);
       f[j] = f[j] + (point[i+24+j*dim_num] - cluster_center[i+24+k*dim_num])*(point[i+24+j*dim_num] - cluster_center[i+24+k*dim_num]);
       f[j] = f[j] + (point[i+25+j*dim_num] - cluster_center[i+25+k*dim_num])*(point[i+25+j*dim_num] - cluster_center[i+25+k*dim_num]);
       f[j] = f[j] + (point[i+26+j*dim_num] - cluster_center[i+26+k*dim_num])*(point[i+26+j*dim_num] - cluster_center[i+26+k*dim_num]);
       f[j] = f[j] + (point[i+27+j*dim_num] - cluster_center[i+27+k*dim_num])*(point[i+27+j*dim_num] - cluster_center[i+27+k*dim_num]);
       f[j] = f[j] + (point[i+28+j*dim_num] - cluster_center[i+28+k*dim_num])*(point[i+28+j*dim_num] - cluster_center[i+28+k*dim_num]);
       f[j] = f[j] + (point[i+29+j*dim_num] - cluster_center[i+29+k*dim_num])*(point[i+29+j*dim_num] - cluster_center[i+29+k*dim_num]);
       f[j] = f[j] + (point[i+30+j*dim_num] - cluster_center[i+30+k*dim_num])*(point[i+30+j*dim_num] - cluster_center[i+30+k*dim_num]);
       f[j] = f[j] + (point[i+31+j*dim_num] - cluster_center[i+31+k*dim_num])*(point[i+31+j*dim_num] - cluster_center[i+31+k*dim_num]);
       f[j] = f[j] + (point[i+32+j*dim_num] - cluster_center[i+32+k*dim_num])*(point[i+32+j*dim_num] - cluster_center[i+32+k*dim_num]);
       f[j] = f[j] + (point[i+33+j*dim_num] - cluster_center[i+33+k*dim_num])*(point[i+33+j*dim_num] - cluster_center[i+33+k*dim_num]);
       f[j] = f[j] + (point[i+34+j*dim_num] - cluster_center[i+34+k*dim_num])*(point[i+34+j*dim_num] - cluster_center[i+34+k*dim_num]);
       f[j] = f[j] + (point[i+35+j*dim_num] - cluster_center[i+35+k*dim_num])*(point[i+35+j*dim_num] - cluster_center[i+35+k*dim_num]);
       f[j] = f[j] + (point[i+36+j*dim_num] - cluster_center[i+36+k*dim_num])*(point[i+36+j*dim_num] - cluster_center[i+36+k*dim_num]);
       f[j] = f[j] + (point[i+37+j*dim_num] - cluster_center[i+37+k*dim_num])*(point[i+37+j*dim_num] - cluster_center[i+37+k*dim_num]);
       f[j] = f[j] + (point[i+38+j*dim_num] - cluster_center[i+38+k*dim_num])*(point[i+38+j*dim_num] - cluster_center[i+38+k*dim_num]);
       f[j] = f[j] + (point[i+39+j*dim_num] - cluster_center[i+39+k*dim_num])*(point[i+39+j*dim_num] - cluster_center[i+39+k*dim_num]);









    }
  }
/*
  Set the cluster energies.
*/
  r8vec_zeros ( cluster_num, cluster_energy );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    cluster_energy[k] = cluster_energy[k] + f[j];
  }
/*
  Adjust the point energies by a weight factor.
*/
  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    if ( 1 < cluster_population[k] )
    {
      f[j] = f[j] * ( double ) ( cluster_population[k] )
        / ( double ) ( cluster_population[k] - 1 );
    }
  }
/*
  Examine each observation in turn to see if it should be
  reassigned to a different cluster.
*/
  *it_num = 0;

  while ( *it_num < it_max )
  {
    *it_num = *it_num + 1;

    swap = 0;

    for ( j = 0; j < point_num; j++ )
    {
      il = cluster[j];
      ir = il;

      if ( cluster_population[il] <= 1 )
      {
        continue;
      }

      dc = f[j];

      for ( k = 0; k < cluster_num; k++ )
      {
        if ( k != il )
        {
          de = 0.0;
          for ( i = 0; i < dim_num; i++ )
          {
            de = de + pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
          }
          de = de * ( double ) cluster_population[k]
             / ( double ) ( cluster_population[k] + 1 );

          if ( de < dc )
          {
            dc = de;
            ir = k;
          }
        }
      }
/*
  If the lowest value was obtained by staying in the current cluster,
  then cycle.
*/
      if ( ir == il )
      {
        continue;
      }
/*
  Reassign the point from cluster IL to cluster IR.
*/
      for ( i = 0; i < dim_num; i++ )
      {
        cluster_center[i+il*dim_num] = ( cluster_center[i+il*dim_num]
          * ( double ) ( cluster_population[il] ) - point[i+j*dim_num] )
          / ( double ) ( cluster_population[il] - 1 );

        cluster_center[i+ir*dim_num] = ( cluster_center[i+ir*dim_num]
          * ( double ) ( cluster_population[ir] ) + point[i+j*dim_num] )
          / ( double ) ( cluster_population[ir] + 1 );
      }
      cluster_energy[il] = cluster_energy[il] - f[j];
      cluster_energy[ir] = cluster_energy[ir] + dc;
      cluster_population[ir] = cluster_population[ir] + 1;
      cluster_population[il] = cluster_population[il] - 1;

      cluster[j] = ir;
/*
  Adjust the value of F for points in clusters IL and IR.
*/
      for ( j2 = 0; j2 < point_num; j2++ )
      {
        k = cluster[j2];

        if ( k == il || k == ir )
        {
          f[j2] = 0.0;
          for ( i = 0; i < dim_num; i++ )
          {
            f[j2] = f[j2] + pow ( point[i+j2*dim_num] - cluster_center[i+k*dim_num], 2 );
          }

          if ( 1 < cluster_population[k] )
          {
            f[j2] = f[j2] * ( double ) ( cluster_population[k] )
              / ( ( double ) ( cluster_population[k] - 1 ) );
          }
        }
      }
      swap = swap + 1;
    }
/*
  Exit if no reassignments were made during this iteration.
*/
    if ( swap == 0 )
    {
      break;
    }
  }
/*
  Compute the cluster energies.
*/
  r8vec_zeros ( cluster_num, cluster_energy );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];

    point_energy = 0.0;
    for ( i = 0; i < dim_num; i++ )
    {
      point_energy = point_energy +
        pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
    }
    cluster_energy[k] = cluster_energy[k] + point_energy;
  }

  free ( f );

  return;
}
/******************************************************************************/

void kmeans_02 ( int dim_num, int point_num, int cluster_num, int it_max,
  int *it_num, double point[], int cluster[], double cluster_center[],
  int cluster_population[], double cluster_energy[] )

/******************************************************************************/
/*
  Purpose:

    KMEANS_02 applies the K-Means algorithm.

  Discussion:

    The routine attempts to divide POINT_NUM points in
    DIM_NUM-dimensional space into CLUSTER_NUM clusters so that the within
    cluster sum of squares is minimized.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    13 October 2011

  Author:

    Original FORTRAN77 by John Hartigan, Manchek Wong.
    C++ version by John Burkardt.

  Reference:

    John Hartigan, Manchek Wong,
    Algorithm AS 136:
    A K-Means Clustering Algorithm,
    Applied Statistics,
    Volume 28, Number 1, 1979, pages 100-108.

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, int IT_MAX, the maximum number of iterations.

    Output, int *IT_NUM, the number of iterations taken.

    Input, double POINT[DIM_NUM*POINT_NUM], the coordinates
    of the points.

    Output, int CLUSTER[POINT_NUM], the cluster each
    point belongs to.

    Input/output, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM],
    the cluster centers.

    Output, int CLUSTER_POPULATION[CLUSTER_NUM], the number
    of points in each cluster.

    Output, double CLUSTER_ENERGY[CLUSTER_NUM], the
    within-cluster sum of squares.
*/
{
  double *an1;
  double *an2;
  int *cluster2;
  double *d;
  double db;
  double dt[2];
  int i;
  int ifault;
  int il;
  int indx;
  int *itran;
  int j;
  int k;
  int *live;
  int *ncp;
  double point_energy;
  double temp;

  *it_num = 0;

  if ( cluster_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_02 - Fatal error!\n" );
    printf ( "  CLUSTER_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( point_num <= cluster_num )
  {
    printf ( "\n" );
    printf ( "KMEANS_02 - Fatal error!\n" );
    printf ( "  POINT_NUM <= CLUSTER_NUM.\n" );
    exit ( 1 );
  }

  if ( dim_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_02 - Fatal error!\n" );
    printf ( "  DIM_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( point_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_02 - Fatal error!\n" );
    printf ( "  POINT_NUM < 1.\n" );
    exit ( 1 );
  }
/*
  For each point I, find its two closest centers, CLUSTER(I) and CLUSTER2(I).
  Assign it to CLUSTER(I).
*/
  cluster2 = ( int * ) malloc ( point_num * sizeof ( int ) );

  for ( j = 0; j < point_num; j++ )
  {
    cluster[j] = 0;
    cluster2[j] = 1;

    for ( il = 0; il < 2; il++ )
    {
      dt[il] = 0.0;
      for ( i = 0; i < dim_num; i++ )
      {
        dt[il] = dt[il] + pow ( point[i+j*dim_num] - cluster_center[i+il*dim_num], 2 );
      }
    }

    if ( dt[1] < dt[0] )
    {
      cluster[j] = 1;
      cluster2[j] = 0;
      temp = dt[0];
      dt[0] = dt[1];
      dt[1] = temp;
    }

    for ( k = 2; k < cluster_num; k++ )
    {
      db = 0.0;
      for ( i = 0; i < dim_num; i++ )
      {
        db = db + pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
      }

      if ( db < dt[0] )
      {
        dt[1] = dt[0];
        cluster2[j] = cluster[j];
        dt[0] = db;
        cluster[j] = k;
      }
      else if ( db < dt[1] )
      {
        dt[1] = db;
        cluster2[j] = k;
      }
    }
  }
/*
  Update cluster centers to be the average of points contained
  within them.
*/
  i4vec_zeros ( cluster_num, cluster_population );
  r8vec_zeros ( dim_num * cluster_num, cluster_center );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    cluster_population[k] = cluster_population[k] + 1;
    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
        + point[i+j*dim_num];
    }
  }
/*
  Check to see if there is any empty cluster.
*/
  an1 = ( double * ) malloc ( cluster_num * sizeof ( double ) );
  an2 = ( double * ) malloc ( cluster_num * sizeof ( double ) );
  itran = ( int * ) malloc ( cluster_num * sizeof ( int ) );
  ncp = ( int * ) malloc ( cluster_num * sizeof ( int ) );

  for ( k = 0; k < cluster_num; k++ )
  {
    if ( cluster_population[k] == 0 )
    {
      printf ( "\n" );
      printf ( "KMEANS_02 - Fatal error!\n" );
      printf ( "  Cluster %d has zero population.\n", k );
      exit ( 1 );
    }

    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
      / ( double ) ( cluster_population[k] );
    }
/*
  Initialize AN1, AN2, ITRAN and NCP
  AN1(K) = CLUSTER_POPULATION(K) / (CLUSTER_POPULATION(K) - 1)
  AN2(K) = CLUSTER_POPULATION(K) / (CLUSTER_POPULATION(K) + 1)
  ITRAN(K) = 1 if cluster K is updated in the quick-transfer stage,
           = 0 otherwise
  In the optimal-transfer stage, NCP(K) stores the step at which
  cluster K is last updated.
  In the quick-transfer stage, NCP(K) stores the step at which
  cluster K is last updated plus POINT_NUM.
*/
    an2[k] = ( double ) ( cluster_population[k] )
      / ( double ) ( cluster_population[k] + 1 );

    if ( 1 < cluster_population[k] )
    {
      an1[k] = ( double ) ( cluster_population[k] )
        / ( double ) ( cluster_population[k] - 1 );
    }
    else
    {
      an1[k] = r8_huge ( );
    }
    itran[k] = 1;
    ncp[k] = -1;
  }

  indx = 0;
  ifault = 2;
  *it_num = 0;

  d =  ( double * ) malloc ( point_num * sizeof ( double ) );;
  live = ( int * ) malloc ( cluster_num * sizeof ( int ) );

  while ( *it_num < it_max )
  {
    *it_num = *it_num + 1;
/*
  In this stage, there is only one pass through the data.   Each
  point is re-allocated, if necessary, to the cluster that will
  induce the maximum reduction in within-cluster sum of squares.
*/
    kmeans_02_optra ( dim_num, point_num, cluster_num, point,
      cluster_center, cluster, cluster2, cluster_population, an1, an2,
      ncp, d, itran, live, &indx );
/*
  Stop if no transfer took place in the last POINT_NUM optimal transfer steps.
*/
    if ( indx == point_num )
    {
      ifault = 0;
      break;
    }
/*
  Each point is tested in turn to see if it should be re-allocated
  to the cluster to which it is most likely to be transferred,
  CLUSTER2(I), from its present cluster, CLUSTER(I).   Loop through the
  data until no further change is to take place.
*/
    kmeans_02_qtran ( dim_num, point_num, cluster_num, point,
      cluster_center, cluster, cluster2, cluster_population, an1, an2,
      ncp, d, itran, &indx );
/*
  If there are only two clusters, there is no need to re-enter the
  optimal transfer stage.
*/
    if ( cluster_num == 2 )
    {
      ifault = 0;
      break;
    }
/*
  NCP has to be set to 0 before entering OPTRA.
*/
    i4vec_zeros ( cluster_num, ncp );
  }
  if ( ifault == 2 )
  {
    printf ( "\n" );
    printf ( "KMEANS_02 - Warning!\n" );
    printf ( "  Maximum number of iterations reached\n" );
    printf ( "  without convergence.\n" );
  }
/*
  Compute the within-cluster sum of squares for each cluster.
*/
  r8vec_zeros ( dim_num * cluster_num, cluster_center );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
        + point[i+j*dim_num];
    }
  }

  for ( k = 0; k < cluster_num; k++ )
  {
    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
        / ( double ) cluster_population[k];
    }
  }
/*
  Compute the cluster energies.
*/
  r8vec_zeros ( cluster_num, cluster_energy );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];

    point_energy = 0.0;
    for ( i = 0; i < dim_num; i++ )
    {
      point_energy = point_energy +
        pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
    }
    cluster_energy[k] = cluster_energy[k] + point_energy;
  }

  free ( an1 );
  free ( an2 );
  free ( cluster2 );
  free ( d );
  free ( itran );
  free ( live );
  free ( ncp );

  return;
}
/******************************************************************************/

void kmeans_02_optra ( int dim_num, int point_num, int cluster_num,
  double point[], double cluster_center[], int cluster[], int cluster2[],
  int cluster_population[], double an1[], double an2[], int ncp[],
  double d[], int itran[], int live[], int *indx )

/******************************************************************************/
/*
  Purpose:

    KMEANS_02_OPTRA carries out the optimal transfer stage.

  Discussion:

    Each point is re-allocated, if necessary, to the cluster that
    will induce a maximum reduction in the within-cluster sum of
    squares.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    09 October 2011

  Author:

    Original FORTRAN77 by John Hartigan, Manchek Wong.
    C++ version by John Burkardt.

  Reference:

    John Hartigan, Manchek Wong,
    Algorithm AS 136:
    A K-Means Clustering Algorithm,
    Applied Statistics,
    Volume 28, Number 1, 1979, pages 100-108.

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, double POINT[DIM_NUM*POINT_NUM], the coordinates of
    the points.

    Input/output, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM],
    the cluster centers.

    Input/output, int CLUSTER[POINT_NUM], the cluster
    each point belongs to.

    Input/output, int CLUSTER2[POINT_NUM], the cluster
    to which each point is most likely to be transferred to.

    Input/output, int CLUSTER_POPULATION[CLUSTER_NUM],
    the number of points in each cluster.

    Input/output, double AN1[CLUSTER_NUM],
    CLUSTER_POPULATION(L) / (CLUSTER_POPULATION(L) - 1)

    Input/output, double AN2[CLUSTER_NUM],
    CLUSTER_POPULATION(L) / (CLUSTER_POPULATION(L) + 1)

    Input/output, int NCP[CLUSTER_NUM], ?

    Input/output, double D[POINT_NUM], ?

    Input/output, int ITRAN[CLUSTER_NUM],
    1 if cluster L is updated in the quick-transfer stage,
    0 otherwise.  Reset to zero on output.

    Input/output, int LIVE[CLUSTER_NUM], ?

    Input/output, int *INDX, ?
*/
{
  double al1;
  double al2;
  double alt;
  double alw;
  double dc;
  int i;
  int j;
  int k;
  int l;
  int l1;
  int l2;
  int ll;
  double r2;
  double rr;
/*
  If cluster L is updated in the last quick-transfer stage, it
  belongs to the live set throughout this stage.   Otherwise, at
  each step, it is not in the live set if it has not been updated
  in the last POINT_NUM optimal transfer steps.
*/
  for ( k = 0; k < cluster_num; k++ )
  {
    if ( itran[k] == 1 )
    {
      live[k] = point_num + 1;
    }
  }

  for ( j = 0; j < point_num; j++ )
  {
    *indx = *indx + 1;
    l1 = cluster[j];
    l2 = cluster2[j];
    ll = l2;
/*
  If point J is the only member of cluster L1, no transfer.
*/
    if ( 1 < cluster_population[l1] )
    {
/*
  If L1 has been updated in this stage, re-compute D(I).
*/
      if ( ncp[l1] != 0 )
      {
        d[j] = 0.0;
        for ( i = 0; i < dim_num; i++ )
        {
          d[j] = d[j] + pow ( point[i+j*dim_num] - cluster_center[i+l1*dim_num], 2 );
        }
        d[j] = an1[l1] * d[j];
      }
/*
  Find the cluster with minimum R2.
*/
      r2 = 0.0;
      for ( i = 0; i < dim_num; i++ )
      {
        r2 = r2 + pow ( point[i+j*dim_num] - cluster_center[i+l2*dim_num], 2 );
      }
      r2 = an2[l2] * r2;

      for ( l = 0; l < cluster_num; l++ )
      {
/*
  If LIVE(L1) <= J, then L1 is not in the live set.   If this is
  true, we only need to consider clusters that are in the live set
  for possible transfer of point J.

  Otherwise, we need to consider all possible clusters.
*/
        if ( ( j < live[l1] || j < live[l] ) && l != l1 && l != ll )
        {
          rr = r2 / an2[l];

          dc = 0.0;
          for ( i = 0; i < dim_num; i++ )
          {
            dc = dc + pow ( point[i+j*dim_num] - cluster_center[i+l*dim_num], 2 );
          }

          if ( dc < rr )
          {
            r2 = dc * an2[l];
            l2 = l;
          }
        }
      }
/*
  If no transfer is necessary, L2 is the new CLUSTER2(J).
*/
      if ( d[j] <= r2 )
      {
        cluster2[j] = l2;
      }
/*
  Update cluster centers, LIVE, NCP, AN1 and AN2 for clusters L1 and
  L2, and update CLUSTER(J) and CLUSTER2(J).
*/
      else
      {
        *indx = 0;
        live[l1] = point_num + j;
        live[l2] = point_num + j;
        ncp[l1] = j;
        ncp[l2] = j;
        al1 = cluster_population[l1];
        alw = al1 - 1.0;
        al2 = cluster_population[l2];
        alt = al2 + 1.0;

        for ( i = 0; i < dim_num; i++ )
        {
          cluster_center[i+l1*dim_num] = ( cluster_center[i+l1*dim_num] * al1
            - point[i+j*dim_num] ) / alw;

          cluster_center[i+l2*dim_num] = ( cluster_center[i+l2*dim_num] * al2
            + point[i+j*dim_num] ) / alt;
        }

        cluster_population[l1] = cluster_population[l1] - 1;
        cluster_population[l2] = cluster_population[l2] + 1;
        an2[l1] = alw / al1;

        if ( 1.0 < alw )
        {
          an1[l1] = alw / ( alw - 1.0 );
        }
        else
        {
          an1[l1] = r8_huge ( );
        }

        an1[l2] = alt / al2;
        an2[l2] = alt / ( alt + 1.0 );
        cluster[j] = l2;
        cluster2[j] = l1;
      }
    }

    if ( *indx == point_num )
    {
      return;
    }

  }
/*
  ITRAN(L) = 0 before entering QTRAN.
*/
  i4vec_zeros ( cluster_num, itran );
/*
  LIVE(L) has to be decreased by POINT_NUM before re-entering OPTRA.
*/
  for ( k = 0; k < cluster_num; k++ )
  {
    live[k] = live[k] - point_num;
  }

  return;
}
/******************************************************************************/

void kmeans_02_qtran ( int dim_num, int point_num, int cluster_num,
  double point[], double cluster_center[], int cluster[], int cluster2[],
  int cluster_population[], double an1[], double an2[], int ncp[], double d[],
  int itran[], int *indx )

/******************************************************************************/
/*
  Purpose:

    KMEANS_02_QTRAN carries out the quick transfer stage.

  Discussion:

    For each point I, CLUSTER(I) and CLUSTER2(I) are switched, if necessary,
    to reduce within-cluster sum of squares.  The cluster centers are
    updated after each step.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    13 October 2011

  Author:

    Original FORTRAN77 by John Hartigan, Manchek Wong.
    C++ version by John Burkardt.

  Reference:

    John Hartigan, Manchek Wong,
    Algorithm AS 136:
    A K-Means Clustering Algorithm,
    Applied Statistics,
    Volume 28, Number 1, 1979, pages 100-108.

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, double POINT[DIM_NUM*POINT_NUM], the coordinates
    of the points.

    Input/output, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM],
    the cluster centers.

    Input/output, int CLUSTER[POINT_NUM], the cluster
    each point belongs to.

    Input/output, int CLUSTER2[POINT_NUM], the cluster to
    which each point is most likely to be transferred to.

    Input/output, int CLUSTER_POPULATION[CLUSTER_NUM],
    the number of points in each cluster.

    Input/output, double AN1[CLUSTER_NUM],
    CLUSTER_POPULATION(L) / (CLUSTER_POPULATION(L) - 1).

    Input/output, double AN2[CLUSTER_NUM],
    CLUSTER_POPULATION(L) / (CLUSTER_POPULATION(L) + 1).

    Input/output, int NCP[CLUSTER_NUM], ?

    Input/output, double D[POINT_NUM], ?

    Input/output, int ITRAN[CLUSTER_NUM],
    1 if cluster L is updated in the quick-transfer stage,
    0 otherwise.

    Input/output, int *INDX, is set to 0 if any
    updating occurs.
*/
{
  double al1;
  double al2;
  double alt;
  double alw;
  int count;
  double dd;
  int i;
  int j;
  int l1;
  int l2;
  double r2;
  int step;
/*
  In the optimal transfer stage, NCP(L) indicates the step at which
  cluster L is last updated.   In the quick transfer stage, NCP(L)
  is equal to the step at which cluster L is last updated plus POINT_NUM.
*/
  count = 0;
  step = 0;

  for ( ; ; )
  {
    for ( j = 0; j < point_num; j++ )
    {
      count = count + 1;
      step = step + 1;
      l1 = cluster[j];
      l2 = cluster2[j];
/*
  If point I is the only member of cluster L1, no transfer.
*/
      if ( 1 < cluster_population[l1] )
      {
/*
  If NCP(L1) < STEP, no need to re-compute distance from point I to
  cluster L1.   Note that if cluster L1 is last updated exactly POINT_NUM
  steps ago, we still need to compute the distance from point I to
  cluster L1.
*/
        if ( step <= ncp[l1] )
        {
          d[j] = 0.0;
          for ( i = 0; i < dim_num; i++ )
          {
            d[j] = d[j] + pow ( point[i+j*dim_num] - cluster_center[i+l1*dim_num], 2 );
          }
          d[j] = an1[l1] * d[j];
        }
/*
  If STEP >= both NCP(L1) and NCP(L2) there will be no transfer of
  point I at this step.
*/
        if ( step < ncp[l1] || step < ncp[l2] )
        {
          r2 = d[j] / an2[l2];

          dd = 0.0;
          for ( i = 0; i < dim_num; i++ )
          {
            dd = dd + pow ( point[i+j*dim_num] - cluster_center[i+l2*dim_num], 2 );
          }
/*
  Update cluster centers, NCP, CLUSTER_POPULATION, ITRAN, AN1 and AN2
  for clusters L1 and L2.   Also update CLUSTER(J) and CLUSTER2(J).

  Note that if any updating occurs in this stage, INDX is set back to 0.
*/
          if ( dd < r2 )
          {
            count = 0;
            *indx = 0;
            itran[l1] = 1;
            itran[l2] = 1;
            ncp[l1] = step + point_num;
            ncp[l2] = step + point_num;
            al1 = cluster_population[l1];
            alw = al1 - 1.0;
            al2 = cluster_population[l2];
            alt = al2 + 1.0;

            for ( i = 0; i < dim_num; i++ )
            {
              cluster_center[i+l1*dim_num] = ( cluster_center[i+l1*dim_num] * al1
                - point[i+j*dim_num] ) / alw;

              cluster_center[i+l2*dim_num] = ( cluster_center[i+l2*dim_num] * al2
                + point[i+j*dim_num] ) / alt;
            }

            cluster_population[l1] = cluster_population[l1] - 1;
            cluster_population[l2] = cluster_population[l2] + 1;
            an2[l1] = alw / al1;

            if ( 1.0 < alw )
            {
              an1[l1] = alw / ( alw - 1.0 );
            }
            else
            {
              an1[l1] = r8_huge ( );
            }
            an1[l2] = alt / al2;
            an2[l2] = alt / ( alt + 1.0 );
            cluster[j] = l2;
            cluster2[j] = l1;
          }
        }
      }
/*
  If no re-allocation took place in the last POINT_NUM steps, return.
*/
      if ( count == point_num )
      {
        return;
      }
    }
  }
  return;
}
/******************************************************************************/

void kmeans_03 ( int dim_num, int point_num, int cluster_num, int it_max,
  int *it_num, double point[], int cluster[], double cluster_center[],
  int cluster_population[], double cluster_energy[] )

/******************************************************************************/
/*
  Purpose:

    KMEANS_03 applies the K-Means algorithm.

  Discussion:

    It is possible for a straightforward K-Means algorithm to
    halt at a non-optimal partition of the points.  This routine
    tries to improve the input partition if possible.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    10 October 2011

  Author:

    John Burkardt

  Reference:

    Wendy Martinez, Angel Martinez,
    Computational Statistics Handbook with MATLAB,
    pages 373-376,
    Chapman and Hall / CRC, 2002.

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of data points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, int IT_MAX, the maximum number of iterations.

    Output, int *IT_NUM, the number of iterations taken.

    Input, double POINT[DIM_NUM*POINT_NUM], the data points.

    Output, int CLUSTER[POINT_NUM], the cluster to which
    each point belongs.

    Input/output, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM], the
    centers associated with the clustering.  On output, these may
    have been altered.

    Output, int CLUSTER_POPULATION[CLUSTER_NUM], the number
    of points in each cluster.

    Output, double CLUSTER_ENERGY[CLUSTER_NUM], the energy of
    the clusters.
*/
{
  int ci;
  int cj;
  double *distsq;
  int i;
  int j;
  int k;
  double point_energy;
  double point_energy_min;
  int swap;
/*
  Check the input.
*/
  if ( cluster_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_03 - Fatal error!\n" );
    printf ( "  CLUSTER_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( dim_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_03 - Fatal error!\n" );
    printf ( "  DIM_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( point_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_03 - Fatal error!\n" );
    printf ( "  POINT_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( it_max < 0 )
  {
    printf ( "\n" );
    printf ( "KMEANS_03 - Fatal error!\n" );
    printf ( "  IT_MAX < 0.\n" );
    exit ( 1 );
  }
/*
  Assign each point to the nearest cluster center.
*/
  for ( j = 0; j < point_num; j++ )
  {
    point_energy_min = r8_huge ( );
    cluster[j] = -1;

    for ( k = 0; k < cluster_num; k++ )
    {
      point_energy = 0.0;
      for ( i = 0; i < dim_num; i++ )
      {
        point_energy = point_energy +
          pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
      }

      if ( point_energy < point_energy_min )
      {
        point_energy_min = point_energy;
        cluster[j] = k;
      }
    }
  }
/*
  Determine the cluster populations.
*/
  i4vec_zeros ( cluster_num, cluster_population );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    cluster_population[k] = cluster_population[k] + 1;
  }
/*
  Average the points in each cluster to get a new cluster center.
*/
  r8vec_zeros ( dim_num * cluster_num, cluster_center );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
        + point[i+j*dim_num];
    }
  }

  for ( k = 0; k < cluster_num; k++ )
  {
    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
        / ( double ) ( cluster_population[k] );
    }
  }
/*
  Carry out the iteration.
*/
  *it_num = 0;
  distsq = ( double * ) malloc ( cluster_num * sizeof ( double ) );

  while ( *it_num < it_max )
  {
    *it_num = *it_num + 1;

    swap = 0;

    for ( j = 0; j < point_num; j++ )
    {
      ci = cluster[j];

      if ( cluster_population[ci] <= 1 )
      {
        continue;
      }

      for ( cj = 0; cj < cluster_num; cj++ )
      {
        if ( cj == ci )
        {
          distsq[cj] = 0.0;
          for ( i = 0; i < dim_num; i++ )
          {
            distsq[cj] = distsq[cj]
              + pow ( point[i+j*dim_num] - cluster_center[i+cj*dim_num], 2 );
          }
          distsq[cj] = distsq[cj] * ( double ) ( cluster_population[cj] )
            / ( double ) ( cluster_population[cj] - 1 );
        }
        else if ( cluster_population[cj] == 0 )
        {
          for ( i = 0; i < dim_num; i++ )
          {
            cluster_center[i+cj*dim_num] = point[i+j*dim_num];
          }
          distsq[cj] = 0.0;
        }
        else
        {
          distsq[cj] = 0.0;
          for ( i = 0; i < dim_num; i++ )
          {
            distsq[cj] = distsq[cj]
              + pow ( point[i+j*dim_num] - cluster_center[i+cj*dim_num], 2 );
          }
          distsq[cj] = distsq[cj] * ( double ) ( cluster_population[cj] )
            / ( double ) ( cluster_population[cj] + 1 );
        }
      }
/*
  Find the index of the minimum value of DISTSQ.
*/
      k = r8vec_min_index ( cluster_num, distsq );
/*
  If that is not the cluster to which point I now belongs, move it there.
*/
      if ( k == ci )
      {
        continue;
      }

      cj = k;

      for ( i = 0; i < dim_num; i++ )
      {
        cluster_center[i+ci*dim_num] = ( ( double ) ( cluster_population[ci] )
          * cluster_center[i+ci*dim_num] - point[i+j*dim_num] )
          / ( double ) ( cluster_population[ci] - 1 );

        cluster_center[i+cj*dim_num] = ( ( double ) ( cluster_population[cj] )
          * cluster_center[i+cj*dim_num] + point[i+j*dim_num] )
          / ( double ) ( cluster_population[cj] + 1 );
      }
      cluster_population[ci] = cluster_population[ci] - 1;
      cluster_population[cj] = cluster_population[cj] + 1;

      cluster[j] = cj;
      swap = swap + 1;
    }
/*
  Exit if no reassignments were made during this iteration.
*/
    if ( swap == 0 )
    {
      break;
    }
  }
/*
  Compute the cluster energies.
*/
  r8vec_zeros ( cluster_num, cluster_energy );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];

    point_energy = 0.0;
    for ( i = 0; i < dim_num; i++ )
    {
      point_energy = point_energy +
        pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
    }
    cluster_energy[k] = cluster_energy[k] + point_energy;
  }

  free ( distsq );

  return;
}
/******************************************************************************/

void kmeans_w_01 ( int dim_num, int point_num, int cluster_num, int it_max,
  int *it_num, double point[], double weight[], int cluster[],
  double cluster_center[], int cluster_population[], double cluster_energy[] )

/******************************************************************************/
/*
  Purpose:

    KMEANS_W_01 applies the weighted K-Means algorithm.

  Discussion:

    The input data for the weight K-Means problem includes:
    * a set of N data points X in M dimensions,
    * a set of N nonnegative weights W,
    * a desired number of clusters K.
    * an initial set of cluster centers Z,
    * an (optional) initial set of cluster assignments.

    The goal is to determine K points Z, called cluster centers, and
    to assign each point X(I) to some cluster Z(J), so that we minimize
    the weighted standard deviation of the distance of each data point
    to the center of its cluster.  Writing J = CLUSTER(I) to
    indicate the index of the nearest cluster center Z(J) to the
    point X(I), the quantity we are trying to minimize is the sum
    of the weighted cluster energies E(J), where:

      E(J) = Sum ( 1 <= I <= N ) W(I) * || X(I) - Z(J) ||^2

    Here, we assume that we are using the Euclidean norm, so that

      || X(I) - Z(J) ||^2 = Sum ( 1 <= K <= M )
         ( X(I)(K) - Z(J)(K) )^2

    In this notation, X(I)(K) is the K-th spatial component of the
    I-th data point.

    Note that this routine should give the same results as KMEANS_01
    in any case in which all the entries of the WEIGHT vector are equal.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    10 October 2011

  Author:

    John Burkardt

  Reference:

    David Sparks,
    Algorithm AS 58: Euclidean Cluster Analysis,
    Applied Statistics,
    Volume 22, Number 1, 1973, pages 126-130.

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, int IT_MAX, the maximum number of iterations.

    Output, int *IT_NUM, the number of iterations taken.

    Input, double POINT[DIM_NUM*POINT_NUM], the points.

    Input, double WEIGHT[POINT_NUM], the weights.

    Output, int CLUSTER[POINT_NUM], indicates which cluster
    each point belongs to.

    Input/output, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM],
    the cluster centers.

    Output, int CLUSTER_POPULATION[CLUSTER_NUM], the number
    of points in each cluster.

    Output, double CLUSTER_ENERGY[CLUSTER_NUM], the
    cluster energies.
*/
{
  double *cluster_weight;
  double dc;
  double de;
  double *f;
  int i;
  int il;
  int ir;
  int j;
  int k;
  double point_energy;
  double point_energy_min;
  int swap;

  *it_num = 0;
/*
  Idiot checks.
*/
  if ( cluster_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_W_01 - Fatal error!\n" );
    printf ( "  CLUSTER_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( dim_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_W_01 - Fatal error!\n" );
    printf ( "  DIM_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( point_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_W_01 - Fatal error!\n" );
    printf ( "  POINT_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( it_max < 0 )
  {
    printf ( "\n" );
    printf ( "KMEANS_W_01 - Fatal error!\n" );
    printf ( "  IT_MAX < 0.\n" );
    exit ( 1 );
  }

  if ( r8vec_any_negative ( point_num, weight ) )
  {
    printf ( "\n" );
    printf ( "KMEANS_W_01 - Fatal error!\n" );
    printf ( "  Some weight entry is negative.\n" );
    exit ( 1 );
  }

  if ( r8vec_all_nonpositive ( point_num, weight ) )
  {
    printf ( "\n" );
    printf ( "KMEANS_W_01 - Fatal error!\n" );
    printf ( "  No weight entry is positive.\n" );
    exit ( 1 );
  }
/*
  Assign each point to the nearest cluster.
*/
  for ( j = 0; j < point_num; j++ )
  {
    point_energy_min = r8_huge ( );
    cluster[j] = -1;

    for ( k = 0; k < cluster_num; k++ )
    {
      point_energy = 0.0;
      for ( i = 0; i < dim_num; i++ )
      {
        point_energy = point_energy +
          pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
      }

      if ( point_energy < point_energy_min )
      {
        point_energy_min = point_energy;
        cluster[j] = k;
      }
    }
  }
/*
  Determine the cluster populations and weights.
*/
  i4vec_zeros ( cluster_num, cluster_population );
  cluster_weight = r8vec_zeros_new ( cluster_num );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    cluster_population[k] = cluster_population[k] + 1;
    cluster_weight[k] = cluster_weight[k] + weight[j];
  }
/*
  Calculate the mean and sum of squares for each cluster.
*/
  r8vec_zeros ( dim_num * cluster_num, cluster_center );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
        + weight[j] * point[i+j*dim_num];
    }
  }

  for ( k = 0; k < cluster_num; k++ )
  {
    if ( 0.0 < cluster_weight[k] )
    {
      for ( i = 0; i < dim_num; i++ )
      {
        cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
          / cluster_weight[k];
      }
    }
  }
/*
  Set the point energies.
*/
  f = r8vec_zeros_new ( point_num );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    for ( i = 0; i < dim_num; i++ )
    {
      f[j] = f[j] + pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
    }
  }
/*
  Set the cluster energies.
*/
  r8vec_zeros ( cluster_num, cluster_energy );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    cluster_energy[k] = cluster_energy[k] + weight[j] * f[j];
  }
/*
  Adjust the point energies by a weight factor.
*/
  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    if ( weight[j] < cluster_weight[k] )
    {
      f[j] = f[j] * cluster_weight[k] / ( cluster_weight[k] - weight[j] );
    }
  }
/*
  Examine each observation in turn to see if it should be
  reassigned to a different cluster.
*/
  *it_num = 0;

  while ( *it_num < it_max )
  {
    *it_num = *it_num + 1;

    swap = 0;

    for ( j = 0; j < point_num; j++ )
    {
      il = cluster[j];
      ir = il;

      if ( cluster_population[il] <= 1 )
      {
        continue;
      }

      dc = f[j];

      for ( k = 0; k < cluster_num; k++ )
      {
        if ( k != il )
        {
          de = 0.0;
          for ( i = 0; i < dim_num; i++ )
          {
            de = de + pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 )
              * cluster_weight[k] / ( cluster_weight[k] + weight[j] );
          }

          if ( de < dc )
          {
            dc = de;
            ir = k;
          }
        }
      }
/*
  If the lowest value was obtained by staying in the current cluster,
  then cycle.
*/
      if ( ir == il )
      {
        continue;
      }
/*
  Reassign the point from cluster IL to cluster IR.
*/
      for ( i = 0; i < dim_num; i++ )
      {
        cluster_center[i+il*dim_num] =
          ( cluster_weight[il] * cluster_center[i+il*dim_num]
          - weight[j] * point[i+j*dim_num] )
          / ( cluster_weight[il] - weight[j] );

        cluster_center[i+ir*dim_num] =
          ( cluster_weight[ir] * cluster_center[i+ir*dim_num]
          + weight[j] * point[i+j*dim_num] )
          / ( cluster_weight[ir] + weight[j] );
      }
      cluster_weight[il] = cluster_weight[il] - weight[j];
      cluster_weight[ir] = cluster_weight[ir] + weight[j];

      cluster_energy[il] = cluster_energy[il] - f[j];
      cluster_energy[ir] = cluster_energy[ir] + dc;

      cluster_population[ir] = cluster_population[ir] + 1;
      cluster_population[il] = cluster_population[il] - 1;

      cluster[j] = ir;
/*
  Adjust the value of F for all points in clusters IL and IR.
*/
      for ( j = 0; j < point_num; j++ )
      {
        k = cluster[j];

        if ( k == il || k == ir )
        {
          f[j] = 0.0;
          for ( i = 0; i < dim_num; i++ )
          {
            f[j] = f[j] + pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
          }
          if ( weight[j] < cluster_weight[k] )
          {
            f[j] = f[j] * cluster_weight[k] / ( cluster_weight[k] - weight[j] );
          }
        }
      }
      swap = swap + 1;
    }
/*
  Exit if no reassignments were made during this iteration.
*/
    if ( swap == 0 )
    {
      break;
    }
  }
/*
  Compute the energy based on the final value of the cluster centers.
*/
  r8vec_zeros ( cluster_num, cluster_energy );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];

    point_energy = 0.0;
    for ( i = 0; i < dim_num; i++ )
    {
      point_energy = point_energy +
        pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
    }
    cluster_energy[k] = cluster_energy[k] + weight[j] * point_energy;
  }

  free ( cluster_weight );
  free ( f );

  return;
}
/******************************************************************************/

void kmeans_w_03 ( int dim_num, int point_num, int cluster_num, int it_max,
  int *it_num, double point[], double weight[], int cluster[],
  double cluster_center[], int cluster_population[], double cluster_energy[] )

/******************************************************************************/
/*
  Purpose:

    KMEANS_W_03 applies the weighted K-Means algorithm.

  Discussion:

    The input data for the weight K-Means problem includes:
    * a set of N data points X in M dimensions,
    * a set of N nonnegative weights W,
    * a desired number of clusters K.
    * an initial set of cluster centers Z,
    * an (optional) initial set of cluster assignments.

    The goal is to determine K points Z, called cluster centers, and
    to assign each point X(I) to some cluster Z(J), so that we minimize
    the weighted standard deviation of the distance of each data point
    to the center of its cluster.  Writing J = CLUSTER(I) to
    indicate the index of the nearest cluster center Z(J) to the
    point X(I), the quantity we are trying to minimize is the sum
    of the weighted cluster energies E(J), where:

      E(J) = Sum ( 1 <= I <= N ) W(I) * || X(I) - Z(J) ||^2

    Here, we assume that we are using the Euclidean norm, so that

      || X(I) - Z(J) ||^2 = Sum ( 1 <= K <= M )
        ( X(I)(K) - Z(J)(K) )^2

    In this notation, X(I)(K) is the K-th spatial component of the
    I-th data point.

    Note that this routine should give the same results as KMEANS_01
    in any case in which all the entries of the WEIGHT vector are equal.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    10 October 2011

  Author:

    John Burkardt

  Reference:

    Wendy Martinez, Angel Martinez,
    Computational Statistics Handbook with MATLAB,
    pages 373-376,
    Chapman and Hall / CRC, 2002.

  Parameters:

    Input, int DIM_NUM, the number of spatial dimensions.

    Input, int POINT_NUM, the number of data points.

    Input, int CLUSTER_NUM, the number of clusters.

    Input, int IT_MAX, the maximum number of iterations.

    Output, int *IT_NUM, the number of iterations taken.

    Input, double POINT[DIM_NUM*POINT_NUM], the data points.

    Input, double WEIGHT[POINT_NUM], the weights.

    Input/output, int CLUSTER[POINT_NUM], the cluster
    to which each point belongs.  On output, these may have been altered.

    Input/output, double CLUSTER_CENTER[DIM_NUM*CLUSTER_NUM], the
    centers associated with the clustering.  On output, these may
    have been altered.

    Output, int CLUSTER_POPULATION[CLUSTER_NUM], the number
    of points in each cluster.

    Output, double CLUSTER_ENERGY[CLUSTER_NUM], the energy of
    the clusters.
*/
{
  int ci;
  int cj;
  double *cluster_weight;
  double *distsq;
  int i;
  int j;
  int k;
  double point_energy;
  double point_energy_min;
  int swap;
/*
  Check the input.
*/
  if ( cluster_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_W_03 - Fatal error!\n" );
    printf ( "  CLUSTER_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( dim_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_W_03 - Fatal error!\n" );
    printf ( "  DIM_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( point_num < 1 )
  {
    printf ( "\n" );
    printf ( "KMEANS_W_03 - Fatal error!\n" );
    printf ( "  POINT_NUM < 1.\n" );
    exit ( 1 );
  }

  if ( it_max < 0 )
  {
    printf ( "\n" );
    printf ( "KMEANS_W_03 - Fatal error!\n" );
    printf ( "  IT_MAX < 0.\n" );
    exit ( 1 );
  }

  if ( r8vec_any_negative ( point_num, weight ) )
  {
    printf ( "\n" );
    printf ( "KMEANS_W_03 - Fatal error!\n" );
    printf ( "  Some weight entry is negative.\n" );
    exit ( 1 );
  }

  if ( r8vec_all_nonpositive ( point_num, weight ) )
  {
    printf ( "\n" );
    printf ( "KMEANS_W_03 - Fatal error!\n" );
    printf ( "  No weight entry is positive.\n" );
    exit ( 1 );
  }
/*
  Assign each observation to the nearest cluster center.
*/
  for ( j = 0; j < point_num; j++ )
  {
    point_energy_min = r8_huge ( );
    cluster[j] = -1;

    for ( k = 0; k < cluster_num; k++ )
    {
      point_energy = 0.0;
      for ( i = 0; i < dim_num; i++ )
      {
        point_energy = point_energy +
          pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
      }

      if ( point_energy < point_energy_min )
      {
        point_energy_min = point_energy;
        cluster[j] = k;
      }
    }
  }
/*
  Determine the cluster populations and weights.
*/
  i4vec_zeros ( cluster_num, cluster_population );
  cluster_weight = r8vec_zeros_new ( cluster_num );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    cluster_population[k] = cluster_population[k] + 1;
    cluster_weight[k] = cluster_weight[k] + weight[j];
  }
/*
  Average the points in each cluster to get a new cluster center.
*/
  r8vec_zeros ( dim_num * cluster_num, cluster_center );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];
    for ( i = 0; i < dim_num; i++ )
    {
      cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num]
        + weight[j] * point[i+j*dim_num];
    }
  }

  for ( k = 0; k < cluster_num; k++ )
  {
    if ( cluster_weight[k] != 0.0 )
    {
      for ( i = 0; i < dim_num; i++ )
      {
        cluster_center[i+k*dim_num] = cluster_center[i+k*dim_num] /
          cluster_weight[k];
      }
    }
  }
/*
  Carry out the iteration.
*/
  *it_num = 0;
  distsq = ( double * ) malloc ( cluster_num * sizeof ( double ) );

  while ( *it_num < it_max )
  {
    *it_num = *it_num + 1;

    swap = 0;

    for ( j = 0; j < point_num; j++ )
    {
      ci = cluster[j];

      if ( cluster_population[ci] <= 1 )
      {
        continue;
      }

      for ( cj = 0; cj < cluster_num; cj++ )
      {
        if ( cj == ci )
        {
          distsq[cj] = 0.0;
          for ( i = 0; i < dim_num; i++ )
          {
            distsq[cj] = distsq[cj]
              + pow ( point[i+j*dim_num] - cluster_center[i+cj*dim_num], 2 );
          }
          distsq[cj] = distsq[cj] * cluster_weight[cj]
            / ( cluster_weight[cj] - weight[j] );
        }
        else if ( cluster_population[cj] == 0 )
        {
          for ( i = 0; i < dim_num; i++ )
          {
            cluster_center[i+cj*dim_num] = point[i+j*dim_num];
          }
          distsq[cj] = 0.0;
        }
        else
        {
          distsq[cj] = 0.0;
          for ( i = 0; i < dim_num; i++ )
          {
            distsq[cj] = distsq[cj]
              + pow ( point[i+j*dim_num] - cluster_center[i+cj*dim_num], 2 );
          }
          distsq[cj] = distsq[cj] * cluster_weight[cj]
            / ( cluster_weight[cj] + weight[j] );
        }
      }
/*
  Find the index of the minimum value of DISTSQ.
*/
      k = r8vec_min_index ( cluster_num, distsq );
/*
  If that is not the cluster to which point I now belongs, move it there.
*/
      if ( k == ci )
      {
        continue;
      }

      cj = k;

      for ( i = 0; i < dim_num; i++ )
      {
        cluster_center[i+ci*dim_num] =
          ( cluster_weight[ci] * cluster_center[i+ci*dim_num]
          - weight[j] * point[i+j*dim_num] )
          / ( cluster_weight[ci] - weight[j] );

        cluster_center[i+cj*dim_num] =
          ( cluster_weight[cj] * cluster_center[i+cj*dim_num]
          + weight[j] * point[i+j*dim_num] )
          / ( cluster_weight[cj] + weight[j] );
      }
      cluster_population[ci] = cluster_population[ci] - 1;
      cluster_population[cj] = cluster_population[cj] + 1;

      cluster_weight[ci] = cluster_weight[ci] - weight[j];
      cluster_weight[cj] = cluster_weight[cj] + weight[j];

      cluster[j] = cj;

      swap = swap + 1;
    }
/*
  Exit if no reassignments were made during this iteration.
*/
    if ( swap == 0 )
    {
      break;
    }
  }
/*
  Compute the energy based on the final value of the cluster centers.
*/
  r8vec_zeros ( cluster_num, cluster_energy );

  for ( j = 0; j < point_num; j++ )
  {
    k = cluster[j];

    point_energy = 0.0;
    for ( i = 0; i < dim_num; i++ )
    {
      point_energy = point_energy +
        pow ( point[i+j*dim_num] - cluster_center[i+k*dim_num], 2 );
    }
    cluster_energy[k] = cluster_energy[k] + weight[j] * point_energy;
  }

  free ( cluster_weight );
  free ( distsq );

  return;
}
/******************************************************************************/

double r8_huge ( )

/******************************************************************************/
/*
  Purpose:

    R8_HUGE returns a "huge" R8.

  Discussion:

    The value returned by this function is intended to be the largest
    representable real number.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    27 September 2014

  Author:

    John Burkardt

  Parameters:

    Output, double R8_HUGE, a "huge" R8 value.
*/
{
  double value;

  value = 1.79769313486231571E+308;

  return value;
}
/******************************************************************************/

double r8_max ( double x, double y )

/******************************************************************************/
/*
  Purpose:

    R8_MAX returns the maximum of two R8's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    07 May 2006

  Author:

    John Burkardt

  Parameters:

    Input, double X, Y, the quantities to compare.

    Output, double R8_MAX, the maximum of X and Y.
*/
{
  double value;

  if ( y < x )
  {
    value = x;
  }
  else
  {
    value = y;
  }
  return value;
}
/******************************************************************************/

double r8_min ( double x, double y )

/******************************************************************************/
/*
  Purpose:

    R8_MIN returns the minimum of two R8's.

  Discussion:

    The C math library provides the function fmin() which should be preferred.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    07 May 2006

  Author:

    John Burkardt

  Parameters:

    Input, double X, Y, the quantities to compare.

    Output, double R8_MIN, the minimum of X and Y.
*/
{
  double value;

  if ( y < x )
  {
    value = y;
  }
  else
  {
    value = x;
  }
  return value;
}
/******************************************************************************/

double r8_uniform_01 ( int *seed )

/******************************************************************************/
/*
  Purpose:

    R8_UNIFORM_01 returns a pseudorandom R8 scaled to [0,1].

  Discussion:

    This routine implements the recursion

      seed = 16807 * seed mod ( 2^31 - 1 )
      r8_uniform_01 = seed / ( 2^31 - 1 )

    The integer arithmetic never requires more than 32 bits,
    including a sign bit.

    If the initial seed is 12345, then the first three computations are

      Input     Output      R8_UNIFORM_01
      SEED      SEED

         12345   207482415  0.096616
     207482415  1790989824  0.833995
    1790989824  2035175616  0.947702

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    11 August 2004

  Author:

    John Burkardt

  Reference:

    Paul Bratley, Bennett Fox, Linus Schrage,
    A Guide to Simulation,
    Springer Verlag, pages 201-202, 1983.

    Pierre L'Ecuyer,
    Random Number Generation,
    in Handbook of Simulation
    edited by Jerry Banks,
    Wiley Interscience, page 95, 1998.

    Bennett Fox,
    Algorithm 647:
    Implementation and Relative Efficiency of Quasirandom
    Sequence Generators,
    ACM Transactions on Mathematical Software,
    Volume 12, Number 4, pages 362-376, 1986.

    P A Lewis, A S Goodman, J M Miller,
    A Pseudo-Random Number Generator for the System/360,
    IBM Systems Journal,
    Volume 8, pages 136-143, 1969.

  Parameters:

    Input/output, int *SEED, the "seed" value.  Normally, this
    value should not be 0.  On output, SEED has been updated.

    Output, double R8_UNIFORM_01, a new pseudorandom variate, strictly between
    0 and 1.
*/
{
  const int i4_huge = 2147483647;
  int k;
  double r;

  if ( *seed == 0 )
  {
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "R8_UNIFORM_01 - Fatal error!\n" );
    fprintf ( stderr, "  Input value of SEED = 0\n" );
    exit ( 1 );
  }

  k = *seed / 127773;

  *seed = 16807 * ( *seed - k * 127773 ) - k * 2836;

  if ( *seed < 0 )
  {
    *seed = *seed + i4_huge;
  }

  r = ( ( double ) ( *seed ) ) * 4.656612875E-10;

  return r;
}
/******************************************************************************/

double *r8mat_data_read ( char *input_filename, int m, int n )

/******************************************************************************/
/*
  Purpose:

    R8MAT_DATA_READ reads the data from an R8MAT file.

  Discussion:

    An R8MAT is an array of R8's.

    The file is assumed to contain one record per line.

    Records beginning with the '#' character are comments, and are ignored.
    Blank lines are also ignored.

    Each line that is not ignored is assumed to contain exactly (or at least)
    M real numbers, representing the coordinates of a point.

    There are assumed to be exactly (or at least) N such records.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    27 January 2005

  Author:

    John Burkardt

  Parameters:

    Input, char *INPUT_FILENAME, the name of the input file.

    Input, int M, the number of spatial dimensions.

    Input, int N, the number of points.  The program
    will stop reading data once N values have been read.

    Output, double R8MAT_DATA_READ[M*N], the data.
*/
{
# define MY_LINE_MAX 255

  int error;
  char *got_string;
  FILE *input;
  int i;
  int j;
  char line[255];
  double *table;
  double *x;

  input = fopen ( input_filename, "r" );

  if ( !input )
  {
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "R8MAT_DATA_READ - Fatal error!\n" );
    fprintf ( stderr, "  Could not open the input file: \"%s\"\n", input_filename );
    exit ( 1 );
  }

  table = ( double * ) malloc ( m * n * sizeof ( double ) );

  x = ( double * ) malloc ( m * sizeof ( double ) );

  j = 0;

  while ( j < n )
  {
    got_string = fgets ( line, MY_LINE_MAX, input );

    if ( !got_string )
    {
      break;
    }

    if ( line[0] == '#' || s_len_trim ( line ) == 0 )
    {
      continue;
    }

    error = s_to_r8vec ( line, m, x );

    if ( error == 1 )
    {
      continue;
    }

    for ( i = 0; i < m; i++ )
    {
      table[i+j*m] = x[i];
    }
    j = j + 1;

  }

  fclose ( input );

  free ( x );

  return table;

# undef MY_LINE_MAX
}
/******************************************************************************/
 
void r8mat_header_read ( char *input_filename, int *m, int *n )
 
/******************************************************************************/
/*
  Purpose:

    R8MAT_HEADER_READ reads the header from an R8MAT file.

  Discussion:

    An R8MAT is an array of R8's.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    04 June 2004

  Author:

    John Burkardt

  Parameters:

    Input, char *INPUT_FILENAME, the name of the input file.

    Output, int *M, the number of spatial dimensions.

    Output, int *N, the number of points.
*/
{
  *m = file_column_count ( input_filename );

  if ( *m <= 0 )
  {
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "R8MAT_HEADER_READ - Fatal error!\n" );
    fprintf ( stderr, "  FILE_COLUMN_COUNT failed.\n" );
    exit ( 1 );
  }

  *n = file_row_count ( input_filename );

  if ( *n <= 0 )
  {
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "R8MAT_HEADER_READ - Fatal error!\n" );
    fprintf ( stderr, "  FILE_ROW_COUNT failed.\n" );
    exit ( 1 );
  }

  return;
}
/******************************************************************************/

double *r8mat_mm_new ( int n1, int n2, int n3, double a[], double b[] )

/******************************************************************************/
/*
  Purpose:

    R8MAT_MM_NEW multiplies two matrices.

  Discussion:

    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
    in column-major order.

    For this routine, the result is returned as the function value.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    08 April 2009

  Author:

    John Burkardt

  Parameters:

    Input, int N1, N2, N3, the order of the matrices.

    Input, double A[N1*N2], double B[N2*N3], the matrices to multiply.

    Output, double R8MAT_MM_NEW[N1*N3], the product matrix C = A * B.
*/
{
  double *c;
  int i;
  int j;
  int k;

  c = ( double * ) malloc ( n1 * n3 * sizeof ( double ) );

  for ( i = 0; i < n1; i++ )
  {
    for ( j = 0; j < n3; j++ )
    {
      c[i+j*n1] = 0.0;
      for ( k = 0; k < n2; k++ )
      {
        c[i+j*n1] = c[i+j*n1] + a[i+k*n1] * b[k+j*n2];
      }
    }
  }

  return c;
}
/******************************************************************************/

void r8mat_uniform_01 ( int m, int n, int *seed, double r[] )

/******************************************************************************/
/*
  Purpose:

    R8MAT_UNIFORM_01 fills an R8MAT with pseudorandom values scaled to [0,1].

  Discussion:

    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
    in column-major order.

    This routine implements the recursion

      seed = 16807 * seed mod ( 2^31 - 1 )
      unif = seed / ( 2^31 - 1 )

    The integer arithmetic never requires more than 32 bits,
    including a sign bit.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    30 June 2009

  Author:

    John Burkardt

  Reference:

    Paul Bratley, Bennett Fox, Linus Schrage,
    A Guide to Simulation,
    Springer Verlag, pages 201-202, 1983.

    Bennett Fox,
    Algorithm 647:
    Implementation and Relative Efficiency of Quasirandom
    Sequence Generators,
    ACM Transactions on Mathematical Software,
    Volume 12, Number 4, pages 362-376, 1986.

    Philip Lewis, Allen Goodman, James Miller,
    A Pseudo-Random Number Generator for the System/360,
    IBM Systems Journal,
    Volume 8, pages 136-143, 1969.

  Parameters:

    Input, int M, N, the number of rows and columns.

    Input/output, int *SEED, the "seed" value.  Normally, this
    value should not be 0, otherwise the output value of SEED
    will still be 0, and R8_UNIFORM will be 0.  On output, SEED has
    been updated.

    Output, double R[M*N], a matrix of pseudorandom values.
*/
{
  int i;
  const int i4_huge = 2147483647;
  int j;
  int k;

  if ( *seed == 0 )
  {
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "R8MAT_UNIFORM_01 - Fatal error!\n" );
    fprintf ( stderr, "  Input value of SEED = 0\n" );
    exit ( 1 );
  }

  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      k = *seed / 127773;

      *seed = 16807 * ( *seed - k * 127773 ) - k * 2836;

      if ( *seed < 0 )
      {
        *seed = *seed + i4_huge;
      }

      r[i+j*m] = ( double ) ( *seed ) * 4.656612875E-10;
    }
  }

  return;
}
/******************************************************************************/

double *r8mat_uniform_01_new ( int m, int n, int *seed )

/******************************************************************************/
/*
  Purpose:

    R8MAT_UNIFORM_01_NEW fills an R8MAT with pseudorandom values scaled to [0,1].

  Discussion:

    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
    in column-major order.

    This routine implements the recursion

      seed = 16807 * seed mod ( 2^31 - 1 )
      unif = seed / ( 2^31 - 1 )

    The integer arithmetic never requires more than 32 bits,
    including a sign bit.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    30 June 2009

  Author:

    John Burkardt

  Reference:

    Paul Bratley, Bennett Fox, Linus Schrage,
    A Guide to Simulation,
    Springer Verlag, pages 201-202, 1983.

    Bennett Fox,
    Algorithm 647:
    Implementation and Relative Efficiency of Quasirandom
    Sequence Generators,
    ACM Transactions on Mathematical Software,
    Volume 12, Number 4, pages 362-376, 1986.

    Philip Lewis, Allen Goodman, James Miller,
    A Pseudo-Random Number Generator for the System/360,
    IBM Systems Journal,
    Volume 8, pages 136-143, 1969.

  Parameters:

    Input, int M, N, the number of rows and columns.

    Input/output, int *SEED, the "seed" value.  Normally, this
    value should not be 0, otherwise the output value of SEED
    will still be 0, and R8_UNIFORM will be 0.  On output, SEED has
    been updated.

    Output, double R8MAT_UNIFORM_01_NEW[M*N], a matrix of pseudorandom values.
*/
{
  int i;
  const int i4_huge = 2147483647;
  int j;
  int k;
  double *r;

  if ( *seed == 0 )
  {
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "R8MAT_UNIFORM_01_NEW - Fatal error!\n" );
    fprintf ( stderr, "  Input value of SEED = 0\n" );
    exit ( 1 );
  }

  r = ( double * ) malloc ( m * n * sizeof ( double ) );

  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      k = *seed / 127773;

      *seed = 16807 * ( *seed - k * 127773 ) - k * 2836;

      if ( *seed < 0 )
      {
        *seed = *seed + i4_huge;
      }
      r[i+j*m] = ( double ) ( *seed ) * 4.656612875E-10;
    }
  }

  return r;
}
/******************************************************************************/

void r8mat_write ( char *output_filename, int m, int n, double table[] )

/******************************************************************************/
/*
  Purpose:

    R8MAT_WRITE writes an R8MAT file.

  Discussion:

    An R8MAT is an array of R8's.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    01 June 2009

  Author:

    John Burkardt

  Parameters:

    Input, char *OUTPUT_FILENAME, the output filename.

    Input, int M, the spatial dimension.

    Input, int N, the number of points.

    Input, double TABLE[M*N], the data.
*/
{
  int i;
  int j;
  FILE *output;
/*
  Open the file.
*/
  output = fopen ( output_filename, "wt" );

  if ( !output )
  {
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "R8MAT_WRITE - Fatal error!\n" );
    fprintf ( stderr, "  Could not open the file '%s'.\n", output_filename );
    exit ( 1 );
  }
/*
  Write the data.
*/
  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      fprintf ( output, "  %24.16g", table[i+j*m] );
    }
    fprintf ( output, "\n" );
  }
/*
  Close the file.
*/
  fclose ( output );

  return;
}
/******************************************************************************/

int r8vec_all_nonpositive ( int n, double a[] )

/******************************************************************************/
/*
  Purpose:

    R8VEC_ALL_NONPOSITIVE: ( all ( A <= 0 ) ) for R8VEC's.

  Discussion:

    An R8VEC is a vector of R8's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    08 October 2011

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries.

    Input, double A[N], the vector to check.

    Output, int R8VEC_ALL_NONPOSITIVE is 1 if all entries
    of A are less than or equal to zero.
*/
{
  int i;
  int value;

  for ( i = 0; i < n; i++ )
  {
    if ( 0.0 < a[i] )
    {
      value = 0;
      return value;
    }
  }
  value = 1;

  return value;
}
/******************************************************************************/

int r8vec_any_negative ( int n, double a[] )

/******************************************************************************/
/*
  Purpose:

    R8VEC_ANY_NEGATIVE: ( any A < 0 ) for R8VEC's.

  Discussion:

    An R8VEC is a vector of R8's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    09 October 2011

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries.

    Input, double A[N], the vector to check.

    Output, int R8VEC_ANY_NEGATIVE is 1 if any entry
    of A is less than zero.
*/
{
  int i;
  int value;

  for ( i = 0; i < n; i++ )
  {
    if ( a[i] < 0.0 )
    {
      value = 1;
      return value;
    }
  }
  value = 0;

  return value;
}
/******************************************************************************/

double r8vec_i4vec_dot_product ( int n, double r8vec[], int i4vec[] )

/******************************************************************************/
/*
  Purpose:

    R8VEC_I4VEC_DOT_PRODUCT computes the dot product of an R8VEC and an I4VEC.

  Discussion:

    An R8VEC is a vector of R8's.

    An I4VEC is a vector of I4's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    30 June 2009

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vectors.

    Input, double R8VEC[N], the first vector.

    Input, int I4VEC[N], the second vector.

    Output, double R8VEC_I4VEC_DOT_PRODUCT, the dot product of the vectors.
*/
{
  int i;
  double value;

  value = 0.0;
  for ( i = 0; i < n; i++ )
  {
    value = value + r8vec[i] * ( double ) ( i4vec[i] );
  }
  return value;
}
/******************************************************************************/

int r8vec_min_index ( int n, double a[] )

/******************************************************************************/
/*
  Purpose:

    R8VEC_MIN_INDEX returns the index of the minimum value in an R8VEC.

  Discussion:

    An R8VEC is a vector of R8's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    08 November 2009

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the array.

    Input, double A[N], the array.

    Output, int R8VEC_MIN_INDEX, the index of the smallest entry.
*/
{
  int i;
  int value;

  if ( n <= 0 )
  {
    value = - 1;
  }
  else
  {
    value = 0;
    for ( i = 1; i < n; i++ )
    {
      if ( a[i] < a[value] )
      {
        value = i;
      }
    }
  }
  return value;
}
/******************************************************************************/

double r8vec_sum ( int n, double a[] )

/******************************************************************************/
/*
  Purpose:

    R8VEC_SUM returns the sum of an R8VEC.

  Discussion:

    An R8VEC is a vector of R8's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    26 August 2008

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vector.

    Input, double A[N], the vector.

    Output, double R8VEC_SUM, the sum of the vector.
*/
{
  int i;
  double value;

  value = 0.0;
  for ( i = 0; i < n; i++ )
  {
    value = value + a[i];
  }

  return value;
}
/******************************************************************************/

void r8vec_uniform_01 ( int n, int *seed, double r[] )

/******************************************************************************/
/*
  Purpose:

    R8VEC_UNIFORM_01 returns a unit pseudorandom R8VEC.

  Discussion:

    This routine implements the recursion

      seed = 16807 * seed mod ( 2^31 - 1 )
      unif = seed / ( 2^31 - 1 )

    The integer arithmetic never requires more than 32 bits,
    including a sign bit.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    19 August 2004

  Author:

    John Burkardt

  Reference:

    Paul Bratley, Bennett Fox, Linus Schrage,
    A Guide to Simulation,
    Second Edition,
    Springer, 1987,
    ISBN: 0387964673,
    LC: QA76.9.C65.B73.

    Bennett Fox,
    Algorithm 647:
    Implementation and Relative Efficiency of Quasirandom
    Sequence Generators,
    ACM Transactions on Mathematical Software,
    Volume 12, Number 4, December 1986, pages 362-376.

    Pierre L'Ecuyer,
    Random Number Generation,
    in Handbook of Simulation,
    edited by Jerry Banks,
    Wiley, 1998,
    ISBN: 0471134031,
    LC: T57.62.H37.

    Peter Lewis, Allen Goodman, James Miller,
    A Pseudo-Random Number Generator for the System/360,
    IBM Systems Journal,
    Volume 8, Number 2, 1969, pages 136-143.

  Parameters:

    Input, int N, the number of entries in the vector.

    Input/output, int *SEED, a seed for the random number generator.

    Output, double R[N], the vector of pseudorandom values.
*/
{
  int i;
  const int i4_huge = 2147483647;
  int k;

  if ( *seed == 0 )
  {
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "R8VEC_UNIFORM_01 - Fatal error!\n" );
    fprintf ( stderr, "  Input value of SEED = 0.\n" );
    exit ( 1 );
  }

  for ( i = 0; i < n; i++ )
  {
    k = *seed / 127773;

    *seed = 16807 * ( *seed - k * 127773 ) - k * 2836;

    if ( *seed < 0 )
    {
      *seed = *seed + i4_huge;
    }

    r[i] = ( double ) ( *seed ) * 4.656612875E-10;
  }

  return;
}
/******************************************************************************/

void r8vec_zeros ( int n, double a[] )

/******************************************************************************/
/*
  Purpose:

    R8VEC_ZEROS zeroes an R8VEC.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    26 August 2008

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vector.

    Output, double A[N], a vector of zeroes.
*/
{
  int i;

  for ( i = 0; i < n; i++ )
  {
    a[i] = 0.0;
  }
  return;
}
/******************************************************************************/

double *r8vec_zeros_new ( int n )

/******************************************************************************/
/*
  Purpose:

    R8VEC_ZEROS_NEW creates and zeroes an R8VEC.

  Discussion:

    An R8VEC is a vector of R8's.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    25 March 2009

  Author:

    John Burkardt

  Parameters:

    Input, int N, the number of entries in the vector.

    Output, double R8VEC_ZEROS_NEW[N], a vector of zeroes.
*/
{
  double *a;
  int i;

  a = ( double * ) malloc ( n * sizeof ( double ) );

  for ( i = 0; i < n; i++ )
  {
    a[i] = 0.0;
  }
  return a;
}
/******************************************************************************/

int s_len_trim ( char *s )

/******************************************************************************/
/*
  Purpose:

    S_LEN_TRIM returns the length of a string to the last nonblank.

  Discussion:

    It turns out that I also want to ignore the '\n' character!

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    05 October 2014

  Author:

    John Burkardt

  Parameters:

    Input, char *S, a pointer to a string.

    Output, int S_LEN_TRIM, the length of the string to the last nonblank.
    If S_LEN_TRIM is 0, then the string is entirely blank.
*/
{
  int n;
  char *t;

  n = strlen ( s );
  t = s + strlen ( s ) - 1;

  while ( 0 < n )
  {
    if ( *t != ' ' && *t != '\n' )
    {
      return n;
    }
    t--;
    n--;
  }

  return n;
}
/******************************************************************************/

double s_to_r8 ( char *s, int *lchar, int *error )

/******************************************************************************/
/*
  Purpose:

    S_TO_R8 reads an R8 value from a string.

  Discussion:

    We have had some trouble with input of the form 1.0E-312.
    For now, let's assume anything less than 1.0E-20 is zero.

    This routine will read as many characters as possible until it reaches
    the end of the string, or encounters a character which cannot be
    part of the real number.

    Legal input is:

       1 blanks,
       2 '+' or '-' sign,
       2.5 spaces
       3 integer part,
       4 decimal point,
       5 fraction part,
       6 'E' or 'e' or 'D' or 'd', exponent marker,
       7 exponent sign,
       8 exponent integer part,
       9 exponent decimal point,
      10 exponent fraction part,
      11 blanks,
      12 final comma or semicolon.

    with most quantities optional.

  Example:

    S                 R

    '1'               1.0
    '     1   '       1.0
    '1A'              1.0
    '12,34,56'        12.0
    '  34 7'          34.0
    '-1E2ABCD'        -100.0
    '-1X2ABCD'        -1.0
    ' 2E-1'           0.2
    '23.45'           23.45
    '-4.2E+2'         -420.0
    '17d2'            1700.0
    '-14e-2'         -0.14
    'e2'              100.0
    '-12.73e-9.23'   -12.73 * 10.0^(-9.23)

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    24 June 2005

  Author:

    John Burkardt

  Parameters:

    Input, char *S, the string containing the
    data to be read.  Reading will begin at position 1 and
    terminate at the end of the string, or when no more
    characters can be read to form a legal real.  Blanks,
    commas, or other nonnumeric data will, in particular,
    cause the conversion to halt.

    Output, int *LCHAR, the number of characters read from
    the string to form the number, including any terminating
    characters such as a trailing comma or blanks.

    Output, int *ERROR, is TRUE (1) if an error occurred and FALSE (0)
    otherwise.

    Output, double S_TO_R8, the value that was read from the string.
*/
{
  char c;
  int ihave;
  int isgn;
  int iterm;
  int jbot;
  int jsgn;
  int jtop;
  int nchar;
  int ndig;
  double r;
  double rbot;
  double rexp;
  double rtop;
  char TAB = 9;

  nchar = s_len_trim ( s );
  *error = 0;
  r = 0.0;
  *lchar = -1;
  isgn = 1;
  rtop = 0.0;
  rbot = 1.0;
  jsgn = 1;
  jtop = 0;
  jbot = 1;
  ihave = 1;
  iterm = 0;

  for ( ; ; )
  {
    c = s[*lchar+1];
    *lchar = *lchar + 1;
/*
  Blank or TAB character.
*/
    if ( c == ' ' || c == TAB )
    {
      if ( ihave == 2 )
      {
      }
      else if ( ihave == 6 || ihave == 7 )
      {
        iterm = 1;
      }
      else if ( 1 < ihave )
      {
        ihave = 11;
      }
    }
/*
  Comma.
*/
    else if ( c == ',' || c == ';' )
    {
      if ( ihave != 1 )
      {
        iterm = 1;
        ihave = 12;
        *lchar = *lchar + 1;
      }
    }
/*
  Minus sign.
*/
    else if ( c == '-' )
    {
      if ( ihave == 1 )
      {
        ihave = 2;
        isgn = -1;
      }
      else if ( ihave == 6 )
      {
        ihave = 7;
        jsgn = -1;
      }
      else
      {
        iterm = 1;
      }
    }
/*
  Plus sign.
*/
    else if ( c == '+' )
    {
      if ( ihave == 1 )
      {
        ihave = 2;
      }
      else if ( ihave == 6 )
      {
        ihave = 7;
      }
      else
      {
        iterm = 1;
      }
    }
/*
  Decimal point.
*/
    else if ( c == '.' )
    {
      if ( ihave < 4 )
      {
        ihave = 4;
      }
      else if ( 6 <= ihave && ihave <= 8 )
      {
        ihave = 9;
      }
      else
      {
        iterm = 1;
      }
    }
/*
  Exponent marker.
*/
    else if ( ch_eqi ( c, 'E' ) || ch_eqi ( c, 'D' ) )
    {
      if ( ihave < 6 )
      {
        ihave = 6;
      }
      else
      {
        iterm = 1;
      }
    }
/*
  Digit.
*/
    else if ( ihave < 11 && '0' <= c && c <= '9' )
    {
      if ( ihave <= 2 )
      {
        ihave = 3;
      }
      else if ( ihave == 4 )
      {
        ihave = 5;
      }
      else if ( ihave == 6 || ihave == 7 )
      {
        ihave = 8;
      }
      else if ( ihave == 9 )
      {
        ihave = 10;
      }

      ndig = ch_to_digit ( c );

      if ( ihave == 3 )
      {
        rtop = 10.0 * rtop + ( double ) ndig;
      }
      else if ( ihave == 5 )
      {
        rtop = 10.0 * rtop + ( double ) ndig;
        rbot = 10.0 * rbot;
      }
      else if ( ihave == 8 )
      {
        jtop = 10 * jtop + ndig;
      }
      else if ( ihave == 10 )
      {
        jtop = 10 * jtop + ndig;
        jbot = 10 * jbot;
      }
    }
/*
  Anything else is regarded as a terminator.
*/
    else
    {
      iterm = 1;
    }
/*
  If we haven't seen a terminator, and we haven't examined the
  entire string, go get the next character.
*/
    if ( iterm == 1 || nchar <= *lchar + 1 )
    {
      break;
    }

  }
/*
  If we haven't seen a terminator, and we have examined the
  entire string, then we're done, and LCHAR is equal to NCHAR.
*/
  if ( iterm != 1 && (*lchar) + 1 == nchar )
  {
    *lchar = nchar;
  }
/*
  Number seems to have terminated.  Have we got a legal number?
  Not if we terminated in states 1, 2, 6 or 7!
*/
  if ( ihave == 1 || ihave == 2 || ihave == 6 || ihave == 7 )
  {
    *error = 1;
    return r;
  }
/*
  Number seems OK.  Form it.

  We have had some trouble with input of the form 1.0E-312.
  For now, let's assume anything less than 1.0E-20 is zero.
*/
  if ( jtop == 0 )
  {
    rexp = 1.0;
  }
  else
  {
    if ( jbot == 1 )
    {
      if ( jsgn * jtop < -20 )
      {
        rexp = 0.0;
      }
      else
      {
        rexp = pow ( ( double ) 10.0, ( double ) ( jsgn * jtop ) );
      }
    }
    else
    {
      if ( jsgn * jtop < -20 * jbot )
      {
        rexp = 0.0;
      }
      else
      {
        rexp = jsgn * jtop;
        rexp = rexp / jbot;
        rexp = pow ( ( double ) 10.0, ( double ) rexp );
      }
    }

  }

  r = isgn * rexp * rtop / rbot;

  return r;
}
/******************************************************************************/

int s_to_r8vec ( char *s, int n, double rvec[] )

/******************************************************************************/
/*
  Purpose:

    S_TO_R8VEC reads an R8VEC from a string.

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    19 February 2001

  Author:

    John Burkardt

  Parameters:

    Input, char *S, the string to be read.

    Input, int N, the number of values expected.

    Output, double RVEC[N], the values read from the string.

    Output, int S_TO_R8VEC, is TRUE (1) if an error occurred and FALSE (0)
    otherwise.
*/
{
  int error;
  int i;
  int lchar;

  error = 0;

  for ( i = 0; i < n; i++ )
  {
    rvec[i] = s_to_r8 ( s, &lchar, &error );

    if ( error )
    {
      return error;
    }

    s = s + lchar;

  }

  return error;
}
/******************************************************************************/

int s_word_count ( char *s )

/******************************************************************************/
/*
  Purpose:

    S_WORD_COUNT counts the number of "words" in a string.

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    16 September 2015

  Author:

    John Burkardt

  Parameters:

    Input, char *S, the string to be examined.

    Output, int S_WORD_COUNT, the number of "words" in the string.
    Words are presumed to be separated by one or more blanks.
*/
{
  int blank;
  int word_num;
  char *t;

  word_num = 0;
  blank = 1;
  t = s;

  while ( *t )
  {
    if ( *t == ' ' || *t == '\n' )
    {
      blank = 1;
    }
    else if ( blank )
    {
      word_num = word_num + 1;
      blank = 0;
    }
    t++;
  }

  return word_num;
}
/******************************************************************************/

void timestamp ( )

/******************************************************************************/
/*
  Purpose:

    TIMESTAMP prints the current YMDHMS date as a time stamp.

  Example:

    31 May 2001 09:45:54 AM

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    24 September 2003

  Author:

    John Burkardt

  Parameters:

    None
*/
{
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );

  strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

  printf ( "%s\n", time_buffer );

  return;
# undef TIME_SIZE
}

