/*
 *  Preprocessor compare with left part undefined.
 */

#include <stdio.h>

#define FOO 123
#undef BAR

int main(int argc, char *argv[]) {
#if defined(FOO) && defined(BAR) && (BAR == FOO)
	printf("FOO == BAR\n");
#else
	printf("FOO != BAR\n");
#endif
	return 0;
}
