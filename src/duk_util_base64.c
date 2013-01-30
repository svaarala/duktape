/*
 *  Base-64 support
 */

#include "duk_internal.h"

/* dst length must be exactly ceil(len/3)*4 */
void duk_util_base64_encode(const unsigned char *src, unsigned char *dst, size_t len) {
	unsigned int i, t, snip;
	unsigned int x, y, t;
	unsigned const char *src_end = src + len;

	while (src < src_end) {
		snip = 4;
		t = 0;
		for (i = 0; i < 3; i++) {
			t = t << 8;
			if (src >= src_end) {
				snip--;
			} else {
				t += (unsigned int) (*src++);
			}
		}

		/*
		 *  Missing bytes    snip     base64 example
		 *    0               4         XXXX
		 *    1               3         XXX=
		 *    2               2         XX==
		 */

		DUK_ASSERT(snip >= 2 && snip <= 4);

		for (i = 0; i < 4; i++) {
			x = (t >> 18) & 0x3f;
			t = t << 6;

			/* A straightforward 64-byte lookup would be faster
			 * and cleaner, but this is shorter.
			 */
			if (i >= snip) {
				y = '=';
			} else if (x <= 25) {
				y = x + 'A';
			} else if (x <= 51) {
				y = x - 26 + 'a';
			} else if (x <= 61) {
				y = x - 52 + '0';
			} else if (x == 62) {
				y = '+';
			} else {
				y = '/';
			}

			*dst++ = (unsigned char) y;
		}
	}
}

