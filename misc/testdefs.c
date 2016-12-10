/*
 *  Check platform specific preprocessor defines.
 */
#include <stdio.h>

int main(int argc, char *argv[]) {

#if defined(__linux)
	printf("__linux defined\n");
#endif

#if defined(__unix)
	printf("__unix defined\n");
#endif

#if defined(__posix)
	printf("__posix defined\n");
#endif

#if defined(__APPLE__)
	printf("__APPLE__ defined\n");
#endif

#if defined(_WIN64)
	printf("_WIN64 defined\n");
#endif

#if defined(_WIN32)
	printf("_WIN32 defined\n");
#endif

}

