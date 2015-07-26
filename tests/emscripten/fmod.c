/*
 *  This should print "0.00000" but on some Emscripten versions it ends up
 *  printing "nan" (when executed with NodeJS or Duktape).
 */

#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[]) {
	printf("%lf\n", fmod(0.0, 4.0));
	return 1;
}
