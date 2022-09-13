#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gsm.h"
#include "gsm_consts.h"

#define MAKE_SHORT(h,l) (((unsigned char)(h) * 0x100) + (unsigned char)(l))

static void pack_it(unsigned char [], char, unsigned char, char);
static void pack_bits(unsigned char [], int *, unsigned char, char);
static void unpack_bits(unsigned char *, int *, unsigned char *, char);

void gsm_pack(struct gsm_frame *f, unsigned char *pf) {
	int offset = 0;
	int i,j;

	/* clear the frame */
	memset(pf, 0, 33);

	/* offset by 4 (with D) for compatability with the other mob */
	pack_bits(pf, &offset, 0x0D, 4);

	/* Pack in the order given in table 1.1 of GSM spec */
	/* Add to convert signed -> unsigned, and to be compatible with
	 * other mob (again) */
	pack_bits(pf, &offset, f->lars[0]+32, 6);
	pack_bits(pf, &offset, f->lars[1]+32, 6);
	pack_bits(pf, &offset, f->lars[2]+16, 5);
	pack_bits(pf, &offset, f->lars[3]+16, 5);
	pack_bits(pf, &offset, f->lars[4]+8, 4);
	pack_bits(pf, &offset, f->lars[5]+8, 4);
	pack_bits(pf, &offset, f->lars[6]+4, 3);
	pack_bits(pf, &offset, f->lars[7]+4, 3);

	for (i = 0; i < 4; i++) {
		pack_bits(pf, &offset, f->N[i], 7);
		pack_bits(pf, &offset, f->b[i], 2);
		pack_bits(pf, &offset, f->Mc[i], 2);
		pack_bits(pf, &offset, f->xmaxc[i], 6);
		for (j = 0; j < 13; j++) {
			pack_bits(pf, &offset, f->xMc[i][j], 3);
		}
	}

}

static void pack_bits(unsigned char c[], int *bit_off, 
		unsigned char val, char size) {
	/* 
	 * Packs value in 'size' lowest bits of val into char array c 
	 * at bit offset bit_off (byte 0, highest bit = bit 0)
	 */
	pack_it(c + (*bit_off / 8), *bit_off % 8, val, size);
	*bit_off += size;
}

static void pack_it(unsigned char c[], char off, 
		unsigned char val, char size) {
	int shiftsize;
	/* 
	 * Packs 'size' least significant bits of 'val' into bytes c[0],c[1] 
	 * shifted right by offset 'off'.
	 *
	 * PRE: size <= 8
	 *      c,c+1 is valid memory
	 *      off < 8
	 */

	/* mask out bits of val we want */
	val &= 0x00FF >> (8 - size);

	/* 
	 * first bit (8 = highest, 1=lowest) = 8 - offset 
	 * shift left = first bit - size 
	 */
	shiftsize = ((8 - off) - size);

	if (shiftsize > 0) {
		*c |= (val << shiftsize);
	} else {
		*c |= (val >> -shiftsize);
	}

	if ((8 - off) < size) {
		/* if this is false we don't need to put any bits in the 
		 * second byte */
		/* 
		 * bits in first byte = size - shiftsize 
		 * (negative shiftsize = right shift)
		 * bits in second byte = size - bits in first byte
		 * shift = 8 - bits in second byte
		 * .'. shift = 8 - size - (size-shiftsize)
		 */
		shiftsize = 8 + shiftsize;
		*(c+1) |= (val << shiftsize);
	}

}

void gsm_unpack(struct gsm_frame *f, unsigned char *pf) {
	int offset = 0;
	int i=0;
	int j=0;

	/* clear the frame */
	memset(f, 0, sizeof(struct gsm_frame));

	/* offset by 4 for compatability with the other mob */
	unpack_bits((unsigned char *)(&(i)), &offset, pf, 4);
	if (i != 0xD) fprintf(stderr, "Warning: May not be a gsm frame!\n");
	i = 0;

	/* Unpack assuming the order given in table 1.1 of GSM spec */
	/* restore signedness by subtracting minimum value */
	unpack_bits(&(f->lars[0]), &offset, pf, 6);
	f->lars[0] -=32;
	unpack_bits(&(f->lars[1]), &offset, pf, 6);
	f->lars[1] -=32;
	unpack_bits(&(f->lars[2]), &offset, pf, 5);
	f->lars[2] -=16;
	unpack_bits(&(f->lars[3]), &offset, pf, 5);
	f->lars[3] -=16;
	unpack_bits(&(f->lars[4]), &offset, pf, 4);
	f->lars[4] -=8;
	unpack_bits(&(f->lars[5]), &offset, pf, 4);
	f->lars[5] -=8;
	unpack_bits(&(f->lars[6]), &offset, pf, 3);
	f->lars[6] -=4;
	unpack_bits(&(f->lars[7]), &offset, pf, 3);
	f->lars[7] -=4;

	for (i = 0; i < 4; i++) {
		unpack_bits(&(f->N[i]), &offset, pf, 7);
		unpack_bits(&(f->b[i]), &offset, pf, 2);
		unpack_bits(&(f->Mc[i]), &offset, pf, 2);
		unpack_bits(&(f->xmaxc[i]), &offset, pf, 6);
		for (j = 0; j < 13; j++) {
			unpack_bits(&(f->xMc[i][j]), &offset, pf, 3);
		}
	}

}

static void unpack_bits(unsigned char *c, int *bit_off, 
		unsigned char *pf, char size) {
	unsigned char off;
	unsigned char *p;
	unsigned short temp = 0;
	
	p = pf + (*bit_off / 8);
	off = *bit_off % 8;
	
	if (off + size <= 8) {
		/* we need to get bits from 1 byte */
		temp = *p;
		/* shift from first bit = off to first bit = 8 - size */
		temp >>= (8 - size) - off;
	} else {
		/* we need to get bits from 2 bytes */
		temp = MAKE_SHORT((char)(*p), (char)(*(p+1)));
		/* shift from first bit = off to first bit = 16 - size */
		temp >>= (16 - size) - off;
	}
	/* Mask out the bits we want */
	temp &= (0x00FF >> (8 - size));
	*c = (char)temp;
	*bit_off += size;
}

