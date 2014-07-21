/*
 *  Hex escapes in C code can be confusing; this is non-portable:
 *
 *      const char *str = "\xffab";
 *
 *  GCC will warn:
 *
 *      warning: hex escape sequence out of range [enabled by default]
 *
 *  To avoid this, you can e.g. use the following form:
 *
 *      const char *str = "\xff" "ab";
 *
 *  If the C hex escape is terminated by a character that cannot be a
 *  valid hex digit, there is no need for this (but breaking up the
 *  string is still good practice), e.g.:
 *
 *      const char *str = "\xffquux";
 *
 *  Another hex escape always terminates a previous hex escape.  For
 *  instance, to write a user internal property constant with two
 *  leading 0xFF bytes:
 *
 *      const char *str = "\xff\xff" "quux";
 */

#include <stdio.h>

int main(int argc, char *argv[]) {
	const char *str = "\xffabcdef";  /* Generates a warning on GCC */
	printf("%s\n", str);
	return 0;
}
