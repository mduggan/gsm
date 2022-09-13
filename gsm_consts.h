#ifndef _GSM_CONSTS_H_
#define _GSM_CONSTS_H_

/* Pre-processing constants */
extern const float alpha;
extern const float beta;

/* Log Area Ratio constants */
extern const float LAR_A[];
extern const float LAR_B[];
extern const signed char LAR_MIN[];
extern const signed char LAR_MAX[];

/* LAR Interpolation constants (Table 3.2) */
extern const int LAR_INTERP_BOUND[];
extern const float LAR_INTERP_COEFF1[];
extern const float LAR_INTERP_COEFF2[];

/* Long Term Predictor constants */
extern const float DLB[];
extern const float QLB[];

/* RPE Encoding Weighting Filter Impulse Response */
extern const float weighting_h[];

/* RPE Quantisation Lookup table */
extern const float xmax_lookup[];

#endif /* GSM_CONSTS_H */
