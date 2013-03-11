/*
 *  Provides the duk_rdtsc() inline function (if available).
 *
 *  See: http://www.mcs.anl.gov/~kazutomo/rdtsc.html
 */

#ifndef DUK_RDTSC_H_INCLUDED
#define DUK_RDTSC_H_INCLUDED

#if defined(__i386__)
static __inline__ unsigned long long duk_rdtsc(void) {
	unsigned long long int x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
}
#define  DUK_RDTSC_AVAILABLE 1
#elif defined(__x86_64__)
static __inline__ unsigned long long duk_rdtsc(void) {
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ((unsigned long long) lo) | (((unsigned long long) hi) << 32);
}
#define  DUK_RDTSC_AVAILABLE 1
#else
/* not available */
#undef  DUK_RDTSC_AVAILABLE
#endif

#endif  /* DUK_RDTSC_H_INCLUDED */

