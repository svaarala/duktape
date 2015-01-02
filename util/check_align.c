/*
 *  Check for alignment requirements.
 *
 *  Called from a shell script check_align.sh to execute one test at a time.
 *  Prohibited unaligned accesses cause a SIGBUS.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>

char temp_buffer[256];

static char *get_aligned(int align_by, int offset) {
	char *p = temp_buffer;
	intptr_t t;

	for (;;) {
		t = (intptr_t) p;
		if (t % align_by == 0) {
			return p + offset;
		}
		p++;
	}
}

static void sigbus_handler(int signum) {
	printf("SIGBUS\n");
	exit(1);
}

int main(int argc, char *argv[]) {
	struct sigaction sa;
	int offset;

	memset((void *) &sa, 0, sizeof(sa));
	sa.sa_handler = sigbus_handler;
	sigaction(SIGBUS, &sa, NULL);

	if (argc != 3) {
		goto usage;
	}

	offset = atoi(argv[2]);

	if (strcmp(argv[1], "uint16_t") == 0) {
		uint16_t *p_u16;
		p_u16 = (uint16_t *) get_aligned(2, offset);
		printf("uint16_t offset %d, ptr %p\n", offset, (void *) p_u16);
		*p_u16 = 0xdeadUL;
		printf("%lx\n", (unsigned long) *p_u16);
	} else if (strcmp(argv[1], "uint32_t") == 0) {
		uint32_t *p_u32;
		p_u32 = (uint32_t *) get_aligned(4, offset);
		printf("uint32_t offset %d, ptr %p\n", offset, (void *) p_u32);
		*p_u32 = 0xdeadbeefUL;
		printf("%lx\n", (unsigned long) *p_u32);
	} else if (strcmp(argv[1], "uint64_t") == 0) {
		uint64_t *p_u64;
		p_u64 = (uint64_t *) get_aligned(8, offset);
		printf("uint64_t offset %d, ptr %p\n", offset, (void *) p_u64);
		*p_u64 = 0xdeadbeef12345678ULL;
		printf("%llx\n", (unsigned long long) *p_u64);
	} else if (strcmp(argv[1], "double") == 0) {
		double *p_dbl;
		p_dbl = (double *) get_aligned(8, offset);
		printf("double offset %d, ptr %p\n", offset, (void *) p_dbl);
		*p_dbl = 123456789.0;
		printf("%lf\n", *p_dbl);
	} else {
		goto usage;
	}

	exit(0);

 usage:
	fprintf(stderr, "Usage: ./check_align (uint16_t|uint32_t|uint64_t|double) <offset>\n");
	exit(1);
}
