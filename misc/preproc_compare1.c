/*
 *  Preprocessor compare with right part undefined.
 */

#include <stdio.h>

#define FOO 123
#undef BAR

int main(int argc, char *argv[]) {
#if defined(FOO) && defined(BAR) && (FOO == BAR)
	printf("FOO == BAR\n");
#else
	printf("FOO != BAR\n");
#endif
	return 0;
}
