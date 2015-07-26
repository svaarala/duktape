/*
 *  Same as test-dev-mandel2-func.js but implemented in C.  Useful to assess
 *  how much time is spent in soft float routines as opposed to everything else.
 */

#include <stdio.h>
#include <math.h>

static void mandel(void) {
	int w = 80;
	int h = 40;
	int iter = 100;
	int i, j, k;
	double x0, y0, xx, yy, xx2, yy2;
	char c;

	for (i = 0; i - h; i += 1) {
		y0 = ((double) i / (double) h) * 4.0 - 2.0;

		for (j = 0; j - w; j += 1) {
			x0 = ((double) j / (double) w) * 4.0 - 2.0;

			xx = 0.0;
			yy = 0.0;
			c = '#';

			for (k = 0; k - iter; k += 1) {
				xx2 = xx * xx;
				yy2 = yy * yy;

				if (fmax(0.0, 4.0 - (xx2 + yy2)) != 0.0) {
					yy = 2 * xx * yy + y0;
					xx = xx2 - yy2 + x0;
				} else {
					c = '.';
					break;
				}
			}

			printf("%c", c);
		}

		printf("\n");
	}
}

int main(int argc, char *argv[]) {
	mandel();
}
