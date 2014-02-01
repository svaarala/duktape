/*
 *  Check platform specific preprocessor defines.
 */
#include <stdio.h>

int main(int argc, char *argv[]) {

#ifdef __linux
	printf("__linux defined\n");
#endif

#ifdef __unix
	printf("__unix defined\n");
#endif

#ifdef __posix
	printf("__posix defined\n");
#endif

#ifdef __APPLE__
	printf("__APPLE__ defined\n");
#endif

#ifdef _WIN64
	printf("_WIN64 defined\n");
#endif

#ifdef _WIN32
	printf("_WIN32 defined\n");
#endif

}

