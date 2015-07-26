/*
 *  Mandelbrot test case.
 */

#include <stdio.h>

int main(int argc, char *argv[]) {
	int w = 80;
	int h = 40;
	int iter = 100;
	int i, j, k;
	double x0, y0, xx, yy, xx2, yy2;
	char row[80+1];
	char c;

	for (i = 0; i < h; i++) {
		y0 = (double) i / (double) h * 4.0 - 2.0;

		for (j = 0; j < w; j++) {
			x0 = (double) j / (double) w * 4.0 - 2.0;
			xx = 0.0;
			yy = 0.0;
			c = '#';

			for (k = 0; k < iter; k++) {
				/* z -> z^2 + c
				 *   -> (xx+i*yy)^2 + (x0+i*y0)
				 *   -> xx*xx+i*2*xx*yy-yy*yy + x0 + i*y0
				 *   -> (xx*xx - yy*yy + x0) + i*(2*xx*yy + y0)
				 */

				xx2 = xx * xx;
				yy2 = yy * yy;
				if (xx2 + yy2 < 4.0) {
					yy = 2 * xx * yy + y0;
					xx = xx2 - yy2 + x0;
				} else {
					c = '.';
					break;
				}
			}

			row[j] = c;
		}

		row[j] = (char) 0;

		printf("%s\n", row);
	}

	return 0;
}
