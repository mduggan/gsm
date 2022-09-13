#include "gsm_consts.h"

/* Pre-processing constants */
const float alpha = 32735.0 / 32768.0;
const float beta = 28180.0 / 32768.0;

/* Log Area Ratio constants */
const float LAR_A[] = { 20.0, 20.0, 20.0, 20.0, 13.637, 15.0, 8.334, 8.824 };
const float LAR_B[] = { 0.0, 0.0, 4.0, -5.0, 0.184, -3.5, -0.666, -2.235 };
const signed char LAR_MIN[] = { -32, -32, -16, -16, -8, -8, -4, -4 };
const signed char LAR_MAX[] = { 31, 31, 15, 15, 7, 7, 3, 3 };

/* LAR Interpolation constants (Table 3.2) */
const int LAR_INTERP_BOUND[] = { 0, 13, 27, 40, 160 };
const float LAR_INTERP_COEFF1[] = { 0.75, 0.5, 0.25, 0.0 };
const float LAR_INTERP_COEFF2[] = { 0.25, 0.5, 0.75, 1.0 };

/* Long Term Predictor constants */
const float DLB[] = { 0.2, 0.5, 0.8 };
const float QLB[] = { 0.10, 0.35, 0.65, 1.0 };

/* RPE Encoding Weighting Filter Impulse Response */
const float weighting_h[] = 
	{-134, -374, 0, 2054, 5741, 8192, 5741, 2054, 0, -374, -134};

/* RPE Quantisation Lookup table */
const float xmax_lookup[] =
	{ 
		31, 63, 95, 127, 159, 191, 223, 255, 287, 319, 351, 383, 415, 
		447, 479, 511, 575, 639, 703, 767, 831, 895, 959, 1023, 1151,
		1279, 1407, 1535, 1663, 1791, 1919, 2047, 2303, 2559, 2815,
		3071, 3327, 3583, 3839, 4095, 4607, 5119, 5631, 6143, 6655,
		7167, 7679, 8191, 9215, 10239, 11263, 12287, 13311, 14335,
		15359, 16383, 18431, 20479, 22527, 24575, 26623, 28671, 
		30719, 32767 
	};

