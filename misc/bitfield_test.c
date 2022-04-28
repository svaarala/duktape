/* Test bitfield patterns. */

/*
 *  1050:       48 83 ec 08             sub    $0x8,%rsp
 *  1054:       8b 05 d6 2f 00 00       mov    0x2fd6(%rip),%eax        # 4030 <val>
 *  105a:       f6 c4 01                test   $0x1,%ah
 *  105d:       75 4c                   jne    10ab <main+0x5b>
 *  105f:       8b 05 cb 2f 00 00       mov    0x2fcb(%rip),%eax        # 4030 <val>
 *  1065:       f6 c4 01                test   $0x1,%ah
 *  1068:       74 6b                   je     10d5 <main+0x85>
 *  106a:       8b 05 c0 2f 00 00       mov    0x2fc0(%rip),%eax        # 4030 <val>
 *  1070:       f6 c4 0f                test   $0xf,%ah
 *  1073:       74 52                   je     10c7 <main+0x77>
 *  1075:       8b 05 b5 2f 00 00       mov    0x2fb5(%rip),%eax        # 4030 <val>
 *  107b:       f6 c4 0f                test   $0xf,%ah
 *  107e:       75 39                   jne    10b9 <main+0x69>
 *  1080:       8b 05 aa 2f 00 00       mov    0x2faa(%rip),%eax        # 4030 <val>
 *  1086:       25 00 0f 00 00          and    $0xf00,%eax
 *  108b:       3d 00 0f 00 00          cmp    $0xf00,%eax
 *  1090:       74 5f                   je     10f1 <main+0xa1>
 *  1092:       8b 05 98 2f 00 00       mov    0x2f98(%rip),%eax        # 4030 <val>
 *  1098:       25 00 0f 00 00          and    $0xf00,%eax
 *  109d:       3d 00 0c 00 00          cmp    $0xc00,%eax
 *  10a2:       74 3f                   je     10e3 <main+0x93>
 *  10a4:       31 c0                   xor    %eax,%eax
 *  10a6:       48 83 c4 08             add    $0x8,%rsp
 *  10aa:       c3                      retq
 */

#include <stdio.h>

volatile int val = 0xdeadbeef;

int main(int argc, const char *argv[]) {
	/* Single-bit test for 0 or 1: single TEST on x64. */
	if ((val & 0x100) != 0) {
		printf("1\n");
	}
	if ((val & 0x100) == 0) {
		printf("2\n");
	}

	/* Multi-bit test for all zeroes: single TEST on x64. */
	if ((val & 0xf00) == 0) {
		printf("3\n");
	}

	/* Multi-bit test for not all zeroes (>= one-bit): single TEST on x64. */
	if ((val & 0xf00) != 0) {
		printf("3\n");
	}

	/* Multi-bit test for all ones: AND + CMP on x64. */
	if ((val & 0xf00) == 0xf00) {
		printf("4\n");
	}

	/* Multi-bit test for arbitrary value: AND + CMP on x64. */
	if ((val & 0xf00) == 0xc00) {
		printf("5\n");
	}

	return 0;
}
