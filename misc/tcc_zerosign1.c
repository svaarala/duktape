#include <stdio.h>

#define  PRINT()  do { \
	printf("%lf -> %02x %02x %02x %02x %02x %02x %02x %02x\n", \
		u.d, \
		(unsigned int) u.x[0], \
		(unsigned int) u.x[1], \
		(unsigned int) u.x[2], \
		(unsigned int) u.x[3], \
		(unsigned int) u.x[4], \
		(unsigned int) u.x[5], \
		(unsigned int) u.x[6], \
		(unsigned int) u.x[7]); \
	} while (0)

int main(int argc, char *argv[]) {
	volatile union {
		double d;
		unsigned char x[8];
	} u;

	u.d = 0.0;
	PRINT();

	/* In TCC, with default options, this operation does not change the
	 * output.  In GCC it does.
	 */
	u.d = -u.d;
	PRINT();
}

