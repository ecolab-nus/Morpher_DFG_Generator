/* $Id: matrix.c,v 1.10 1997/02/10 19:47:53 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  2.2
 * Copyright (C) 1995-1997  Brian Paul
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
 * $Log: matrix.c,v $
 * Revision 1.10  1997/02/10 19:47:53  brianp
 * moved buffer resize code out of gl_Viewport() into gl_ResizeBuffersMESA()
 *
 * Revision 1.9  1997/01/31 23:32:40  brianp
 * now clear depth buffer after reallocation due to window resize
 *
 * Revision 1.8  1997/01/29 19:06:04  brianp
 * removed extra, local definition of Identity[] matrix
 *
 * Revision 1.7  1997/01/28 22:19:17  brianp
 * new matrix inversion code from Stephane Rehel
 *
 * Revision 1.6  1996/12/22 17:53:11  brianp
 * faster invert_matrix() function from scotter@lafn.org
 *
 * Revision 1.5  1996/12/02 18:58:34  brianp
 * gl_rotation_matrix() now returns identity matrix if given a 0 rotation axis
 *
 * Revision 1.4  1996/09/27 01:29:05  brianp
 * added missing default cases to switches
 *
 * Revision 1.3  1996/09/15 14:18:37  brianp
 * now use GLframebuffer and GLvisual
 *
 * Revision 1.2  1996/09/14 06:46:04  brianp
 * better matmul() from Jacques Leroy
 *
 * Revision 1.1  1996/09/13 01:38:16  brianp
 * Initial revision
 *
 */


/*
 * Matrix operations
 *
 *
 * NOTES:
 * 1. 4x4 transformation matrices are stored in memory in column major order.
 * 2. Points/vertices are to be thought of as column vectors.
 * 3. Transformation of a point p by a matrix M is: p' = M * p
 *
 */



#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "context.h"
//#include "dlist.h"
//#include "macros.h"
//#include "matrix.h"
//#include "types.h"



static int Identity[16] = {
   1.0, 0.0, 0.0, 0.0,
   0.0, 1.0, 0.0, 0.0,
   0.0, 0.0, 1.0, 0.0,
   0.0, 0.0, 0.0, 1.0
};



/*
 * Compute the inverse of a 4x4 matrix.  Contributed by scotter@lafn.org
 */
 void invert_matrix_general( const int *m, int *out )
{

/* NB. OpenGL Matrices are COLUMN major. */
#define MAT(m,r,c) (m)[(c)*4+(r)]

/* Here's some shorthand converting standard (row,column) to index. */
#define m11 MAT(m,0,0)
#define m12 MAT(m,0,1)
#define m13 MAT(m,0,2)
#define m14 MAT(m,0,3)
#define m21 MAT(m,1,0)
#define m22 MAT(m,1,1)
#define m23 MAT(m,1,2)
#define m24 MAT(m,1,3)
#define m31 MAT(m,2,0)
#define m32 MAT(m,2,1)
#define m33 MAT(m,2,2)
#define m34 MAT(m,2,3)
#define m41 MAT(m,3,0)
#define m42 MAT(m,3,1)
#define m43 MAT(m,3,2)
#define m44 MAT(m,3,3)

   int det;
   int d12, d13, d23, d24, d34, d41, tmp0, tmp1, tmp2,tmp3;
   int tmp[160]; /* Allow out == in. */

   /* Inverse = adjoint / det. (See linear algebra texts.)*/

   /* pre-compute 2x2 dets for last two rows when computing */
   /* cofactors of first two rows. */
   for (int i= 0 ; i<160;i=i+16){

           #ifdef CGRA_COMPILER
           please_map_me();
           #endif
        //    i= i+10;
        // }
        // int i = 0;
   d12 = (m31*m42-m41*m32);
   d13 = (m31*m43-m41*m33);
   d23 = (m32*m43-m42*m33);
   d24 = (m32*m44-m42*m34);
   d34 = (m33*m44-m43*m34);
   d41 = (m34*m41-m44*m31);

   tmp0 =  (m22 * d34 - m23 * d24 + m24 * d23);
   tmp1 = -(m21 * d34 + m23 * d41 + m24 * d13);
   tmp2 =  (m21 * d24 + m22 * d41 + m24 * d12);
   tmp3 = -(m21 * d23 - m22 * d13 + m23 * d12);

   /* Compute determinant as early as possible using these cofactors. */
   det = m11 * tmp0 + m12 * tmp1 + m13 * tmp2 + m14 * tmp3;

   /* Run singularity test. */
  // if (det == 0.0F) {
      /* printf("invert_matrix: Warning: Singular matrix.\n"); */
    //  MEMCPY( out, Identity, 16*sizeof(int) );
 //  }
 //  else {
      int invDet = 1.0F / det;
      /* Compute rest of inverse. */
      tmp0 *= invDet;
      tmp1 *= invDet;
      tmp2 *= invDet;
      tmp3 *= invDet;

      tmp[4+i] = -(m12 * d34 - m13 * d24 + m14 * d23) * invDet;
      tmp[5+i] =  (m11 * d34 + m13 * d41 + m14 * d13) * invDet;
      tmp[6+i] = -(m11 * d24 + m12 * d41 + m14 * d12) * invDet;
      tmp[7+i] =  (m11 * d23 - m12 * d13 + m13 * d12) * invDet;

      /* Pre-compute 2x2 dets for first two rows when computing */
      /* cofactors of last two rows. */
      d12 = m11*m22-m21*m12;
      d13 = m11*m23-m21*m13;
      d23 = m12*m23-m22*m13;
      d24 = m12*m24-m22*m14;
      d34 = m13*m24-m23*m14;
      d41 = m14*m21-m24*m11;

      tmp[8+i] =  (m42 * d34 - m43 * d24 + m44 * d23) * invDet;
      tmp[9+i] = -(m41 * d34 + m43 * d41 + m44 * d13) * invDet;
      tmp[10+i] =  (m41 * d24 + m42 * d41 + m44 * d12) * invDet;
      tmp[11+i] = -(m41 * d23 - m42 * d13 + m43 * d12) * invDet;
      tmp[12+i] = -(m32 * d34 - m33 * d24 + m34 * d23) * invDet;
      tmp[13+i] =  (m31 * d34 + m33 * d41 + m34 * d13) * invDet;
      tmp[14+i] = -(m31 * d24 + m32 * d41 + m34 * d12) * invDet;
      tmp[15+i] =  (m31 * d23 - m32 * d13 + m33 * d12) * invDet;
      tmp[0+i]=tmp0;
      tmp[1+i]=tmp1;
      tmp[2+i]=tmp2;
      tmp[3+i]=tmp3;

     // MEMCPY(out, tmp, 16*sizeof(int));
  }
//}
printf("%d\n", tmp[15]);
#undef m11
#undef m12
#undef m13
#undef m14
#undef m21
#undef m22
#undef m23
#undef m24
#undef m31
#undef m32
#undef m33
#undef m34
#undef m41
#undef m42
#undef m43
#undef m44
#undef MAT
}

/*
 * Compute the inverse of a 4x4 matrix.  Contributed by scotter@lafn.org
 */
 void invert_matrix_general_unrolled( const int *m, int *out )
{

/* NB. OpenGL Matrices are COLUMN major. */
#define MAT(m,r,c) (m)[(c)*4+(r)]

/* Here's some shorthand converting standard (row,column) to index. */
#define m11 MAT(m,0,0)
#define m12 MAT(m,0,1)
#define m13 MAT(m,0,2)
#define m14 MAT(m,0,3)
#define m21 MAT(m,1,0)
#define m22 MAT(m,1,1)
#define m23 MAT(m,1,2)
#define m24 MAT(m,1,3)
#define m31 MAT(m,2,0)
#define m32 MAT(m,2,1)
#define m33 MAT(m,2,2)
#define m34 MAT(m,2,3)
#define m41 MAT(m,3,0)
#define m42 MAT(m,3,1)
#define m43 MAT(m,3,2)
#define m44 MAT(m,3,3)

#define m11_ MAT(m,0,0)
#define m12_ MAT(m,0,1)
#define m13_ MAT(m,0,2)
#define m14_ MAT(m,0,3)
#define m21_ MAT(m,1,0)
#define m22_ MAT(m,1,1)
#define m23_ MAT(m,1,2)
#define m24_ MAT(m,1,3)
#define m31_ MAT(m,2,0)
#define m32_ MAT(m,2,1)
#define m33_ MAT(m,2,2)
#define m34_ MAT(m,2,3)
#define m41_ MAT(m,3,0)
#define m42_ MAT(m,3,1)
#define m43_ MAT(m,3,2)
#define m44_ MAT(m,3,3)

   int det;
   int d12, d13, d23, d24, d34, d41, tmp0, tmp1, tmp2,tmp3;
   int d12_, d13_, d23_, d24_, d34_, d41_, tmp0_, tmp1_, tmp2_,tmp3_;
   int tmp[160]; /* Allow out == in. */

   /* Inverse = adjoint / det. (See linear algebra texts.)*/

   /* pre-compute 2x2 dets for last two rows when computing */
   /* cofactors of first two rows. */
   for (int i= 0 ; i<160;i=i+32){

           #ifdef CGRA_COMPILER
           please_map_me();
           #endif
        //    i= i+10;
        // }
        // int i = 0;
   d12 = (m31*m42-m41*m32);
   d13 = (m31*m43-m41*m33);
   d23 = (m32*m43-m42*m33);
   d24 = (m32*m44-m42*m34);
   d34 = (m33*m44-m43*m34);
   d41 = (m34*m41-m44*m31);

   tmp0 =  (m22 * d34 - m23 * d24 + m24 * d23);
   tmp1 = -(m21 * d34 + m23 * d41 + m24 * d13);
   tmp2 =  (m21 * d24 + m22 * d41 + m24 * d12);
   tmp3 = -(m21 * d23 - m22 * d13 + m23 * d12);

   /* Compute determinant as early as possible using these cofactors. */
   det = m11 * tmp0 + m12 * tmp1 + m13 * tmp2 + m14 * tmp3;

   /* Run singularity test. */
  // if (det == 0.0F) {
      /* printf("invert_matrix: Warning: Singular matrix.\n"); */
    //  MEMCPY( out, Identity, 16*sizeof(int) );
 //  }
 //  else {
      int invDet = 1.0F / det;
      /* Compute rest of inverse. */
      tmp0 *= invDet;
      tmp1 *= invDet;
      tmp2 *= invDet;
      tmp3 *= invDet;

      tmp[4+i] = -(m12 * d34 - m13 * d24 + m14 * d23) * invDet;
      tmp[5+i] =  (m11 * d34 + m13 * d41 + m14 * d13) * invDet;
      tmp[6+i] = -(m11 * d24 + m12 * d41 + m14 * d12) * invDet;
      tmp[7+i] =  (m11 * d23 - m12 * d13 + m13 * d12) * invDet;

      /* Pre-compute 2x2 dets for first two rows when computing */
      /* cofactors of last two rows. */
      d12 = m11*m22-m21*m12;
      d13 = m11*m23-m21*m13;
      d23 = m12*m23-m22*m13;
      d24 = m12*m24-m22*m14;
      d34 = m13*m24-m23*m14;
      d41 = m14*m21-m24*m11;

      tmp[8+i] =  (m42 * d34 - m43 * d24 + m44 * d23) * invDet;
      tmp[9+i] = -(m41 * d34 + m43 * d41 + m44 * d13) * invDet;
      tmp[10+i] =  (m41 * d24 + m42 * d41 + m44 * d12) * invDet;
      tmp[11+i] = -(m41 * d23 - m42 * d13 + m43 * d12) * invDet;
      tmp[12+i] = -(m32 * d34 - m33 * d24 + m34 * d23) * invDet;
      tmp[13+i] =  (m31 * d34 + m33 * d41 + m34 * d13) * invDet;
      tmp[14+i] = -(m31 * d24 + m32 * d41 + m34 * d12) * invDet;
      tmp[15+i] =  (m31 * d23 - m32 * d13 + m33 * d12) * invDet;
      tmp[0+i]=tmp0;
      tmp[1+i]=tmp1;
      tmp[2+i]=tmp2;
      tmp[3+i]=tmp3;
      //i=i+16;

            // int i = 0;
   d12_ = (m31_*m42_-m41_*m32_);
   d13_ = (m31_*m43_-m41_*m33_);
   d23_ = (m32_*m43_-m42_*m33_);
   d24_ = (m32_*m44_-m42_*m34_);
   d34_ = (m33_*m44_-m43_*m34_);
   d41_ = (m34_*m41_-m44_*m31_);

   tmp0_ =  (m22_ * d34 - m23_ * d24 + m24_ * d23);
   tmp1_ = -(m21_ * d34 + m23_ * d41 + m24_ * d13);
   tmp2_ =  (m21_ * d24 + m22_ * d41 + m24_ * d12);
   tmp3_ = -(m21_ * d23 - m22_ * d13 + m23_ * d12);

   /* Compute determinant as early as possible using these cofactors. */
   det = m11_ * tmp0 + m12_ * tmp1 + m13_ * tmp2 + m14_ * tmp3;

   /* Run singularity test. */
  // if (det == 0.0F) {
      /* printf("invert_matrix: Warning: Singular matrix.\n"); */
    //  MEMCPY( out, Identity, 16*sizeof(int) );
 //  }
 //  else {
       invDet = 1.0F / det;
      /* Compute rest of inverse. */
      tmp0_ *= invDet;
      tmp1_ *= invDet;
      tmp2_ *= invDet;
      tmp3_ *= invDet;

      tmp[4+16+i] = -(m12_ * d34 - m13_ * d24 + m14_ * d23) * invDet;
      tmp[5+16+i] =  (m11_ * d34 + m13_ * d41 + m14_ * d13) * invDet;
      tmp[6+16+i] = -(m11_ * d24 + m12_ * d41 + m14_ * d12) * invDet;
      tmp[7+16+i] =  (m11_ * d23 - m12_ * d13 + m13_ * d12) * invDet;

      /* Pre-compute 2x2 dets for first two rows when computing */
      /* cofactors of last two rows. */
      d12_ = m11_*m22_-m21_*m12_;
      d13_ = m11_*m23_-m21_*m13_;
      d23_ = m12_*m23_-m22_*m13_;
      d24_ = m12_*m24_-m22_*m14_;
      d34_ = m13_*m24_-m23_*m14_;
      d41_ = m14_*m21_-m24_*m11_;

      tmp[8+16+i] =  (m42_ * d34_ - m43_ * d24_ + m44_ * d23_) * invDet;
      tmp[9+16+i] = -(m41_ * d34_ + m43_ * d41_ + m44_ * d13_) * invDet;
      tmp[10+16+i] =  (m41_ * d24_ + m42_ * d41_ + m44_ * d12_) * invDet;
      tmp[11+16+i] = -(m41_ * d23_ - m42_ * d13_ + m43_ * d12_) * invDet;
      tmp[12+16+i] = -(m32_ * d34_ - m33_ * d24_ + m34_ * d23_) * invDet;
      tmp[13+16+i] =  (m31_ * d34_ + m33_ * d41_ + m34_ * d13_) * invDet;
      tmp[14+16+i] = -(m31_ * d24_ + m32_ * d41_ + m34_ * d12_) * invDet;
      tmp[15+16+i] =  (m31_ * d23_ - m32_ * d13_ + m33_ * d12_) * invDet;
      tmp[0+16+i]=tmp0_;
      tmp[1+16+i]=tmp1_;
      tmp[2+16+i]=tmp2_;
      tmp[3+16+i]=tmp3_;

     // MEMCPY(out, tmp, 16*sizeof(int));
  }
//}
printf("%d\n", tmp[15]);
#undef m11
#undef m12
#undef m13
#undef m14
#undef m21
#undef m22
#undef m23
#undef m24
#undef m31
#undef m32
#undef m33
#undef m34
#undef m41
#undef m42
#undef m43
#undef m44
#undef MAT
}


/*
 * Compute the inverse of a 4x4 matrix.  Contributed by scotter@lafn.org
 */
 void invert_matrix_general_more_unrolled( const int *m, int *out )
{

/* NB. OpenGL Matrices are COLUMN major. */
#define MAT(m,r,c) (m)[(c)*4+(r)]

/* Here's some shorthand converting standard (row,column) to index. */
#define m11 MAT(m,0,0)
#define m12 MAT(m,0,1)
#define m13 MAT(m,0,2)
#define m14 MAT(m,0,3)
#define m21 MAT(m,1,0)
#define m22 MAT(m,1,1)
#define m23 MAT(m,1,2)
#define m24 MAT(m,1,3)
#define m31 MAT(m,2,0)
#define m32 MAT(m,2,1)
#define m33 MAT(m,2,2)
#define m34 MAT(m,2,3)
#define m41 MAT(m,3,0)
#define m42 MAT(m,3,1)
#define m43 MAT(m,3,2)
#define m44 MAT(m,3,3)

#define m11_ MAT(m,0,0)
#define m12_ MAT(m,0,1)
#define m13_ MAT(m,0,2)
#define m14_ MAT(m,0,3)
#define m21_ MAT(m,1,0)
#define m22_ MAT(m,1,1)
#define m23_ MAT(m,1,2)
#define m24_ MAT(m,1,3)
#define m31_ MAT(m,2,0)
#define m32_ MAT(m,2,1)
#define m33_ MAT(m,2,2)
#define m34_ MAT(m,2,3)
#define m41_ MAT(m,3,0)
#define m42_ MAT(m,3,1)
#define m43_ MAT(m,3,2)
#define m44_ MAT(m,3,3)

#define m11__ MAT(m,0,0)
#define m12__ MAT(m,0,1)
#define m13__ MAT(m,0,2)
#define m14__ MAT(m,0,3)
#define m21__ MAT(m,1,0)
#define m22__ MAT(m,1,1)
#define m23__ MAT(m,1,2)
#define m24__ MAT(m,1,3)
#define m31__ MAT(m,2,0)
#define m32__ MAT(m,2,1)
#define m33__ MAT(m,2,2)
#define m34__ MAT(m,2,3)
#define m41__ MAT(m,3,0)
#define m42__ MAT(m,3,1)
#define m43__ MAT(m,3,2)
#define m44__ MAT(m,3,3)

   int det;
   int d12, d13, d23, d24, d34, d41, tmp0, tmp1, tmp2,tmp3;
   int d12_, d13_, d23_, d24_, d34_, d41_, tmp0_, tmp1_, tmp2_,tmp3_;
   int d12__, d13__, d23__, d24__, d34__, d41__, tmp0__, tmp1__, tmp2__,tmp3__;
   int tmp[160]; /* Allow out == in. */

   /* Inverse = adjoint / det. (See linear algebra texts.)*/

   /* pre-compute 2x2 dets for last two rows when computing */
   /* cofactors of first two rows. */
   for (int i= 0 ; i<160;i=i+48){

           #ifdef CGRA_COMPILER
           please_map_me();
           #endif
        //    i= i+10;
        // }
        // int i = 0;
   d12 = (m31*m42-m41*m32);
   d13 = (m31*m43-m41*m33);
   d23 = (m32*m43-m42*m33);
   d24 = (m32*m44-m42*m34);
   d34 = (m33*m44-m43*m34);
   d41 = (m34*m41-m44*m31);

   tmp0 =  (m22 * d34 - m23 * d24 + m24 * d23);
   tmp1 = -(m21 * d34 + m23 * d41 + m24 * d13);
   tmp2 =  (m21 * d24 + m22 * d41 + m24 * d12);
   tmp3 = -(m21 * d23 - m22 * d13 + m23 * d12);

   /* Compute determinant as early as possible using these cofactors. */
   det = m11 * tmp0 + m12 * tmp1 + m13 * tmp2 + m14 * tmp3;

   /* Run singularity test. */
  // if (det == 0.0F) {
      /* printf("invert_matrix: Warning: Singular matrix.\n"); */
    //  MEMCPY( out, Identity, 16*sizeof(int) );
 //  }
 //  else {
      int invDet = 1.0F / det;
      /* Compute rest of inverse. */
      tmp0 *= invDet;
      tmp1 *= invDet;
      tmp2 *= invDet;
      tmp3 *= invDet;

      tmp[4+i] = -(m12 * d34 - m13 * d24 + m14 * d23) * invDet;
      tmp[5+i] =  (m11 * d34 + m13 * d41 + m14 * d13) * invDet;
      tmp[6+i] = -(m11 * d24 + m12 * d41 + m14 * d12) * invDet;
      tmp[7+i] =  (m11 * d23 - m12 * d13 + m13 * d12) * invDet;

      /* Pre-compute 2x2 dets for first two rows when computing */
      /* cofactors of last two rows. */
      d12 = m11*m22-m21*m12;
      d13 = m11*m23-m21*m13;
      d23 = m12*m23-m22*m13;
      d24 = m12*m24-m22*m14;
      d34 = m13*m24-m23*m14;
      d41 = m14*m21-m24*m11;

      tmp[8+i] =  (m42 * d34 - m43 * d24 + m44 * d23) * invDet;
      tmp[9+i] = -(m41 * d34 + m43 * d41 + m44 * d13) * invDet;
      tmp[10+i] =  (m41 * d24 + m42 * d41 + m44 * d12) * invDet;
      tmp[11+i] = -(m41 * d23 - m42 * d13 + m43 * d12) * invDet;
      tmp[12+i] = -(m32 * d34 - m33 * d24 + m34 * d23) * invDet;
      tmp[13+i] =  (m31 * d34 + m33 * d41 + m34 * d13) * invDet;
      tmp[14+i] = -(m31 * d24 + m32 * d41 + m34 * d12) * invDet;
      tmp[15+i] =  (m31 * d23 - m32 * d13 + m33 * d12) * invDet;
      tmp[0+i]=tmp0;
      tmp[1+i]=tmp1;
      tmp[2+i]=tmp2;
      tmp[3+i]=tmp3;
      //i=i+16;

            // int i = 0;
   d12_ = (m31_*m42_-m41_*m32_);
   d13_ = (m31_*m43_-m41_*m33_);
   d23_ = (m32_*m43_-m42_*m33_);
   d24_ = (m32_*m44_-m42_*m34_);
   d34_ = (m33_*m44_-m43_*m34_);
   d41_ = (m34_*m41_-m44_*m31_);

   tmp0_ =  (m22_ * d34_ - m23_ * d24_ + m24_ * d23_);
   tmp1_ = -(m21_ * d34_ + m23_ * d41_ + m24_ * d13_);
   tmp2_ =  (m21_ * d24_ + m22_ * d41_ + m24_ * d12_);
   tmp3_ = -(m21_ * d23_ - m22_ * d13_ + m23_ * d12_);

   /* Compute determinant as early as possible using these cofactors. */
   det = m11_ * tmp0 + m12_ * tmp1 + m13_ * tmp2 + m14_ * tmp3;

   /* Run singularity test. */
  // if (det == 0.0F) {
      /* printf("invert_matrix: Warning: Singular matrix.\n"); */
    //  MEMCPY( out, Identity, 16*sizeof(int) );
 //  }
 //  else {
       invDet = 1.0F / det;
      /* Compute rest of inverse. */
      tmp0_ *= invDet;
      tmp1_ *= invDet;
      tmp2_ *= invDet;
      tmp3_ *= invDet;

      tmp[4+16+i] = -(m12_ * d34_ - m13_ * d24_ + m14_ * d23_) * invDet;
      tmp[5+16+i] =  (m11_ * d34_ + m13_ * d41_ + m14_ * d13_) * invDet;
      tmp[6+16+i] = -(m11_ * d24_ + m12_ * d41_ + m14_ * d12_) * invDet;
      tmp[7+16+i] =  (m11_ * d23_ - m12_ * d13_ + m13_ * d12_) * invDet;

      /* Pre-compute 2x2 dets for first two rows when computing */
      /* cofactors of last two rows. */
      d12_ = m11_*m22_-m21_*m12_;
      d13_ = m11_*m23_-m21_*m13_;
      d23_ = m12_*m23_-m22_*m13_;
      d24_ = m12_*m24_-m22_*m14_;
      d34_ = m13_*m24_-m23_*m14_;
      d41_ = m14_*m21_-m24_*m11_;

      tmp[8+16+i] =  (m42_ * d34_ - m43_ * d24_ + m44_ * d23_) * invDet;
      tmp[9+16+i] = -(m41_ * d34_ + m43_ * d41_ + m44_ * d13_) * invDet;
      tmp[10+16+i] =  (m41_ * d24_ + m42_ * d41_ + m44_ * d12_) * invDet;
      tmp[11+16+i] = -(m41_ * d23_ - m42_ * d13_ + m43_ * d12_) * invDet;
      tmp[12+16+i] = -(m32_ * d34_ - m33_ * d24_ + m34_ * d23_) * invDet;
      tmp[13+16+i] =  (m31_ * d34_ + m33_ * d41_ + m34_ * d13_) * invDet;
      tmp[14+16+i] = -(m31_ * d24_ + m32_ * d41_ + m34_ * d12_) * invDet;
      tmp[15+16+i] =  (m31_ * d23_ - m32_ * d13_ + m33_ * d12_) * invDet;
      tmp[0+16+i]=tmp0_;
      tmp[1+16+i]=tmp1_;
      tmp[2+16+i]=tmp2_;
      tmp[3+16+i]=tmp3_;



                // int i = 0;
   d12__ = (m31__*m42__-m41__*m32__);
   d13__ = (m31__*m43__-m41__*m33__);
   d23__ = (m32__*m43__-m42__*m33__);
   d24__ = (m32__*m44__-m42__*m34__);
   d34__ = (m33__*m44__-m43__*m34__);
   d41__ = (m34__*m41__-m44__*m31__);

   tmp0__ =  (m22__ * d34__ - m23__ * d24__ + m24__ * d23__);
   tmp1__ = -(m21__ * d34__ + m23__ * d41__ + m24__ * d13__);
   tmp2__ =  (m21__ * d24__ + m22__ * d41__ + m24__ * d12__);
   tmp3__ = -(m21__ * d23__ - m22__ * d13__ + m23__ * d12__);

   /* Compute determinant as early as possible using these cofactors. */
   det = m11__ * tmp0 + m12__ * tmp1 + m13__ * tmp2 + m14__ * tmp3;

   /* Run singularity test. */
  // if (det == 0.0F) {
      /* printf("invert__matrix: Warning: Singular matrix.\n"); */
    //  MEMCPY( out, Identity, 16*sizeof(int) );
 //  }
 //  else {
       invDet = 1.0F / det;
      /* Compute rest of inverse. */
      tmp0__ *= invDet;
      tmp1__ *= invDet;
      tmp2__ *= invDet;
      tmp3__ *= invDet;

      tmp[4+32+i] = -(m12__ * d34__ - m13__ * d24__ + m14__ * d23__) * invDet;
      tmp[5+32+i] =  (m11__ * d34__ + m13__ * d41__ + m14__ * d13__) * invDet;
      tmp[6+32+i] = -(m11__ * d24__ + m12__ * d41__ + m14__ * d12__) * invDet;
      tmp[7+32+i] =  (m11__ * d23__ - m12__ * d13__ + m13__ * d12__) * invDet;

      /* Pre-compute 2x2 dets for first two rows when computing */
      /* cofactors of last two rows. */
      d12__ = m11__*m22__-m21__*m12__;
      d13__ = m11__*m23__-m21__*m13__;
      d23__ = m12__*m23__-m22__*m13__;
      d24__ = m12__*m24__-m22__*m14__;
      d34__ = m13__*m24__-m23__*m14__;
      d41__ = m14__*m21__-m24__*m11__;

      tmp[8+32+i] =  (m42__ * d34__ - m43__ * d24__ + m44__ * d23__) * invDet;
      tmp[9+32+i] = -(m41__ * d34__ + m43__ * d41__ + m44__ * d13__) * invDet;
      tmp[10+32+i] =  (m41__ * d24__ + m42__ * d41__ + m44__ * d12__) * invDet;
      tmp[11+32+i] = -(m41__ * d23__ - m42__ * d13__ + m43__ * d12__) * invDet;
      tmp[12+32+i] = -(m32__ * d34__ - m33__ * d24__ + m34__ * d23__) * invDet;
      tmp[13+32+i] =  (m31__ * d34__ + m33__ * d41__ + m34__ * d13__) * invDet;
      tmp[14+32+i] = -(m31__ * d24__ + m32__ * d41__ + m34__ * d12__) * invDet;
      tmp[15+32+i] =  (m31__ * d23__ - m32__ * d13__ + m33__ * d12__) * invDet;
      tmp[0+32+i]=tmp0__;
      tmp[1+32+i]=tmp1__;
      tmp[2+32+i]=tmp2__;
      tmp[3+32+i]=tmp3__;

     // MEMCPY(out, tmp, 16*sizeof(int));
  }
//}
printf("%d\n", tmp[15]);
#undef m11
#undef m12
#undef m13
#undef m14
#undef m21
#undef m22
#undef m23
#undef m24
#undef m31
#undef m32
#undef m33
#undef m34
#undef m41
#undef m42
#undef m43
#undef m44
#undef MAT
}