/*
 *  Problem with clang 3.3 (and 4.0) on FreeBSD; seems to be fixed in
 *  Clang 5.0 at least:
 *
 *    $ clang -v
 *    FreeBSD clang version 3.3 (tags/RELEASE_33/final 183502) 20130610
 *    Target: x86_64-unknown-freebsd10.0
 *    Thread model: posix
 *
 *  Possible root cause:
 *
 *    https://bugs.llvm.org/show_bug.cgi?id=32056
 *    https://reviews.llvm.org/D33328
 *    More discussion: https://github.com/svaarala/duktape/issues/1752
 *
 *  The problem manifests itself as follows (x86_64):
 *
 *    $ clang -Os -m32 -std=c99 -fstrict-aliasing -fomit-frame-pointer clang_aliasing.c
 *    $ ./a.out
 *    11 22 33 44 00 00 f1 ff 
 *    11 22 33 44 00 00 f9 ff   <==
 *    11 22 33 44 00 00 f9 ff   <==
 *    11 22 33 44 00 00 f1 ff 
 *
 *  The value is corrupted even in the base case where 'a' is initialized
 *  as bytes and then copied with a structural assignment; the double part
 *  of the union is not accessed at all.  The corruption is always that the
 *  7th byte gets OR'd with 0x08.  This bit is the highest bit of the IEEE
 *  double mantissa, so this is probably floating point related.  The code
 *  generated uses a floating point load/store to copy the value except in
 *  the cases where the copy works.
 *
 *  The asm code generated for the failing assignment and the COPY_FAILS
 *  case is the same: a floating point load/store is used to copy the union
 *  contents.  This shows that COPY_FAILS is indeed converted automatically
 *  from a byte/memory copy to a floating point load/store):
 *
 *    8048653:       dd 44 24 20             fldl   0x20(%esp)
 *    8048657:       dd 5c 24 10             fstpl  0x10(%esp)
 *    804865b:       8d 44 24 10             lea    0x10(%esp),%eax
 *    804865f:       89 04 24                mov    %eax,(%esp)
 *    8048662:       e8 61 ff ff ff          call   80485c8 <dump>
 *
 *  Apparently an FLD + FSTP sequence causes a signaling NaN (with mantissa
 *  highest bit cleared) to be converted into a quiet NaN (with mantissa 
 *  highest bit set) so the generated code is incorrect.  See for instance:
 *
 *    http://caml.inria.fr/mantis/view.php?id=5038
 *
 *  The COPY_WORKS case looks like:
 *
 *    8048667:       c7 44 24 04 00 00 00    movl   $0x0,0x4(%esp)
 *    804866e:       00 
 *    804866f:       8b 44 24 04             mov    0x4(%esp),%eax
 *    8048673:       83 f8 07                cmp    $0x7,%eax
 *    8048676:       7f 1d                   jg     8048695 <main+0x96>
 *    8048678:       8b 44 24 04             mov    0x4(%esp),%eax
 *    804867c:       8a 44 04 20             mov    0x20(%esp,%eax,1),%al
 *    8048680:       8b 4c 24 04             mov    0x4(%esp),%ecx
 *    8048684:       88 44 0c 08             mov    %al,0x8(%esp,%ecx,1)
 *    8048688:       ff 44 24 04             incl   0x4(%esp)
 *    804868c:       8b 44 24 04             mov    0x4(%esp),%eax
 *    8048690:       83 f8 08                cmp    $0x8,%eax
 *    8048693:       7c e3                   jl     8048678 <main+0x79>
 *    8048695:       8d 44 24 08             lea    0x8(%esp),%eax
 *    8048699:       89 04 24                mov    %eax,(%esp)
 *    804869c:       e8 27 ff ff ff          call   80485c8 <dump>
 */

#include <stdio.h>
#include <stdint.h>

typedef union { double d; uint8_t c[8]; } my_union;

void dump(my_union *u) {
	int i;
	for (i = 0; i < 8; i++) { printf("%02x ", (int) u->c[i]); }
	printf("\n");
}

#define  COPY_FAILS(s,d) { int _i; for(_i = 0; _i < 8; _i++) { \
		((uint8_t *) (d))[_i] = ((uint8_t *) (s))[_i]; \
	} }

#define  COPY_WORKS(s,d) { volatile int _i; for(_i = 0; _i < 8; _i++) { \
		((uint8_t *) (d))[_i] = ((uint8_t *) (s))[_i]; \
	} }

int main(int argc, char *argv[]) {
	my_union a, b, c, d;

	/* Initial value, the contents of bytes 6 and 7 seem to matter */
	a.c[0] = 0x11; a.c[1] = 0x22; a.c[2] = 0x33; a.c[3] = 0x44;
	a.c[4] = 0x00; a.c[5] = 0x00; a.c[6] = 0xf1; a.c[7] = 0xff;
	dump(&a);

	/* Copy with assignment (does not work) */
	b = a; dump(&b);

	/* Copy with a byte-by-byte macro (does not work) */
	COPY_FAILS(&a, &c); dump(&c);

	/* Copy with a byte-by-byte macro (volatile loop variable - works) */
	COPY_WORKS(&a, &d); dump(&d);

	return 0;
}
