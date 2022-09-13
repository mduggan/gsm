#ifndef _GSM_LPC_H_
#define _GSM_LPC_H_

void gsm_lpc(struct gsm_frame *, float []);

void gsm_get_lars(float lars[], float s[]);

#endif /* GSM_LPC_H */
