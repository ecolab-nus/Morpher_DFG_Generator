/**
 * @file fir.c FIR -- Finite Impulse Response
 *
 * Copyright (C) 2010 Creytiv.com
https://github.com/creytiv/rem/blob/master/src/fir/fir.c
 */

#include <string.h>
//#include <re.h>
#include "rem_fir.h"


/**
 * Reset the FIR-filter
 *
 * @param fir FIR-filter state
 */
void fir_reset(struct fir *fir)
{
	if (!fir)
		return;

	memset(fir, 0, sizeof(*fir));
}


/**
 * Process samples with the FIR filter
 *
 * @note product of channel and tap-count must be power of two
 *
 * @param fir  FIR filter
 * @param outv Output samples
 * @param inv  Input samples
 * @param inc  Number of samples
 * @param ch   Number of channels
 * @param tapv Filter taps
 * @param tapc Number of taps
 */
void fir_filter(struct fir *fir, int *outv, const int *inv, size_t inc,
		unsigned ch, const int *tapv, size_t tapc)
{
	const unsigned hmask = (ch * (unsigned)tapc) - 1;

	if (!fir || !outv || !inv || !ch || !tapv || !tapc)
		return;

	if (hmask >= ARRAY_SIZE(fir->history) || hmask & (hmask+1))
		return;

	while (inc--) {

		int acc = 0;
		unsigned i, j;

		fir->history[fir->index & hmask] = *inv++;

		for (i=0, j=fir->index++; i<tapc; i=i+46, j-=ch){
		#ifdef CGRA_COMPILER
           	please_map_me();
           	#endif
			acc += (int)fir->history[j & hmask] * tapv[i];
			acc += (int)fir->history[j & hmask] * tapv[i+1];
			acc += (int)fir->history[j & hmask] * tapv[i+2];
			acc += (int)fir->history[j & hmask] * tapv[i+3];
			acc += (int)fir->history[j & hmask] * tapv[i+4];
			acc += (int)fir->history[j & hmask] * tapv[i+5];
			acc += (int)fir->history[j & hmask] * tapv[i+6];
			acc += (int)fir->history[j & hmask] * tapv[i+7];
			acc += (int)fir->history[j & hmask] * tapv[i+8];
			acc += (int)fir->history[j & hmask] * tapv[i+9];
			acc += (int)fir->history[j & hmask] * tapv[i+10];
			acc += (int)fir->history[j & hmask] * tapv[i+11];
			acc += (int)fir->history[j & hmask] * tapv[i+12];
			acc += (int)fir->history[j & hmask] * tapv[i+13];
			acc += (int)fir->history[j & hmask] * tapv[i+14];
			acc += (int)fir->history[j & hmask] * tapv[i+15];
			acc += (int)fir->history[j & hmask] * tapv[i+16];
			acc += (int)fir->history[j & hmask] * tapv[i+17];
			acc += (int)fir->history[j & hmask] * tapv[i+18];
			acc += (int)fir->history[j & hmask] * tapv[i+19];
			acc += (int)fir->history[j & hmask] * tapv[i+20];
			acc += (int)fir->history[j & hmask] * tapv[i+21];
			acc += (int)fir->history[j & hmask] * tapv[i+22];
			acc += (int)fir->history[j & hmask] * tapv[i+23];
			acc += (int)fir->history[j & hmask] * tapv[i+24];
			acc += (int)fir->history[j & hmask] * tapv[i+25];
			acc += (int)fir->history[j & hmask] * tapv[i+26];
			acc += (int)fir->history[j & hmask] * tapv[i+27];
			acc += (int)fir->history[j & hmask] * tapv[i+28];
			acc += (int)fir->history[j & hmask] * tapv[i+29];
			acc += (int)fir->history[j & hmask] * tapv[i+30];
			acc += (int)fir->history[j & hmask] * tapv[i+31];
			acc += (int)fir->history[j & hmask] * tapv[i+32];
			acc += (int)fir->history[j & hmask] * tapv[i+33];
			acc += (int)fir->history[j & hmask] * tapv[i+34];
			acc += (int)fir->history[j & hmask] * tapv[i+35];
			acc += (int)fir->history[j & hmask] * tapv[i+36];
			acc += (int)fir->history[j & hmask] * tapv[i+37];
			acc += (int)fir->history[j & hmask] * tapv[i+38];
			acc += (int)fir->history[j & hmask] * tapv[i+39];
			acc += (int)fir->history[j & hmask] * tapv[i+40];
			acc += (int)fir->history[j & hmask] * tapv[i+41];
			acc += (int)fir->history[j & hmask] * tapv[i+42];
			acc += (int)fir->history[j & hmask] * tapv[i+43];
			acc += (int)fir->history[j & hmask] * tapv[i+44];
			acc += (int)fir->history[j & hmask] * tapv[i+45];
		}

		if (acc > 0x3fffffff)
			acc = 0x3fffffff;
		else if (acc < -0x40000000)
			acc = -0x40000000;

		*outv++ = (int)(acc>>15);
	}
}


void fir_filter_unrolled_2(struct fir *fir, int *outv, const int *inv, size_t inc,
		unsigned ch, const int *tapv, size_t tapc)
{
	const unsigned hmask = (ch * (unsigned)tapc) - 1;

	if (!fir || !outv || !inv || !ch || !tapv || !tapc)
		return;

	if (hmask >= ARRAY_SIZE(fir->history) || hmask & (hmask+1))
		return;

	while (inc--) {

		int acc = 0;
		unsigned i, j;

		fir->history[fir->index & hmask] = *inv++;

		for (i=0, j=fir->index++; i<tapc; i=i+90, j-=ch){
		#ifdef CGRA_COMPILER
           	please_map_me();
           	#endif
			acc = acc+(int)fir->history[j & hmask] * tapv[i]+ (int)fir->history[j & hmask] * tapv[i+1] +
			 (int)fir->history[j & hmask] * tapv[i+2]
			+ (int)fir->history[j & hmask] * tapv[i+3]
			+ (int)fir->history[j & hmask] * tapv[i+4]
			+ (int)fir->history[j & hmask] * tapv[i+5]
			+ (int)fir->history[j & hmask] * tapv[i+6]
			+ (int)fir->history[j & hmask] * tapv[i+7]
			+ (int)fir->history[j & hmask] * tapv[i+8]
			+ (int)fir->history[j & hmask] * tapv[i+9]
			+ (int)fir->history[j & hmask] * tapv[i+10]
			+ (int)fir->history[j & hmask] * tapv[i+11]
			+ (int)fir->history[j & hmask] * tapv[i+12]
			+ (int)fir->history[j & hmask] * tapv[i+13]
			+ (int)fir->history[j & hmask] * tapv[i+14]
			+ (int)fir->history[j & hmask] * tapv[i+15]
			+ (int)fir->history[j & hmask] * tapv[i+16]
			+ (int)fir->history[j & hmask] * tapv[i+17]
			+ (int)fir->history[j & hmask] * tapv[i+18]
			+ (int)fir->history[j & hmask] * tapv[i+19]
			+ (int)fir->history[j & hmask] * tapv[i+20]
			+ (int)fir->history[j & hmask] * tapv[i+21]
			+ (int)fir->history[j & hmask] * tapv[i+22]
			+ (int)fir->history[j & hmask] * tapv[i+23]
			+ (int)fir->history[j & hmask] * tapv[i+24]
			+ (int)fir->history[j & hmask] * tapv[i+25]
			+ (int)fir->history[j & hmask] * tapv[i+26]
			+ (int)fir->history[j & hmask] * tapv[i+27]
			+ (int)fir->history[j & hmask] * tapv[i+28]
			+ (int)fir->history[j & hmask] * tapv[i+29]
			+ (int)fir->history[j & hmask] * tapv[i+30]
			+ (int)fir->history[j & hmask] * tapv[i+31]
			+ (int)fir->history[j & hmask] * tapv[i+32]
			+ (int)fir->history[j & hmask] * tapv[i+33]
			+ (int)fir->history[j & hmask] * tapv[i+34]
			+ (int)fir->history[j & hmask] * tapv[i+35]
			+ (int)fir->history[j & hmask] * tapv[i+36]
			+ (int)fir->history[j & hmask] * tapv[i+37]
			+ (int)fir->history[j & hmask] * tapv[i+38]
			+ (int)fir->history[j & hmask] * tapv[i+39]
			+ (int)fir->history[j & hmask] * tapv[i+40]
			+ (int)fir->history[j & hmask] * tapv[i+41]
			+ (int)fir->history[j & hmask] * tapv[i+42]
			+ (int)fir->history[j & hmask] * tapv[i+43]
			+ (int)fir->history[j & hmask] * tapv[i+44]
			+ (int)fir->history[j & hmask] * tapv[i+45]
			+ (int)fir->history[j & hmask] * tapv[i+46]
			+ (int)fir->history[j & hmask] * tapv[i+47]
			+ (int)fir->history[j & hmask] * tapv[i+48]
			+ (int)fir->history[j & hmask] * tapv[i+49]
			+ (int)fir->history[j & hmask] * tapv[i+50]
			+ (int)fir->history[j & hmask] * tapv[i+51]
			+ (int)fir->history[j & hmask] * tapv[i+52]
			+ (int)fir->history[j & hmask] * tapv[i+53]
			+ (int)fir->history[j & hmask] * tapv[i+54]
			+ (int)fir->history[j & hmask] * tapv[i+55]
			+ (int)fir->history[j & hmask] * tapv[i+56]
			+ (int)fir->history[j & hmask] * tapv[i+57]
			+ (int)fir->history[j & hmask] * tapv[i+58]
			+ (int)fir->history[j & hmask] * tapv[i+59]
			+ (int)fir->history[j & hmask] * tapv[i+60]
			+ (int)fir->history[j & hmask] * tapv[i+61]
			+ (int)fir->history[j & hmask] * tapv[i+62]
			+ (int)fir->history[j & hmask] * tapv[i+63]
			+ (int)fir->history[j & hmask] * tapv[i+64]
			+ (int)fir->history[j & hmask] * tapv[i+65]
			+ (int)fir->history[j & hmask] * tapv[i+66]
			+ (int)fir->history[j & hmask] * tapv[i+67]
			+ (int)fir->history[j & hmask] * tapv[i+68]
			+ (int)fir->history[j & hmask] * tapv[i+69]
			+ (int)fir->history[j & hmask] * tapv[i+70]
			+ (int)fir->history[j & hmask] * tapv[i+71]
			+ (int)fir->history[j & hmask] * tapv[i+72]
			+ (int)fir->history[j & hmask] * tapv[i+73]
			+ (int)fir->history[j & hmask] * tapv[i+74]
			+ (int)fir->history[j & hmask] * tapv[i+75]
			+ (int)fir->history[j & hmask] * tapv[i+76]
			+ (int)fir->history[j & hmask] * tapv[i+77]
			+ (int)fir->history[j & hmask] * tapv[i+78]
			+ (int)fir->history[j & hmask] * tapv[i+79]
			+ (int)fir->history[j & hmask] * tapv[i+80]
			+ (int)fir->history[j & hmask] * tapv[i+81]
			+ (int)fir->history[j & hmask] * tapv[i+82]
			+ (int)fir->history[j & hmask] * tapv[i+83]
			+ (int)fir->history[j & hmask] * tapv[i+84]
			+ (int)fir->history[j & hmask] * tapv[i+85]
			+ (int)fir->history[j & hmask] * tapv[i+86]
			+ (int)fir->history[j & hmask] * tapv[i+87]
			+ (int)fir->history[j & hmask] * tapv[i+88]
			+ (int)fir->history[j & hmask] * tapv[i+89]
			+ (int)fir->history[j & hmask] * tapv[i+90]
			+ (int)fir->history[j & hmask] * tapv[i+91]
			+ (int)fir->history[j & hmask] * tapv[i+92]
			+ (int)fir->history[j & hmask] * tapv[i+93]
			+ (int)fir->history[j & hmask] * tapv[i+94]
			+ (int)fir->history[j & hmask] * tapv[i+95];
		}

		if (acc > 0x3fffffff)
			acc = 0x3fffffff;
		else if (acc < -0x40000000)
			acc = -0x40000000;

		*outv++ = (int)(acc>>15);
	}
}
