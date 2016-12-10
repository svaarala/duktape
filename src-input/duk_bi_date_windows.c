/*
 *  Windows Date providers
 *
 *  Platform specific links:
 *
 *    - http://msdn.microsoft.com/en-us/library/windows/desktop/ms725473(v=vs.85).aspx
 */

#include "duk_internal.h"

/* The necessary #includes are in place in duk_config.h. */

#if defined(DUK_USE_DATE_NOW_WINDOWS) || defined(DUK_USE_DATE_TZO_WINDOWS)
/* Shared Windows helpers. */
DUK_LOCAL void duk__convert_systime_to_ularge(const SYSTEMTIME *st, ULARGE_INTEGER *res) {
	FILETIME ft;
	if (SystemTimeToFileTime(st, &ft) == 0) {
		DUK_D(DUK_DPRINT("SystemTimeToFileTime() failed, returning 0"));
		res->QuadPart = 0;
	} else {
		res->LowPart = ft.dwLowDateTime;
		res->HighPart = ft.dwHighDateTime;
	}
}
DUK_LOCAL void duk__set_systime_jan1970(SYSTEMTIME *st) {
	DUK_MEMZERO((void *) st, sizeof(*st));
	st->wYear = 1970;
	st->wMonth = 1;
	st->wDayOfWeek = 4;  /* not sure whether or not needed; Thursday */
	st->wDay = 1;
	DUK_ASSERT(st->wHour == 0);
	DUK_ASSERT(st->wMinute == 0);
	DUK_ASSERT(st->wSecond == 0);
	DUK_ASSERT(st->wMilliseconds == 0);
}
#endif  /* defined(DUK_USE_DATE_NOW_WINDOWS) || defined(DUK_USE_DATE_TZO_WINDOWS) */

#if defined(DUK_USE_DATE_NOW_WINDOWS)
DUK_INTERNAL duk_double_t duk_bi_date_get_now_windows(duk_context *ctx) {
	/* Suggested step-by-step method from documentation of RtlTimeToSecondsSince1970:
	 * http://msdn.microsoft.com/en-us/library/windows/desktop/ms724928(v=vs.85).aspx
	 */
	SYSTEMTIME st1, st2;
	ULARGE_INTEGER tmp1, tmp2;

	DUK_UNREF(ctx);

	GetSystemTime(&st1);
	duk__convert_systime_to_ularge((const SYSTEMTIME *) &st1, &tmp1);

	duk__set_systime_jan1970(&st2);
	duk__convert_systime_to_ularge((const SYSTEMTIME *) &st2, &tmp2);

	/* Difference is in 100ns units, convert to milliseconds w/o fractions */
	return (duk_double_t) ((tmp1.QuadPart - tmp2.QuadPart) / 10000LL);
}
#endif  /* DUK_USE_DATE_NOW_WINDOWS */


#if defined(DUK_USE_DATE_TZO_WINDOWS)
DUK_INTERNAL_DECL duk_int_t duk_bi_date_get_local_tzoffset_windows(duk_double_t d) {
	SYSTEMTIME st1;
	SYSTEMTIME st2;
	SYSTEMTIME st3;
	ULARGE_INTEGER tmp1;
	ULARGE_INTEGER tmp2;
	ULARGE_INTEGER tmp3;
	FILETIME ft1;

	/* XXX: handling of timestamps outside Windows supported range.
	 * How does Windows deal with dates before 1600?  Does windows
	 * support all Ecmascript years (like -200000 and +200000)?
	 * Should equivalent year mapping be used here too?  If so, use
	 * a shared helper (currently integrated into timeval-to-parts).
	 */

	/* Use the approach described in "Remarks" of FileTimeToLocalFileTime:
	 * http://msdn.microsoft.com/en-us/library/windows/desktop/ms724277(v=vs.85).aspx
	 */

	duk__set_systime_jan1970(&st1);
	duk__convert_systime_to_ularge((const SYSTEMTIME *) &st1, &tmp1);
	tmp2.QuadPart = (ULONGLONG) (d * 10000.0);  /* millisec -> 100ns units since jan 1, 1970 */
	tmp2.QuadPart += tmp1.QuadPart;             /* input 'd' in Windows UTC, 100ns units */

	ft1.dwLowDateTime = tmp2.LowPart;
	ft1.dwHighDateTime = tmp2.HighPart;
	FileTimeToSystemTime((const FILETIME *) &ft1, &st2);
	if (SystemTimeToTzSpecificLocalTime((LPTIME_ZONE_INFORMATION) NULL, &st2, &st3) == 0) {
		DUK_D(DUK_DPRINT("SystemTimeToTzSpecificLocalTime() failed, return tzoffset 0"));
		return 0;
	}
	duk__convert_systime_to_ularge((const SYSTEMTIME *) &st3, &tmp3);

	/* Positive if local time ahead of UTC. */
	return (duk_int_t) (((LONGLONG) tmp3.QuadPart - (LONGLONG) tmp2.QuadPart) / 10000000LL);  /* seconds */
}
#endif  /* DUK_USE_DATE_TZO_WINDOWS */
