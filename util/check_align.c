/*
 *  Check for alignment requirements and endianness.
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
	(void) signum;
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
		volatile uint16_t *p_u16;
		volatile uint8_t *p_u8;

		p_u16 = (uint16_t *) get_aligned(2, offset);
		printf("uint16_t offset %d, ptr %p\n", offset, (void *) p_u16);
		*p_u16 = 0x1122UL;

		p_u8 = (volatile uint8_t *) p_u16;
		printf("%02x %02x = 0x%lx\n",
		       (unsigned int) p_u8[0],
		       (unsigned int) p_u8[1],
		       (unsigned long) *p_u16);

		if (p_u8[0] == 0x11 && p_u8[1] == 0x22) {
			printf("big endian\n");
		} else if (p_u8[0] == 0x22 && p_u8[1] == 0x11) {
			printf("little endian\n");
		} else {
			printf("unknown endian\n");
		}
	} else if (strcmp(argv[1], "uint32_t") == 0) {
		volatile uint32_t *p_u32;
		volatile uint8_t *p_u8;

		p_u32 = (uint32_t *) get_aligned(4, offset);
		printf("uint32_t offset %d, ptr %p\n", offset, (void *) p_u32);
		*p_u32 = 0x11223344UL;

		p_u8 = (volatile uint8_t *) p_u32;
		printf("%02x %02x %02x %02x = 0x%lx\n",
		       (unsigned int) p_u8[0],
		       (unsigned int) p_u8[1],
		       (unsigned int) p_u8[2],
		       (unsigned int) p_u8[3],
		       (unsigned long) *p_u32);

		if (p_u8[0] == 0x11 && p_u8[1] == 0x22 && p_u8[2] == 0x33 && p_u8[3] == 0x44) {
			printf("big endian\n");
		} else if (p_u8[0] == 0x44 && p_u8[1] == 0x33 && p_u8[2] == 0x22 && p_u8[3] == 0x11) {
			printf("little endian\n");
		} else {
			printf("unknown endian\n");
		}
	} else if (strcmp(argv[1], "uint64_t") == 0) {
		volatile uint64_t *p_u64;
		volatile uint8_t *p_u8;

		p_u64 = (uint64_t *) get_aligned(8, offset);
		printf("uint64_t offset %d, ptr %p\n", offset, (void *) p_u64);
		*p_u64 = 0x1122334455667788ULL;

		p_u8 = (volatile uint8_t *) p_u64;
		printf("%02x %02x %02x %02x %02x %02x %02x %02x = 0x%llx\n",
		       (unsigned int) p_u8[0],
		       (unsigned int) p_u8[1],
		       (unsigned int) p_u8[2],
		       (unsigned int) p_u8[3],
		       (unsigned int) p_u8[4],
		       (unsigned int) p_u8[5],
		       (unsigned int) p_u8[6],
		       (unsigned int) p_u8[7],
		       (unsigned long long) *p_u64);

		if (p_u8[0] == 0x11 && p_u8[1] == 0x22 && p_u8[2] == 0x33 && p_u8[3] == 0x44 &&
		    p_u8[4] == 0x55 && p_u8[5] == 0x66 && p_u8[6] == 0x77 && p_u8[7] == 0x88) {
			printf("big endian\n");
		} else if (p_u8[0] == 0x88 && p_u8[1] == 0x77 && p_u8[2] == 0x66 && p_u8[3] == 0x55 &&
		           p_u8[4] == 0x44 && p_u8[5] == 0x33 && p_u8[6] == 0x22 && p_u8[7] == 0x11) {
			printf("little endian\n");
		} else if (p_u8[0] == 0x44 && p_u8[1] == 0x33 && p_u8[2] == 0x22 && p_u8[3] == 0x11 &&
		           p_u8[4] == 0x88 && p_u8[5] == 0x77 && p_u8[6] == 0x66 && p_u8[7] == 0x55) {
			printf("mixed endian\n");
		} else {
			printf("unknown endian\n");
		}
	} else if (strcmp(argv[1], "double") == 0) {
		volatile double *p_dbl;
		volatile uint8_t *p_u8;

		p_dbl = (double *) get_aligned(8, offset);
		printf("double offset %d, ptr %p\n", offset, (void *) p_dbl);
		*p_dbl = 112233445566778899.0;

		/* >>> struct.pack('>d', 112233445566778899).encode('hex')
		 * '4378ebbb95eed0e1'
		 *
		 * 43 78 eb bb 95 ee d0 e1    big endian
		 * e1 d0 ee 95 bb eb 78 43    little endian
		 * bb eb 78 43 e1 d0 ee 95    mixed endian
		 *
		 * Rounds to 112233445566778896.0.
		 */

		p_u8 = (volatile uint8_t *) p_dbl;
		printf("%02x %02x %02x %02x %02x %02x %02x %02x = %lf\n",
		       (unsigned int) p_u8[0],
		       (unsigned int) p_u8[1],
		       (unsigned int) p_u8[2],
		       (unsigned int) p_u8[3],
		       (unsigned int) p_u8[4],
		       (unsigned int) p_u8[5],
		       (unsigned int) p_u8[6],
		       (unsigned int) p_u8[7],
		       *p_dbl);

		if (p_u8[0] == 0x43 && p_u8[1] == 0x78 && p_u8[2] == 0xeb && p_u8[3] == 0xbb &&
		    p_u8[4] == 0x95 && p_u8[5] == 0xee && p_u8[6] == 0xd0 && p_u8[7] == 0xe1) {
			printf("big endian\n");
		} else if (p_u8[0] == 0xe1 && p_u8[1] == 0xd0 && p_u8[2] == 0xee && p_u8[3] == 0x95 &&
		           p_u8[4] == 0xbb && p_u8[5] == 0xeb && p_u8[6] == 0x78 && p_u8[7] == 0x43) {
			printf("little endian\n");
		} else if (p_u8[0] == 0xbb && p_u8[1] == 0xeb && p_u8[2] == 0x78 && p_u8[3] == 0x43 &&
		           p_u8[4] == 0xe1 && p_u8[5] == 0xd0 && p_u8[6] == 0xee && p_u8[7] == 0x95) {
			printf("mixed endian\n");
		} else {
			printf("unknown endian\n");
		}
	} else {
		goto usage;
	}

	exit(0);

 usage:
	fprintf(stderr, "Usage: ./check_align (uint16_t|uint32_t|uint64_t|double) <offset>\n");
	exit(1);
}
