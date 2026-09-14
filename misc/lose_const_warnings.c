/*
 *  Doing a cost-to-non-const cast without warnings, and without portability
 *  or compiler specific issues is a bit challenging.
 *
 *  $ gcc -o/tmp/test -std=c99 -Wall -Wextra -Wcast-qual misc/lose_const_warnings.c -lm
 *  misc/lose_const_warnings.c: In function ‘main’:
 *  misc/lose_const_warnings.c:24:7: warning: cast discards ‘__attribute__((const))’ qualifier from pointer target type [-Wcast-qual]
  p1 = (void *) p2;
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef union {
	void *x;
	const void *y;
} lose_const;

int main(int argc, char *argv[]) {
	void *p1;
	const void *p2;
	lose_const lc;

	(void) argc; (void) argv;

	/* Warning with gcc -Wcast-qual */
	p2 = (const void *) 0xdeadbeef;
	p1 = (void *) p2;	
	printf("p1=%p\n", p1);

	/* No warning with gcc -Wcast-qual, but (u)intptr_t types are optional */
	p2 = (const void *) 0xdeadbeef;
	p1 = (void *) (uintptr_t) p2;	
	printf("p1=%p\n", p1);

	/* No warning with gcc -Wcast-qual doesn't rely on optional types */
	p2 = (const void *) 0xdeadbeef;
	lc.y = p2;
	p1 = lc.x;
	printf("p1=%p\n", p1);
}
