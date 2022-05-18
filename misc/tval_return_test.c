/* Test return by value or write to out pointer. */

/* For gcc 9.4.0 -O2 (loop highlighted):

0000000000001170 <run1>:
    1170:       b8 00 e1 f5 05          mov    $0x5f5e100,%eax
    1175:       0f 1f 00                nopl   (%rax)
===============================================================================
    1178:       c7 44 24 e8 7b 00 00    movl   $0x7b,-0x18(%rsp)
    117f:       00
    1180:       f2 0f 10 05 88 2e 00    movsd  0x2e88(%rip),%xmm0        # 4010 <dbl>
    1187:       00
    1188:       f2 0f 11 44 24 f0       movsd  %xmm0,-0x10(%rsp)
    118e:       83 e8 01                sub    $0x1,%eax
    1191:       75 e5                   jne    1178 <run1+0x8>
===============================================================================
    1193:       8b 54 24 e8             mov    -0x18(%rsp),%edx
    1197:       48 8d 35 66 0e 00 00    lea    0xe66(%rip),%rsi        # 2004 <_IO_stdin_used+0x4>
    119e:       bf 01 00 00 00          mov    $0x1,%edi
    11a3:       31 c0                   xor    %eax,%eax
    11a5:       e9 a6 fe ff ff          jmpq   1050 <__printf_chk@plt>
    11aa:       66 0f 1f 44 00 00       nopw   0x0(%rax,%rax,1)

00000000000011b0 <run2>:
    11b0:       48 bf 00 00 00 00 ff    movabs $0xffffffff00000000,%rdi
    11b7:       ff ff ff
    11ba:       be 00 e1 f5 05          mov    $0x5f5e100,%esi
    11bf:       90                      nop
===============================================================================
    11c0:       48 89 c1                mov    %rax,%rcx
    11c3:       f2 0f 10 05 45 2e 00    movsd  0x2e45(%rip),%xmm0        # 4010 <dbl>
    11ca:       00
    11cb:       48 21 f9                and    %rdi,%rcx
    11ce:       48 83 c9 7b             or     $0x7b,%rcx
    11d2:       48 89 4c 24 e8          mov    %rcx,-0x18(%rsp)
    11d7:       48 89 c8                mov    %rcx,%rax
    11da:       66 0f d6 44 24 f0       movq   %xmm0,-0x10(%rsp)
    11e0:       83 ee 01                sub    $0x1,%esi
    11e3:       75 db                   jne    11c0 <run2+0x10>
===============================================================================
    11e5:       8b 54 24 e8             mov    -0x18(%rsp),%edx
    11e9:       48 8d 35 14 0e 00 00    lea    0xe14(%rip),%rsi        # 2004 <_IO_stdin_used+0x4>
    11f0:       bf 01 00 00 00          mov    $0x1,%edi
    11f5:       31 c0                   xor    %eax,%eax
    11f7:       e9 54 fe ff ff          jmpq   1050 <__printf_chk@plt>
    11fc:       0f 1f 40 00             nopl   0x0(%rax)
*/

#include <stdio.h>

typedef struct {
	int t;
	union {
		double d;
		void *p;
	} u;
} tval;

volatile double dbl = 123.0;

static void test1(volatile tval *out) {
	out->t = 123;
	out->u.d = dbl;
}

static tval test2(void) {
	tval tv;
	tv.t = 123;
	tv.u.d = dbl;
	return tv;
}

static __attribute__((noinline)) void run1(void) {
	volatile tval tv1;
	int i;

	for (i = 0; i < 100000000; i++) {
#if 1
		test1(&tv1);
#endif
	}
	printf("%d\n", tv1.t);

}

static __attribute__((noinline)) void run2(void) {
	volatile tval tv2;
	int i;

	for (i = 0; i < 100000000; i++) {
#if 1
		tv2 = test2();
#endif
	}
	printf("%d\n", tv2.t);
}

int main(int argc, const char *argv[]) {
	(void) argc;
	(void) argv;

	run1();
	run2();
	return 0;
}
