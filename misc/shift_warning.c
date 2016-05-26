/*
 *  With 32-bit integers the expression:
 *
 *      uint32_t x = 26 << 27;
 *
 *  causes a warning on some compilers because the signed integer 26 (0x1a) is
 *  first shifted 27 positions to the left which remains within unsigned 32-bit
 *  range but overflows the signed 32-bit range.  This has no practical impact
 *  (at least on common compilers) because the result is ultimately an unsigned
 *  32-bit integer, 0xd0000000.
 *
 *  In GCC 4.x there doesn't seem to be a warning option to detect this.
 *  In clang this works (also -Weverything):
 *
 *      $ clang -Wshift-sign-overflow -otest misc/shift_warning.c
 *      misc/shift_warning.c:19:9: warning: signed shift result (0xD0000000)
 *           sets the sign bit of the shift expression's type ('int') and
 *           becomes negative [-Wshift-sign-overflow]
 *             t = 26 << 27;
 *                 ~~ ^  ~~
 *      1 warning generated.
 *
 *      $ ./test
 *      d0000000
 */

#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
	uint32_t t;

	t = 26 << 27;
	printf("%08lx\n", (unsigned long) t);

	(void) argc;
	(void) argv;

	return 0;
}
