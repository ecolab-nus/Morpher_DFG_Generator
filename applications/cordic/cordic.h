//Cordic in 32 bit signed fixed point math
//Function is valid for arguments in range -pi/2 -- pi/2
//for values pi/2--pi: value = half_pi-(theta-half_pi) and similarly for values -pi---pi/2
//
// 1.0 = 1073741824
// 1/k = 0.6072529350088812561694
// pi = 3.1415926536897932384626
//Constants
#define cordic_1K 0x26DD3B6A
#define half_pi 0x6487ED51
#define MUL 1073741824.000000
#define CORDIC_NTAB 32
int cordic_ctab [] = {0x3243F6A8, 0x1DAC6705, 0x0FADBAFC, 0x07F56EA6, 0x03FEAB76, 0x01FFD55B, 0x00FFFAAA, 0x007FFF55, 0x003FFFEA, 0x001FFFFD, 0x000FFFFF, 0x0007FFFF, 0x0003FFFF, 0x0001FFFF, 0x0000FFFF, 0x00007FFF, 0x00003FFF, 0x00001FFF, 0x00000FFF, 0x000007FF, 0x000003FF, 0x000001FF, 0x000000FF, 0x0000007F, 0x0000003F, 0x0000001F, 0x0000000F, 0x00000008, 0x00000004, 0x00000002, 0x00000001, 0x00000000, };

int cordic(int theta, int *s, int *c, int n)
{
  int k, d, tx, ty, tz;
  int x=cordic_1K,y=0,z=theta;
  n = (n>CORDIC_NTAB) ? CORDIC_NTAB : n;
  for (k=0; k<n; k=k+9)
  {
          #ifdef CGRA_COMPILER
           please_map_me();
           #endif
    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k) ^ d) - d);
    ty = y + (((x>>k) ^ d) - d);
    tz = z - ((cordic_ctab[k] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+1) ^ d) - d);
    ty = y + (((x>>k+1) ^ d) - d);
    tz = z - ((cordic_ctab[k+1] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+2) ^ d) - d);
    ty = y + (((x>>k+2) ^ d) - d);
    tz = z - ((cordic_ctab[k+2] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+3) ^ d) - d);
    ty = y + (((x>>k+3) ^ d) - d);
    tz = z - ((cordic_ctab[k+3] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+4) ^ d) - d);
    ty = y + (((x>>k+4) ^ d) - d);
    tz = z - ((cordic_ctab[k+4] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+5) ^ d) - d);
    ty = y + (((x>>k+5) ^ d) - d);
    tz = z - ((cordic_ctab[k+5] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+6) ^ d) - d);
    ty = y + (((x>>k+6) ^ d) - d);
    tz = z - ((cordic_ctab[k+6] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+7) ^ d) - d);
    ty = y + (((x>>k+7) ^ d) - d);
    tz = z - ((cordic_ctab[k+7] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+8) ^ d) - d);
    ty = y + (((x>>k+8) ^ d) - d);
    tz = z - ((cordic_ctab[k+8] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;
  }  
 return x+y+z;
 //*c = x; *s = y;
}


int cordic_unrolled_24(int theta, int *s, int *c, int n)
{
  int k, d, tx, ty, tz;
  int x=cordic_1K,y=0,z=theta;
  n = (n>CORDIC_NTAB) ? CORDIC_NTAB : n;
  for (k=0; k<n; k=k+24)
  {
          #ifdef CGRA_COMPILER
           please_map_me();
           #endif
    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k) ^ d) - d);
    ty = y + (((x>>k) ^ d) - d);
    tz = z - ((cordic_ctab[k] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+1) ^ d) - d);
    ty = y + (((x>>k+1) ^ d) - d);
    tz = z - ((cordic_ctab[k+1] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+2) ^ d) - d);
    ty = y + (((x>>k+2) ^ d) - d);
    tz = z - ((cordic_ctab[k+2] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+3) ^ d) - d);
    ty = y + (((x>>k+3) ^ d) - d);
    tz = z - ((cordic_ctab[k+3] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+4) ^ d) - d);
    ty = y + (((x>>k+4) ^ d) - d);
    tz = z - ((cordic_ctab[k+4] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+5) ^ d) - d);
    ty = y + (((x>>k+5) ^ d) - d);
    tz = z - ((cordic_ctab[k+5] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+6) ^ d) - d);
    ty = y + (((x>>k+6) ^ d) - d);
    tz = z - ((cordic_ctab[k+6] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+7) ^ d) - d);
    ty = y + (((x>>k+7) ^ d) - d);
    tz = z - ((cordic_ctab[k+7] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+8) ^ d) - d);
    ty = y + (((x>>k+8) ^ d) - d);
    tz = z - ((cordic_ctab[k+8] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+9) ^ d) - d);
    ty = y + (((x>>k+9) ^ d) - d);
    tz = z - ((cordic_ctab[k+9] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+10) ^ d) - d);
    ty = y + (((x>>k+10) ^ d) - d);
    tz = z - ((cordic_ctab[k+10] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+11) ^ d) - d);
    ty = y + (((x>>k+11) ^ d) - d);
    tz = z - ((cordic_ctab[k+11] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+12) ^ d) - d);
    ty = y + (((x>>k+12) ^ d) - d);
    tz = z - ((cordic_ctab[k+12] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+13) ^ d) - d);
    ty = y + (((x>>k+13) ^ d) - d);
    tz = z - ((cordic_ctab[k+13] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+14) ^ d) - d);
    ty = y + (((x>>k+14) ^ d) - d);
    tz = z - ((cordic_ctab[k+14] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+15) ^ d) - d);
    ty = y + (((x>>k+15) ^ d) - d);
    tz = z - ((cordic_ctab[k+15] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+16) ^ d) - d);
    ty = y + (((x>>k+16) ^ d) - d);
    tz = z - ((cordic_ctab[k+16] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+17) ^ d) - d);
    ty = y + (((x>>k+17) ^ d) - d);
    tz = z - ((cordic_ctab[k+17] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+18) ^ d) - d);
    ty = y + (((x>>k+18) ^ d) - d);
    tz = z - ((cordic_ctab[k+18] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+19) ^ d) - d);
    ty = y + (((x>>k+19) ^ d) - d);
    tz = z - ((cordic_ctab[k+19] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+20) ^ d) - d);
    ty = y + (((x>>k+20) ^ d) - d);
    tz = z - ((cordic_ctab[k+20] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+21) ^ d) - d);
    ty = y + (((x>>k+21) ^ d) - d);
    tz = z - ((cordic_ctab[k+21] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+22) ^ d) - d);
    ty = y + (((x>>k+22) ^ d) - d);
    tz = z - ((cordic_ctab[k+22] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+23) ^ d) - d);
    ty = y + (((x>>k+23) ^ d) - d);
    tz = z - ((cordic_ctab[k+23] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;

    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k+24) ^ d) - d);
    ty = y + (((x>>k+24) ^ d) - d);
    tz = z - ((cordic_ctab[k+24] ^ d) - d);
    x = tx; 
    y = ty; 
    z = tz;
  }  
 return x+y+z;
 //*c = x; *s = y;
}
/*
original
void cordic(int theta, int *s, int *c, int n)
{
  int k, d, tx, ty, tz;
  int x=cordic_1K,y=0,z=theta;
  n = (n>CORDIC_NTAB) ? CORDIC_NTAB : n;
  for (k=0; k<n; ++k)
  {
          #ifdef CGRA_COMPILER
           please_map_me();
           #endif
    d = z>>31;
    //get sign. for other architectures, you might want to use the more portable version
    //d = z>=0 ? 0 : -1;
    tx = x - (((y>>k) ^ d) - d);
    ty = y + (((x>>k) ^ d) - d);
    tz = z - ((cordic_ctab[k] ^ d) - d);
    x = tx; y = ty; z = tz;
  }  
 *c = x; *s = y;
}
*/

