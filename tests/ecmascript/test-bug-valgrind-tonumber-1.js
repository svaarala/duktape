/* This caused a valgrind error at some point.
 *
 * This might actually be a libc/valgrind issue?  The following example
 * will also cause similar valgrind complaints:
 *
 *   #include <stdio.h>
 *   #include <string.h>
 *
 *   int main(int argc, char *argv[]) {
 *       double d = 0.0;
 *       char buf[256];
 *
 *       memset(buf, 0, sizeof(buf));
 *       sprintf(buf, "%s", "NaN");
 *       sscanf(buf, "%lg", &d);
 *       printf("%lg\n", d);
 *   }
 */

/*===
still here
===*/

var x;

x = "NaN" >>> 0;
print('still here');
