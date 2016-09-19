/*
 *  Generate a few random doubles to compare Duktape internal implementation
 *  against the original splitmix64 source.
 *
 *  $ gcc -o /tmp/test -std=c99 splitmix64.c splitmix64_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern uint64_t x;
extern uint64_t next(void);

int main(int argc, char *argv[]) {
	union {
		uint64_t x;
		double d;
	} u;
	int i;

	x = 0xdeadbeef12345678ULL;

	for (i = 0; i < 10; i++) {
		printf("%08llx\n", (unsigned long long) next());
	}
}
