//=====================================================================================================
// MadgwickAHRS.c
//=====================================================================================================
//
// Implementation of Madgwick's IMU and AHRS algorithms.
// See: http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms
//
// Date			Author          Notes
// 29/09/2011	SOH Madgwick    Initial release
// 02/10/2011	SOH Madgwick	Optimised for reduced CPU load
// 19/02/2012	SOH Madgwick	Magnetometer measurement is normalised
//
//=====================================================================================================

//---------------------------------------------------------------------------------------------------
// Header files
#include<stdio.h>
#include "MadgwickAHRS.h"
#include <math.h>
#include<stdlib.h>

// for test
#include "stdio.h"
#include<string.h>

//---------------------------------------------------------------------------------------------------
// Definitions

#define sampleFreq	512	// sample frequency in Hz
#define betaDef		0.1f		// 2 * proportional gain
#define shift_bits 9
#define sft 15

#define mul_fr32_c(inArg0, inArg1)\
({\
__int32_t A = (inArg0 >> 16); \
__int32_t C = (inArg1 >> 16); \
__uint32_t B = (inArg0 & 0xFFFF); \
__uint32_t D = (inArg1 & 0xFFFF); \
__int32_t AC = A*C; \
__int32_t AD_CB = A*D + C*B; \
__uint32_t BD = B*D; \
__int32_t product_hi = AC + (AD_CB >> 16); \
__uint32_t ad_cb_temp = AD_CB << 16; \
__uint32_t product_lo = BD + ad_cb_temp; \
if (product_lo < BD) \
		product_hi++; \
__int32_t result = (product_hi << (32-sft)) | (product_lo >> sft); \
result; \
})

#define mul_fr32(inArg0, inArg1)\
({\
__int32_t result = (inArg0 * inArg1) >> sft; \
result; \
})

#define rshift_fr32(inArg)\
({\
__int32_t result = inArg >> sft; \
result; \
})

//---------------------------------------------------------------------------------------------------
// Variable definitions

volatile float beta = betaDef;								// 2 * proportional gain (Kp)
//volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;	// quaternion of sensor frame relative to auxiliary frame
volatile float q_0 = 1.0f, q_1 = 0.0f, q_2 = 0.0f, q_3 = 0.0f;	// quaternion of sensor frame relative to auxiliary frame
volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;	
// volatile int shift_bits = 9;   // log2(sampleFreq)
//---------------------------------------------------------------------------------------------------
// Function declarations

float invSqrt(float x);

//====================================================================================================
// Functions

__int32_t float_to_fr32(float real){
	return ((__int32_t)((1U << sft) * (real)));
}

float fr32_to_float(__int32_t fr32){
	return ((float)fr32 * 1.0F / (1U << sft));
}


// //---------------------------------------------------------------------------------------------------
// // Fast inverse square-root
// // See: http://en.wikipedia.org/wiki/Fast_inverse_square_root

float invSqrt(float x) {
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));
	return fabs(y);
}

// int FastDistance2D(int x, int y)
// {
//         x = abs(x);
//         y = abs(y);
//         int mn = x+(((y-x)>>(31))&(y-x)); 
//         return(x+y-(mn>>1)-(mn>>2)+(mn>>4));
// }

int MadgwickAHRSupdateIMU(float *gxf, float *gyf, float *gzf, float *axf, float *ayf, float *azf, int length ) {
	float recipNorm;
	int _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 , _4q3, _8q1, _8q2, q0q0, q1q1, q2q2, q3q3; 
	int qDot1, qDot2, qDot3, qDot4;
	int s0, s1, s2, s3;
	int gx[length], gy[length], gz[length], ax[length], ay[length], az[length];
	int reverse_1 = 0x55555555;
	int reverse_2 = 0x33333333; 
	int reverse_3 = 0x0F0F0F0F;
	int reverse_4 = 0x00FF00FF;
	int mediate = 10;
	// unsigned int logical_number = 0x077CB531U;
	// static const int MultiplyDeBruijnBitPosition[32] = {0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9};
	unsigned int logical_number = 0x07C4ACDDU;
	static int MultiplyDeBruijnBitPosition[32] = {0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30, 8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31};
	for(int i=0; i<32; i++){
		MultiplyDeBruijnBitPosition[i] = MultiplyDeBruijnBitPosition[i] - sft;
	}
	// MultiplyDeBruijnBitPosition = MultiplyDeBruijnBitPosition - sft;
	int q0_int = float_to_fr32(q0);
	int q1_int = float_to_fr32(q1);
	int q2_int = float_to_fr32(q2);
	int q3_int = float_to_fr32(q3);

	for(int iter=0; iter<length; iter++){
		recipNorm = invSqrt(axf[iter] * axf[iter] + ayf[iter] * ayf[iter] + azf[iter] * azf[iter]);
		axf[iter] *= recipNorm;
		ayf[iter] *= recipNorm;
		azf[iter] *= recipNorm;   
		ax[iter] = float_to_fr32(axf[iter]);
		ay[iter] = float_to_fr32(ayf[iter]);
		az[iter] = float_to_fr32(azf[iter]);

		gxf[iter] *= 0.0174533f * 5;
		gyf[iter] *= 0.0174533f * 5;
		gzf[iter] *= 0.0174533f * 5;
		gx[iter] = (int)gxf[iter];
		gy[iter] = (int)gyf[iter];
		gz[iter] = (int)gzf[iter];
	}

	for(int i=0; i<length; i++){

		#ifdef CGRA_COMPILER
		please_map_me();
		#endif		

		int recipNorm_second;
		// Rate of change of quaternion from gyroscope
		// qDot1 = (-q1_int * gx[i] - q2_int * gy[i] - q3_int * gz[i]) * 5;
		// qDot2 = (q0_int * gx[i] + q2_int * gz[i] - q3_int * gy[i]) * 5;
		// qDot3 = (q0_int * gy[i] - q1_int * gz[i] + q3_int * gx[i]) * 5;
		// qDot4 = (q0_int * gz[i] + q1_int * gy[i] - q2_int * gx[i]) * 5;
		qDot1 = -q1_int * gx[i] - q2_int * gy[i] - q3_int * gz[i];
		qDot2 = q0_int * gx[i] + q2_int * gz[i] - q3_int * gy[i];
		qDot3 = q0_int * gy[i] - q1_int * gz[i] + q3_int * gx[i];
		qDot4 = q0_int * gz[i] + q1_int * gy[i] - q2_int * gx[i];

		// Auxiliary variables to avoid repeated arithmetic
		_2q0 = 2 * q0_int;
		_2q1 = 2 * q1_int;
		_2q2 = 2 * q2_int;
		_2q3 = 2 * q3_int;
		_4q0 = 4 * q0_int;
		_4q1 = 4 * q1_int;
		_4q2 = 4 * q2_int;
		_4q3 = 4 * q3_int;
		_8q1 = 8 * q1_int;
		_8q2 = 8 * q2_int;
		q0q0 = mul_fr32(q0_int, q0_int);
		q1q1 = mul_fr32(q1_int, q1_int);
		q2q2 = mul_fr32(q2_int, q2_int);
		q3q3 = mul_fr32(q3_int, q3_int);

	// // Gradient decent algorithm corrective step
	// 	s0 = mul_fr32(_4q0, q2q2)  + mul_fr32(_2q2, ax[i]) + rshift_fr32(_4q0 * q1q1 - _2q1 * ay[i]);
	// 	s1 = rshift_fr32(_4q1 * q3q3 - _2q3 * ax[i] ) + rshift_fr32(q0q0 * _4q1 - _2q0 * ay[i]) - _4q1 + mul_fr32(_8q1, q1q1) + mul_fr32(_8q1, q2q2) + mul_fr32(_4q1, az[i]);
	// 	s2 = mul_fr32(q0q0, _4q2) + mul_fr32(_2q0, ax[i]) + rshift_fr32(_4q2 * q3q3 - _2q3 * ay[i])  - _4q2 + mul_fr32(_8q2, q1q1) + mul_fr32(_8q2, q2q2) + mul_fr32(_4q2, az[i]);
	// 	s3 = rshift_fr32(q1q1 * _4q3 - _2q1 * ax[i]) + rshift_fr32(q2q2 * _4q3 - _2q2 * ay[i]);
		// s0 = 2 * q0_int * q2q2  + q2_int * ax[i] + 2 * q0_int * q1q1 - q1_int * ay[i];
		// s1 = 2 * q1_int * q3q3 - q3_int * ax[i]  + q0q0 * 2 * q1_int - q0_int * ay[i] - 2 * q1_int + 4 * q1_int * q1q1 + 4 * q1_int * q2q2 + 2 * q1_int * az[i];
		// s2 = q0q0 * 2 * q2_int + q0_int * ax[i] + 2 * q2_int * q3q3 - q3_int * ay[i]  - 2 * q2_int + 4 * q2_int * q1q1 + 4 * q2_int * q2q2 + 2 * q2_int * az[i];
		// s3 = q1q1 * 2 * q3_int - q1_int * ax[i] + q2q2 * 2 * q3_int - q2_int * ay[i];

		s0 = _4q0 * q2q2 + _2q2 * ax[i] + _4q0 * q1q1 - _2q1 * ay[i];
		s1 = _4q1 * q3q3 - _2q3 * ax[i] + q0q0 * _4q1 - _2q0 * ay[i] - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az[i];
		s2 = q0q0 * _4q2 + _2q0 * ax[i] + _4q2 * q3q3 - _2q3 * ay[i] - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az[i];
		s3 = q1q1 * _4q3 - _2q1 * ax[i] + q2q2 * _4q3 - _2q2 * ay[i];

		s0 >>= sft;
		s1 >>= sft;
		s2 >>= sft;
		s3 >>= sft;
		// normalise step magnitude
		// recipNorm_second = FastDistance2D(FastDistance2D(s0, s1), FastDistance2D(s2, s3));
		int abs_s0 = ((s0^(s0>>31))-(s0>>31));
		int abs_s1 = ((s1^(s1>>31))-(s1>>31));
		int min_s = abs_s0+(((abs_s1-abs_s0)>>(31))&(abs_s1-abs_s0)); 
		int abs_ss0 = (abs_s0+abs_s1-(min_s>>1)-(min_s>>2)+(min_s>>4));

		int abs_s2 = ((s2^(s2>>31))-(s2>>31));
		int abs_s3 = ((s3^(s3>>31))-(s3>>31));	
		int min_ss = abs_s2+(((abs_s3-abs_s2)>>(31))&(abs_s3-abs_s2)); 
		int abs_ss1 = (abs_s2+abs_s3-(min_ss>>1)-(min_ss>>2)+(min_ss>>4));

		int min_sss = abs_ss0+(((abs_ss1-abs_ss0)>>(31))&(abs_ss1-abs_ss0)); 
		recipNorm_second = (abs_ss0+abs_ss1-(min_sss>>1)-(min_sss>>2)+(min_sss>>4));
		// recipNorm_second = s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3;

		// //Binary reverse
		// recipNorm_second =((recipNorm_second >>1)&reverse_1)|((recipNorm_second&reverse_1)<<1);
		// recipNorm_second =((recipNorm_second >>2)&reverse_2)|((recipNorm_second&reverse_2)<<2);
		// recipNorm_second =((recipNorm_second >>4)&reverse_3)|((recipNorm_second&reverse_3)<<4);
		// recipNorm_second =((recipNorm_second >>8)&reverse_4)|((recipNorm_second&reverse_4)<<8);
		// recipNorm_second =(recipNorm_second >>16)|(recipNorm_second <<16);
		// //Count the consecutive zero bits (trailing) on the right with multiply and lookup
		// recipNorm_second = 17 - MultiplyDeBruijnBitPosition[(((recipNorm_second & -recipNorm_second) * logical_number)) >> 27];
		recipNorm_second |= (recipNorm_second >> 1); 
		recipNorm_second |=  (recipNorm_second >> 2);
		recipNorm_second |=  (recipNorm_second >> 4);
		recipNorm_second |=  (recipNorm_second >> 8);
		recipNorm_second |=  (recipNorm_second >> 16); 
		recipNorm_second = MultiplyDeBruijnBitPosition[(recipNorm_second * logical_number) >> 27];

		//Adjust value; avoid overflow
		// sft: the number of bits to represent the decimal part
		// recipNorm_second -= sft;
		s0 >>= recipNorm_second;
		s1 >>= recipNorm_second;
		s2 >>= recipNorm_second;
		s3 >>= recipNorm_second;	

		// Apply feedback step
		qDot1 -=  s0;
		qDot2 -= s1;
		qDot3 -= s2;
		qDot4 -= s3;
		// Integrate rate of change of quaternion to yield quaternion
		q0_int =  q0_int * mediate + (qDot1 >> shift_bits);
		q1_int =  q1_int * mediate + (qDot2 >> shift_bits);
		q2_int =  q2_int * mediate + (qDot3 >> shift_bits);
		q3_int =  q3_int * mediate + (qDot4 >> shift_bits);

	//	recipNorm_second = FastDistance2D(FastDistance2D(q0_int, q1_int), FastDistance2D(q2_int, q3_int));	
		abs_s0 = ((q0_int^(q0_int>>31))-(q0_int>>31));
		abs_s1 = ((q1_int^(q1_int>>31))-(q1_int>>31));
		min_s = abs_s0+(((abs_s1-abs_s0)>>(31))&(abs_s1-abs_s0)); 
		abs_ss0 = (abs_s0+abs_s1-(min_s>>1)-(min_s>>2)+(min_s>>4));

		abs_s2 = ((q2_int^(q2_int>>31))-(q2_int>>31));
		abs_s3 = ((q3_int^(q3_int>>31))-(q3_int>>31));
		min_ss = abs_s2+(((abs_s3-abs_s2)>>(31))&(abs_s3-abs_s2)); 
		abs_ss1 = (abs_s2+abs_s3-(min_ss>>1)-(min_ss>>2)+(min_ss>>4));

		min_sss = abs_ss0+(((abs_ss1-abs_ss0)>>(31))&(abs_ss1-abs_ss0)); 
		recipNorm_second = (abs_ss0+abs_ss1-(min_sss>>1)-(min_sss>>2)+(min_sss>>4));
		// recipNorm_second = q0_int * q0_int + q1_int * q1_int + q2_int * q2_int + q3_int + q3_int;
		
		// Normalise quaternion
		// //Binary reverse
		// recipNorm_second =((recipNorm_second >>1)&reverse_1)|((recipNorm_second&reverse_1)<<1);
		// recipNorm_second =((recipNorm_second >>2)&reverse_2)|((recipNorm_second&reverse_2)<<2);
		// recipNorm_second =((recipNorm_second >>4)&reverse_3)|((recipNorm_second&reverse_3)<<4);
		// recipNorm_second =((recipNorm_second >>8)&reverse_4)|((recipNorm_second&reverse_4)<<8);
		// recipNorm_second =(recipNorm_second >>16)|(recipNorm_second <<16);
		// //Count the consecutive zero bits (trailing) on the right with multiply and lookup
		// recipNorm_second = 17 - MultiplyDeBruijnBitPosition[(((recipNorm_second & -recipNorm_second) * logical_number)) >> 27];
		recipNorm_second |= (recipNorm_second >> 1); 
		recipNorm_second |=  (recipNorm_second >> 2);
		recipNorm_second |=  (recipNorm_second >> 4);
		recipNorm_second |=  (recipNorm_second >> 8);
		recipNorm_second |=  (recipNorm_second >> 16); 
		recipNorm_second = MultiplyDeBruijnBitPosition[(recipNorm_second * logical_number) >> 27];
		
		//Adjust the value of quaternion; avoid overflow
		// sft: the number of bits to represent the decimal part
		// recipNorm_second -= sft;
		q0_int >>= recipNorm_second;
		q1_int >>= recipNorm_second;
		q2_int >>= recipNorm_second;
		q3_int >>= recipNorm_second;	

		// q0 = fr32_to_float(q0_int);
		// q1 = fr32_to_float(q1_int);
		// q2 = fr32_to_float(q2_int);
		// q3 = fr32_to_float(q3_int);	
		// recipNorm = invSqrt((q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3));
		// printf("q0: %f; q1: %f; q2: %f; q3: %f\n", q0*recipNorm, q1*recipNorm, q2*recipNorm, q3*recipNorm);

	}
	//Convert the quaternion into floating number and then normalise it
	q0 = fr32_to_float(q0_int);
	q1 = fr32_to_float(q1_int);
	q2 = fr32_to_float(q2_int);
	q3 = fr32_to_float(q3_int);		
	recipNorm = invSqrt((q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3));
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;
	return q0 * q1 * q2 * q3;
}

//---------------------------------------------------------------------------------------------------
// AHRS algorithm update

void MadgwickAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) {
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float hx, hy;
	float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2bz, _4bx, _4bz, _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

	// // Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
	// if((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
	// 	MadgwickAHRSupdateIMU(gx, gy, gz, ax, ay, az, 1);
	// 	return;
	// }
	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-q_1 * gx - q_2 * gy - q_3 * gz);
	qDot2 = 0.5f * (q_0 * gx + q_2 * gz - q_3 * gy);
	qDot3 = 0.5f * (q_0 * gy - q_1 * gz + q_3 * gx);
	qDot4 = 0.5f * (q_0 * gz + q_1 * gy - q_2 * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;   

		// Normalise magnetometer measurement
		recipNorm = invSqrt(mx * mx + my * my + mz * mz);
		mx *= recipNorm;
		my *= recipNorm;
		mz *= recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		_2q0mx = 2.0f * q_0 * mx;
		_2q0my = 2.0f * q_0 * my;
		_2q0mz = 2.0f * q_0 * mz;
		_2q1mx = 2.0f * q_1 * mx;
		_2q0 = 2.0f * q_0;
		_2q1 = 2.0f * q_1;
		_2q2 = 2.0f * q_2;
		_2q3 = 2.0f * q_3;
		_2q0q2 = 2.0f * q_0 * q_2;
		_2q2q3 = 2.0f * q_2 * q_3;
		q0q0 = q_0 * q_0;
		q0q1 = q_0 * q_1;
		q0q2 = q_0 * q_2;
		q0q3 = q_0 * q_3;
		q1q1 = q_1 * q_1;
		q1q2 = q_1 * q_2;
		q1q3 = q_1 * q_3;
		q2q2 = q_2 * q_2;
		q2q3 = q_2 * q_3;
		q3q3 = q_3 * q_3;

		// Reference direction of Earth's magnetic field
		hx = mx * q0q0 - _2q0my * q_3 + _2q0mz * q_2 + mx * q1q1 + _2q1 * my * q_2 + _2q1 * mz * q_3 - mx * q2q2 - mx * q3q3;
		hy = _2q0mx * q_3 + my * q0q0 - _2q0mz * q_1 + _2q1mx * q_2 - my * q1q1 + my * q2q2 + _2q2 * mz * q_3 - my * q3q3;
		_2bx = sqrt(hx * hx + hy * hy);
		_2bz = -_2q0mx * q_2 + _2q0my * q_1 + mz * q0q0 + _2q1mx * q_3 - mz * q1q1 + _2q2 * my * q_3 - mz * q2q2 + mz * q3q3;
		_4bx = 2.0f * _2bx;
		_4bz = 2.0f * _2bz;

		// Gradient decent algorithm corrective step
		s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax) + _2q1 * (2.0f * q0q1 + _2q2q3 - ay) - _2bz * q_2 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q_3 + _2bz * q_1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q_2 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		s1 = _2q3 * (2.0f * q1q3 - _2q0q2 - ax) + _2q0 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q_1 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + _2bz * q_3 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q_2 + _2bz * q_0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q_3 - _4bz * q_1) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax) + _2q3 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q_2 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + (-_4bx * q_2 - _2bz * q_0) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q_1 + _2bz * q_3) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q_0 - _4bz * q_2) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		s3 = _2q1 * (2.0f * q1q3 - _2q0q2 - ax) + _2q2 * (2.0f * q0q1 + _2q2q3 - ay) + (-_4bx * q_3 + _2bz * q_1) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q_0 + _2bz * q_2) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q_1 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
		recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= beta * s0;
		qDot2 -= beta * s1;
		qDot3 -= beta * s2;
		qDot4 -= beta * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	q_0 += qDot1 * (1.0f / sampleFreq);
	q_1 += qDot2 * (1.0f / sampleFreq);
	q_2 += qDot3 * (1.0f / sampleFreq);
	q_3 += qDot4 * (1.0f / sampleFreq);
	// Normalise quaternion
	recipNorm = invSqrt(q_0 * q_0 + q_1 * q_1 + q_2 * q_2 + q_3 * q_3);
	q_0 *= recipNorm;
	q_1 *= recipNorm;
	q_2 *= recipNorm;
	q_3 *= recipNorm;
}

void MadgwickAHRSupdateIMU_sub_original_change(float gx, float gy, float gz, float ax, float ay, float az) {
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 ,_8q1, _8q2, q0q0, q1q1, q2q2, q3q3;

	gx *= 0.0174533f;
	gy *= 0.0174533f;
	gz *= 0.0174533f;
	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-q_1 * gx - q_2 * gy - q_3 * gz);
	qDot2 = 0.5f * (q_0 * gx + q_2 * gz - q_3 * gy);
	qDot3 = 0.5f * (q_0 * gy - q_1 * gz + q_3 * gx);
	qDot4 = 0.5f * (q_0 * gz + q_1 * gy - q_2 * gx);
	// printf("O_c: qDot1: %f; qDot2: %f; qDot3: %f; qDot4: %f\n", qDot1, qDot2, qDot3, qDot4);

	// Normalise accelerometer measurement
	recipNorm = invSqrt(ax * ax + ay * ay + az * az);
	ax *= recipNorm;
	ay *= recipNorm;
	az *= recipNorm;   
	// printf("O_c: ax: %f; ay: %f; az: %f\n", ax, ay, az);
	// Auxiliary variables to avoid repeated arithmetic
	_2q0 = 2.0f * q_0;
	_2q1 = 2.0f * q_1;
	_2q2 = 2.0f * q_2;
	_2q3 = 2.0f * q_3;
	_4q0 = 4.0f * q_0;
	_4q1 = 4.0f * q_1;
	_4q2 = 4.0f * q_2;
	_8q1 = 8.0f * q_1;
	_8q2 = 8.0f * q_2;
	q0q0 = q_0 * q_0;
	q1q1 = q_1 * q_1;
	q2q2 = q_2 * q_2;
	q3q3 = q_3 * q_3;

	// Gradient decent algorithm corrective step
	s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
	s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q_1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
	s2 = 4.0f * q0q0 * q_2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
	s3 = 4.0f * q1q1 * q_3 - _2q1 * ax + 4.0f * q2q2 * q_3 - _2q2 * ay;
	// printf("O_c: s0: %f; s1: %f; s2: %f; s3: %f\n", s0, s1, s2, s3);
	recipNorm = sqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
	// printf("O_c: s_recipNorm: %f\n", recipNorm);

	float mediate = 10 * recipNorm;
	qDot1 = qDot1 * mediate - s0;
	qDot2 = qDot2 * mediate - s1;
	qDot3 = qDot3 * mediate - s2;
	qDot4 = qDot4 * mediate - s3;
	// printf("O_c: qDot1_: %f; qDot2_: %f; qDot3_: %f; qDot4_: %f\n", qDot1, qDot2, qDot3, qDot4);

	// Integrate rate of change of quaternion to yield quaternion
	q_0 = q_0 * mediate + qDot1 * (1.0f / sampleFreq);
	q_1 = q_1 * mediate + qDot2 * (1.0f / sampleFreq);
	q_2 = q_2 * mediate + qDot3 * (1.0f / sampleFreq);
	q_3 = q_3 * mediate + qDot4 * (1.0f / sampleFreq);
	// printf("q0: %f; q1: %f; q2: %f; q3: %f\n", q0, q1, q2, q3);
	// Normalise quaternion
	// printf("O_c: _q0: %f; _q1: %f; _q2: %f; _q3: %f\n", q_0, q_1, q_2, q_3);

	recipNorm = sqrt(q_0 * q_0 + q_1 * q_1 + q_2 * q_2 + q_3 * q_3);
	// printf("O: q_recipNorm: %f\n", recipNorm);
	q_0 /= recipNorm;
	q_1 /= recipNorm;
	q_2 /= recipNorm;
	q_3 /= recipNorm;

	printf("O_c: q0: %f; q1: %f; q2: %f; q3: %f; recipNorm: %f\n", q_0, q_1, q_2, q_3, recipNorm);
}


int main(){
	float gx[600], gy[600], gz[600], ax[600], ay[600], az[600];
	int record_order = 0;
	FILE *fp;
	char *load_file = "/home/yujie/CGRA/Morpher_DFG_Generator/MadgwickAHRS/data/input.txt";
	char line[1000];
	char *delim = ", ";
	fp = fopen(load_file, "r");
	if(fp==NULL){
		printf("can not load file!");
		return 1;
	}
	while(!feof(fp)){
		fgets(line,1000,fp);
		char *r = strtok(line, delim);
		if (r != NULL){
			gx[record_order] = (float)atoi(r);
			r = strtok(NULL, delim);
			gy[record_order] = (float)atoi(r);
			r = strtok(NULL, delim);
			gz[record_order] = (float)atoi(r);	
			r = strtok(NULL, delim);
			ax[record_order] = (float)atoi(r);
			r = strtok(NULL, delim);		
			ay[record_order] = (float)atoi(r);
			r = strtok(NULL, delim);		
			az[record_order] = (float)atoi(r);		
			// printf("gx: %f; gy: %f; gz: %f; ax: %f; ay: %f; az: %f\n", gx[record_order], gy[record_order], gz[record_order], ax[record_order], ay[record_order], az[record_order]);
			// MadgwickAHRSupdateIMU_sub(gx[record_order], gy[record_order], gz[record_order], ax[record_order], ay[record_order], az[record_order]);
			// MadgwickAHRSupdateIMU_sub_original(gx[record_order], gy[record_order], gz[record_order], ax[record_order], ay[record_order], az[record_order]);
			// MadgwickAHRSupdateIMU_sub_original_change(gx[record_order], gy[record_order], gz[record_order], ax[record_order], ay[record_order], az[record_order]);
		}
		record_order ++;
	}
	fclose(fp);
	MadgwickAHRSupdateIMU(gx, gy, gz, ax, ay, az, record_order);
	return 0;
}

//====================================================================================================
// END OF CODE
//====================================================================================================
