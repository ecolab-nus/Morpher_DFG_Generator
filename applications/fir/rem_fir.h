/**
 * @file rem_fir.h  Finite Impulse Response (FIR) functions
 *
 * Copyright (C) 2010 Creytiv.com
 */

/** Defines the fir filter state */
struct fir {
	int history[256];  /**< Previous samples */
	unsigned index;        /**< Sample index */
};

void fir_reset(struct fir *fir);
void fir_filter(struct fir *fir, int *outv, const int *inv, size_t inc,
		unsigned ch, const int *tapv, size_t tapc);
