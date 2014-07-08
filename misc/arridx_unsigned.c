/*
 *  Illustrate a common 64-bit portability bug when 32-bit unsigned array
 *  indices are used with 64-bit pointers.
 */

#include <stdio.h>
#include <inttypes.h>

int main(int argc, char *argv[]) {
	const char *test = "test string";
	const char *p;
	uint32_t uidx = 2;
	int32_t sidx = 2;

	printf("uidx == %ld (uint32_t), idx == %ld (int32t)\n",
	       (long) uidx, (long) sidx);

	p = test;
	printf("test -> %p\n", (void *) p);

	p = &test[uidx + 1];
	printf("&test[uidx + 1] -> %p\n", (void *) p);

	p = &test[sidx + 1];
	printf("&test[sidx + 1] -> %p\n", (void *) p);

	/* This causes a problem: uidx - 3 is computed as an unsigned
	 * value and results in 0xffffffffUL.  Adding this value to a
	 * 32-bit pointer is the same as subtracting 1 from the pointer.
	 * But for 64-bit pointers these two are not the same.
	 */
	p = &test[uidx - 3];
	printf("&test[uidx - 3] -> %p\n", (void *) p);

	p = &test[sidx - 3];
	printf("&test[sidx - 3] -> %p\n", (void *) p);

	p = test + uidx - 3;
	printf("test + uidx - 3 -> %p\n", (void *) p);

	p = test + sidx - 3;
	printf("test + sidx - 3 -> %p\n", (void *) p);

	return 0;
}
