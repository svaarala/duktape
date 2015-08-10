#include <stdio.h>
#include <stdlib.h>

struct dummy {
	double d;
};

int main(int argc, char *argv[]) {
	struct dummy *d;
	char *p;

	(void) argc; (void) argv;

	/* This causes a warning with clang -Wcast-align:
	 *
	 * clang_cast_align.c:13:6: warning: cast from 'char *' to 'struct dummy *' increases required alignment
	 *       from 1 to 8 [-Wcast-align]
	 *         d = (struct dummy *) p;
	 *
	 * Note that malloc() alignment guarantees are enough to make the
	 * warning harmless, so it'd be nice to suppress.
	 */

	p = (char *) malloc(sizeof(struct dummy));
	d = (struct dummy *) p;

	/* Casting through a void pointer suppressed the warning for clang. */

	p = (char *) malloc(sizeof(struct dummy));
	d = (struct dummy *) (void *) p;

	return 0;
}
