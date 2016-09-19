/*
 *  Generate a few random doubles to compare Duktape internal implementation
 *  against the original xoroshiro128+ source.
 *
 *  Doesn't work on middle endian hosts.
 *
 *  $ gcc -o /tmp/test -std=c99 xoroshiro128plus.c xoroshiro128plus_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern uint64_t s[2];
extern uint64_t next(void);

int main(int argc, char *argv[]) {
	union {
		uint64_t x;
		double d;
	} u;
	int i;

	s[0] = 0xdeadbeef12345678ULL;
	s[1] = 0xcafed00d12345678ULL;

	for (i = 0; i < 100; i++) {
		if (i == 10) {
			printf("--- duk_heap_alloc init mixing ends here\n");
		}
		u.x = (0x3ffULL << 52) + (next() >> 12);
		printf("%08llx -> %f\n", (unsigned long long) u.x, u.d - 1.0);
	}
}
