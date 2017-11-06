/*
 *  Handling of floating point casts.
 *
 *  Little endian only.
 */

#include <stdio.h>
#include <math.h>

#if 0
#define __FLT_MAX__ 3.40282346638528859812e+38F
#endif

#define FLT_MAX        340282346638528859811704183484516925440.0
#define FLT_MAX_PLUS   340282366920938463463374607431768211456.0

static void dump_memory(volatile unsigned char *p, size_t len) {
	size_t i, idx;

	for (i = 0; i < len; i++) {
		idx = len - i - 1;  /* for little endian */
		printf("%02x", (unsigned int) p[idx]);
	}
	printf("\n");
}

int main(int argc, const char *argv[]) {
	volatile double d;
	volatile float f;
	volatile unsigned char *p;

	/* FLT_MAX: (1 - 2^(-24)) * 2^128. */
	printf("- FLT_MAX\n");
	d = FLT_MAX;
	f = (float) d;
	printf("d: %f\n", d);
	printf("f: %f\n", f);
	dump_memory((volatile unsigned char *) &d, 8);
	dump_memory((volatile unsigned char *) &f, 4);

	/* FLT_MAX + floating point unit: 2^128. */
	printf("- FLT_MAX_PLUS\n");
	d = FLT_MAX_PLUS;
	f = (float) d;
	printf("d: %f\n", d);
	printf("f: %f\n", f);
	dump_memory((volatile unsigned char *) &d, 8);
	dump_memory((volatile unsigned char *) &f, 4);

	/* Number between FLT_MAX and FLT_MAX + unit. */
	printf("- between FLT_MAX and FLT_MAX_PLUS\n");
	d = (FLT_MAX + FLT_MAX_PLUS) / 2.0;
	f = (float) d;
	printf("d: %f\n", d);
	printf("f: %f\n", f);
	dump_memory((volatile unsigned char *) &d, 8);
	dump_memory((volatile unsigned char *) &f, 4);

	/* Same as above, but one double unit less.
	 * 47effffff0000000 ->
	 * 47efffffefffffff
	 *
	 * This is the largest double that doesn't round to infinity on x64.
	 */
	printf("- just below above\n");
	p = (volatile unsigned char *) &d;
	p[7] = 0x47U;
	p[6] = 0xefU;
	p[5] = 0xffU;
	p[4] = 0xffU;
	p[3] = 0xefU;
	p[2] = 0xffU;
	p[1] = 0xffU;
	p[0] = 0xffU;
	f = (float) d;
	printf("d: %f\n", d);
	printf("f: %f\n", f);
	dump_memory((volatile unsigned char *) &d, 8);
	dump_memory((volatile unsigned char *) &f, 4);

	return 0;
}
