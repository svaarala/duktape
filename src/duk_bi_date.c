/*
 *  Date built-ins
 *
 *  Unlike most built-ins, Date has a lot of platform dependencies for
 *  getting UTC time, converting between UTC and local time, and parsing
 *  and formatting time values.
 *
 *  See doc/datetime.txt.
 *
 *  Platform specific links:
 *
 *    - http://msdn.microsoft.com/en-us/library/windows/desktop/ms725473(v=vs.85).aspx
 */

#include "duk_internal.h"

/*
 *  Platform specific includes and defines
 *
 *  Note that necessary system headers (like <sys/time.h>) are included
 *  by duk_internal.h (or duk_features.h, which is included by duk_internal.h)
 *  because the header locations vary between systems and we don't want
 *  that clutter here.
 */

#define DUK__GET_NOW_TIMEVAL      duk_bi_date_get_now
#define DUK__GET_LOCAL_TZOFFSET   duk__get_local_tzoffset

/* Buffer sizes for some UNIX calls.  Larger than strictly necessary
 * to avoid Valgrind errors.
 */
#define DUK__STRPTIME_BUF_SIZE  64
#define DUK__STRFTIME_BUF_SIZE  64

/*
 *  Other file level defines
 */

/* Forward declarations. */
static double duk__push_this_get_timeval_tzoffset(duk_context *ctx, int flags, int *out_tzoffset);
static double duk__push_this_get_timeval(duk_context *ctx, int flags);
static void duk__timeval_to_parts(double d, int *parts, double *dparts, int flags);
static double duk__get_timeval_from_dparts(double *dparts, int flags);
static void duk__twodigit_year_fixup(duk_context *ctx, int idx_val);

/* Millisecond count constants. */
#define DUK__MS_SECOND          1000
#define DUK__MS_MINUTE          (60 * 1000)
#define DUK__MS_HOUR            (60 * 60 * 1000)
#define DUK__MS_DAY             (24 * 60 * 60 * 1000)

/* Part indices for internal breakdowns.  Part order from DUK__IDX_YEAR to
 * DUK__IDX_MILLISECOND matches argument ordering of Ecmascript API calls
 * (like Date constructor call).  A few functions in this file depend
 * on the specific ordering, so change with care.
 *
 * (Must be in-sync with genbuiltins.py.)
 */
#define DUK__IDX_YEAR           0  /* year */
#define DUK__IDX_MONTH          1  /* month: 0 to 11 */
#define DUK__IDX_DAY            2  /* day within month: 0 to 30 */
#define DUK__IDX_HOUR           3
#define DUK__IDX_MINUTE         4
#define DUK__IDX_SECOND         5
#define DUK__IDX_MILLISECOND    6
#define DUK__IDX_WEEKDAY        7  /* weekday: 0 to 6, 0=sunday, 1=monday, etc */
#define DUK__NUM_PARTS          8

/* Internal API call flags, used for various functions in this file.
 * Certain flags are used by only certain functions, but since the flags
 * don't overlap, a single flags value can be passed around to multiple
 * functions.
 *
 * The unused top bits of the flags field are also used to pass values
 * to helpers (duk__get_part_helper() and duk__set_part_helper()).
 *
 * (Must be in-sync with genbuiltins.py.)
 */
#define DUK__FLAG_NAN_TO_ZERO          (1 << 0)  /* timeval breakdown: internal time value NaN -> zero */
#define DUK__FLAG_NAN_TO_RANGE_ERROR   (1 << 1)  /* timeval breakdown: internal time value NaN -> RangeError (toISOString) */
#define DUK__FLAG_ONEBASED             (1 << 2)  /* timeval breakdown: convert month and day-of-month parts to one-based (default is zero-based) */
#define DUK__FLAG_LOCALTIME            (1 << 3)  /* convert time value to local time */
#define DUK__FLAG_SUB1900              (1 << 4)  /* getter: subtract 1900 from year when getting year part */
#define DUK__FLAG_TOSTRING_DATE        (1 << 5)  /* include date part in string conversion result */
#define DUK__FLAG_TOSTRING_TIME        (1 << 6)  /* include time part in string conversion result */
#define DUK__FLAG_TOSTRING_LOCALE      (1 << 7)  /* use locale specific formatting if available */
#define DUK__FLAG_TIMESETTER           (1 << 8)  /* setter: call is a time setter (affects hour, min, sec, ms); otherwise date setter (affects year, month, day-in-month) */
#define DUK__FLAG_YEAR_FIXUP           (1 << 9)  /* setter: perform 2-digit year fixup (00...99 -> 1900...1999) */
#define DUK__FLAG_SEP_T                (1 << 10) /* string conversion: use 'T' instead of ' ' as a separator */

/*
 *  Platform specific helpers
 */

#ifdef DUK_USE_DATE_NOW_GETTIMEOFDAY
/* Get current Ecmascript time (= UNIX/Posix time, but in milliseconds). */
duk_double_t duk_bi_date_get_now(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	struct timeval tv;
	double d;

	if (gettimeofday(&tv, NULL) != 0) {
		DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "gettimeofday failed");
	}

	d = ((double) tv.tv_sec) * 1000.0 +
	    ((double) (tv.tv_usec / 1000));
	DUK_ASSERT(DUK_FLOOR(d) == d);  /* no fractions */

	return d;
}
#endif  /* DUK_USE_DATE_NOW_GETTIMEOFDAY */

#ifdef DUK_USE_DATE_NOW_TIME
/* Not a very good provider: only full seconds are available. */
duk_double_t duk_bi_date_get_now(duk_context *ctx) {
	time_t t = time(NULL);
	return ((double) t) * 1000.0;
}
#endif  /* DUK_USE_DATE_NOW_TIME */

#if defined(DUK_USE_DATE_NOW_WINDOWS) || defined(DUK_USE_DATE_TZO_WINDOWS)
/* Shared Windows helpers. */
static void duk__convert_systime_to_ularge(const SYSTEMTIME *st, ULARGE_INTEGER *res) {
	FILETIME ft;
	if (SystemTimeToFileTime(st, &ft) == 0) {
		DUK_D(DUK_DPRINT("SystemTimeToFileTime() failed, returning 0"));
		res->QuadPart = 0;
	} else {
		res->LowPart = ft.dwLowDateTime;
		res->HighPart = ft.dwHighDateTime;
	}
}
static void duk__set_systime_jan1970(SYSTEMTIME *st) {
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

#ifdef DUK_USE_DATE_NOW_WINDOWS
duk_double_t duk_bi_date_get_now(duk_context *ctx) {
	/* Suggested step-by-step method from documentation of RtlTimeToSecondsSince1970:
	 * http://msdn.microsoft.com/en-us/library/windows/desktop/ms724928(v=vs.85).aspx
	 */
	SYSTEMTIME st1, st2;
	ULARGE_INTEGER tmp1, tmp2;

	GetSystemTime(&st1);
	duk__convert_systime_to_ularge((const SYSTEMTIME *) &st1, &tmp1);

	duk__set_systime_jan1970(&st2);
	duk__convert_systime_to_ularge((const SYSTEMTIME *) &st2, &tmp2);

	/* Difference is in 100ns units, convert to milliseconds */
	return (double) ((tmp1.QuadPart - tmp2.QuadPart) / 10000LL);
}
#endif  /* DUK_USE_DATE_NOW_WINDOWS */

#if defined(DUK_USE_DATE_TZO_GMTIME) || defined(DUK_USE_DATE_TZO_GMTIME_R)
/* Get local time offset (in seconds) for a certain (UTC) instant 'd'. */
static int duk__get_local_tzoffset(double d) {
	time_t t, t1, t2;
	int parts[DUK__NUM_PARTS];
	double dparts[DUK__NUM_PARTS];
	struct tm tms[2];
#ifdef DUK_USE_DATE_TZO_GMTIME
	struct tm *tm_ptr;
#endif

	/* For NaN/inf, the return value doesn't matter. */
	if (!DUK_ISFINITE(d)) {
		return 0;
	}

	/*
	 *  This is a bit tricky to implement portably.  The result depends
	 *  on the timestamp (specifically, DST depends on the timestamp).
	 *  If e.g. UNIX APIs are used, they'll have portability issues with
	 *  very small and very large years.
	 *
	 *  Current approach:
	 *
	 *  - Clamp year to stay within portable UNIX limits.  Avoid 2038 as
	 *    some conversions start to fail.  Avoid 1970, as some conversions
	 *    in January 1970 start to fail (verified in practice).
	 *
	 *  - Create a UTC time breakdown from 't', and then pretend it is a
	 *    local time breakdown and build a UTC time from it.  The timestamp
	 *    will effectively shift backwards by time the time offset (e.g. -2h
	 *    or -3h for EET/EEST).  Convert with mktime() twice to get the DST
	 *    flag for the final conversion.
	 *
	 *  FIXME: this is probably not entirely correct nor clear, but is
	 *  good enough for now.
	 */

	duk__timeval_to_parts(d, parts, dparts, 0 /*flags*/);

	/*
	 *  FIXME: must choose 'equivalent year', E5 Section 15.9.1.8, instead
	 *  of just clamping.
	 */
	if (parts[DUK__IDX_YEAR] < 1971) {
		dparts[DUK__IDX_YEAR] = 1971.0;
	} else if (parts[DUK__IDX_YEAR] > 2037) {
		dparts[DUK__IDX_YEAR] = 2037.0;
	}

	d = duk__get_timeval_from_dparts(dparts, 0 /*flags*/);
	DUK_ASSERT(d >= 0 && d < 2147483648.0 * 1000.0);  /* unsigned 31-bit range */
	t = (size_t) (d / 1000.0);
	DUK_DDD(DUK_DDDPRINT("timeval: %lf -> time_t %d", d, (int) t));

	t1 = t;

	DUK_MEMZERO((void *) tms, sizeof(struct tm) * 2);

#if defined(DUK_USE_DATE_TZO_GMTIME_R)
	(void) gmtime_r(&t, &tms[0]);
#elif defined(DUK_USE_DATE_TZO_GMTIME)
	tm_ptr = gmtime(&t);
	DUK_MEMCPY((void *) &tms[0], tm_ptr, sizeof(struct tm));
#else
#error internal error
#endif
	DUK_MEMCPY((void *) &tms[1], &tms[0], sizeof(struct tm));

	DUK_DDD(DUK_DDDPRINT("before mktime: tm={sec:%d,min:%d,hour:%d,mday:%d,mon:%d,year:%d,"
	                     "wday:%d,yday:%d,isdst:%d}",
	                     (int) tms[0].tm_sec, (int) tms[0].tm_min, (int) tms[0].tm_hour,
	                     (int) tms[0].tm_mday, (int) tms[0].tm_mon, (int) tms[0].tm_year,
	                     (int) tms[0].tm_wday, (int) tms[0].tm_yday, (int) tms[0].tm_isdst));

	(void) mktime(&tms[0]);
	tms[1].tm_isdst = tms[0].tm_isdst;
	t2 = mktime(&tms[1]);
	DUK_ASSERT_DISABLE(t2 >= 0);  /* On some platforms time_t is unsigned and this would cause a warning */
	if (t2 == (time_t) -1) {
		/* This check used to be for (t2 < 0) but on some platforms
		 * time_t is unsigned and apparently the proper way to detect
		 * an mktime() error return is the cast above.  See e.g.:
		 * http://pubs.opengroup.org/onlinepubs/009695299/functions/mktime.html
		 */
		goto error;
	}

	DUK_DDD(DUK_DDDPRINT("after mktime: tm={sec:%d,min:%d,hour:%d,mday:%d,mon:%d,year:%d,"
	                     "wday:%d,yday:%d,isdst:%d}",
	                     (int) tms[1].tm_sec, (int) tms[1].tm_min, (int) tms[1].tm_hour,
	                     (int) tms[1].tm_mday, (int) tms[1].tm_mon, (int) tms[1].tm_year,
	                     (int) tms[1].tm_wday, (int) tms[1].tm_yday, (int) tms[1].tm_isdst));
	DUK_DDD(DUK_DDDPRINT("t2=%d", (int) t2));

	/* Positive if local time ahead of UTC. */

	/* difftime() returns a double, so coercion to int generates quite
	 * a lot of code.  Direct subtraction is not portable, however.
	 *
	 * FIXME: allow direct subtraction on known platforms.
	 */
#if 0
	return t1 - t2;
#endif
	return (int) difftime(t1, t2);

 error:
	/* FIXME: return something more useful, so that caller can throw? */
	DUK_D(DUK_DPRINT("mktime() failed, d=%lf", d));
	return 0;
}
#endif  /* DUK_USE_DATE_TZO_GMTIME */

#if defined(DUK_USE_DATE_TZO_WINDOWS)
static int duk__get_local_tzoffset(double d) {
	SYSTEMTIME st1;
	SYSTEMTIME st2;
	SYSTEMTIME st3;
	ULARGE_INTEGER tmp1;
	ULARGE_INTEGER tmp2;
	ULARGE_INTEGER tmp3;
	FILETIME ft1;

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
	return (int) (((LONGLONG) tmp3.QuadPart - (LONGLONG) tmp2.QuadPart) / 10000000LL);  /* seconds */
}
#endif  /* DUK_USE_DATE_TZO_WINDOWS */

#ifdef DUK_USE_DATE_PRS_STRPTIME
static int duk__parse_string_strptime(duk_context *ctx, const char *str) {
	struct tm tm;
	time_t t;
	char buf[DUK__STRPTIME_BUF_SIZE];

	/* copy to buffer with spare to avoid Valgrind gripes from strptime */
	DUK_ASSERT(str != NULL);
	DUK_MEMZERO(buf, sizeof(buf));  /* valgrind whine without this */
	DUK_SNPRINTF(buf, sizeof(buf), "%s", str);
	buf[sizeof(buf) - 1] = (char) 0;

	DUK_DDD(DUK_DDDPRINT("parsing: '%s'", buf));

	DUK_MEMZERO(&tm, sizeof(tm));
	if (strptime((const char *) buf, "%c", &tm) != NULL) {
		DUK_DDD(DUK_DDDPRINT("before mktime: tm={sec:%d,min:%d,hour:%d,mday:%d,mon:%d,year:%d,"
		                     "wday:%d,yday:%d,isdst:%d}",
		                     (int) tm.tm_sec, (int) tm.tm_min, (int) tm.tm_hour,
		                     (int) tm.tm_mday, (int) tm.tm_mon, (int) tm.tm_year,
		                      (int) tm.tm_wday, (int) tm.tm_yday, (int) tm.tm_isdst));
		tm.tm_isdst = -1;  /* negative: dst info not available */

		t = mktime(&tm);
		DUK_DDD(DUK_DDDPRINT("mktime() -> %d", (int) t));
		if (t >= 0) {
			duk_push_number(ctx, ((double) t) * 1000.0);
			return 1;
		}
	}

	return 0;
}
#endif  /* DUK_USE_DATE_PRS_STRPTIME */

#ifdef DUK_USE_DATE_PRS_GETDATE
static int duk__parse_string_getdate(duk_context *ctx, const char *str) {
	struct tm tm;
	int rc;
	time_t t;

	/* For this to work, DATEMSK must be set, to this is not very
	 * convenient for an embeddable interpreter.
	 */

	DUK_MEMZERO(&tm, sizeof(struct tm));
	rc = getdate_r(str, &tm);
	DUK_DDD(DUK_DDDPRINT("getdate_r() -> %d", rc));

	if (rc == 0) {
		t = mktime(&tm);
		DUK_DDD(DUK_DDDPRINT("mktime() -> %d", (int) t));
		if (t >= 0) {
			duk_push_number(ctx, (double) t);
			return 1;
		}
	}

	duk_push_nan(ctx);
	return 1;
}
#endif  /* DUK_USE_DATE_PRS_GETDATE */

#ifdef DUK_USE_DATE_FMT_STRFTIME
static int duk__format_parts_strftime(duk_context *ctx, int *parts, int tzoffset, int flags) {
	char buf[DUK__STRFTIME_BUF_SIZE];
	struct tm tm;
	const char *fmt;

	DUK_UNREF(tzoffset);

	/* If platform doesn't support the entire Ecmascript range, we need to
	 * return 0 so that the caller can fall back to the default formatter.
	 *
	 * FIXME: how to detect this more correctly?  add a feature define?
	 * The size of time_t is probably not an accurate guarantee of strftime
	 * supporting or not supporting a large time range.
	 */
	if (sizeof(time_t) < 8 &&
	   (parts[DUK__IDX_YEAR] < 1970 || parts[DUK__IDX_YEAR] > 2037)) {
		/* be paranoid for 32-bit time values (even avoiding negative ones) */
		return 0;
	}

	DUK_MEMZERO(&tm, sizeof(tm));
	tm.tm_sec = parts[DUK__IDX_SECOND];
	tm.tm_min = parts[DUK__IDX_MINUTE];
	tm.tm_hour = parts[DUK__IDX_HOUR];
	tm.tm_mday = parts[DUK__IDX_DAY];       /* already one-based */
	tm.tm_mon = parts[DUK__IDX_MONTH] - 1;  /* one-based -> zero-based */
	tm.tm_year = parts[DUK__IDX_YEAR] - 1900;
	tm.tm_wday = parts[DUK__IDX_WEEKDAY];
	tm.tm_isdst = 0;

	DUK_MEMZERO(buf, sizeof(buf));
	if ((flags & DUK__FLAG_TOSTRING_DATE) && (flags & DUK__FLAG_TOSTRING_TIME)) {
		fmt = "%c";
	} else if (flags & DUK__FLAG_TOSTRING_DATE) {
		fmt = "%x";
	} else {
		DUK_ASSERT(flags & DUK__FLAG_TOSTRING_TIME);
		fmt = "%X";
	}
	(void) strftime(buf, sizeof(buf) - 1, fmt, &tm);
	DUK_ASSERT(buf[sizeof(buf) - 1] == 0);

	duk_push_string(ctx, buf);
	return 1;
}
#endif  /* DUK_USE_DATE_FMT_STRFTIME */

/*
 *  ISO 8601 subset parser.
 */

/* Parser part count. */
#define DUK__NUM_ISO8601_PARSER_PARTS  9

/* Parser part indices. */
#define DUK__PI_YEAR         0
#define DUK__PI_MONTH        1
#define DUK__PI_DAY          2
#define DUK__PI_HOUR         3
#define DUK__PI_MINUTE       4
#define DUK__PI_SECOND       5
#define DUK__PI_MILLISECOND  6
#define DUK__PI_TZHOUR       7
#define DUK__PI_TZMINUTE     8

/* Parser part masks. */
#define DUK__PM_YEAR         (1 << DUK__PI_YEAR)
#define DUK__PM_MONTH        (1 << DUK__PI_MONTH)
#define DUK__PM_DAY          (1 << DUK__PI_DAY)
#define DUK__PM_HOUR         (1 << DUK__PI_HOUR)
#define DUK__PM_MINUTE       (1 << DUK__PI_MINUTE)
#define DUK__PM_SECOND       (1 << DUK__PI_SECOND)
#define DUK__PM_MILLISECOND  (1 << DUK__PI_MILLISECOND)
#define DUK__PM_TZHOUR       (1 << DUK__PI_TZHOUR)
#define DUK__PM_TZMINUTE     (1 << DUK__PI_TZMINUTE)

/* Parser separator indices. */
#define DUK__SI_PLUS         0
#define DUK__SI_MINUS        1
#define DUK__SI_T            2
#define DUK__SI_SPACE        3
#define DUK__SI_COLON        4
#define DUK__SI_PERIOD       5
#define DUK__SI_Z            6
#define DUK__SI_NUL          7

/* Parser separator masks. */
#define DUK__SM_PLUS         (1 << DUK__SI_PLUS)
#define DUK__SM_MINUS        (1 << DUK__SI_MINUS)
#define DUK__SM_T            (1 << DUK__SI_T)
#define DUK__SM_SPACE        (1 << DUK__SI_SPACE)
#define DUK__SM_COLON        (1 << DUK__SI_COLON)
#define DUK__SM_PERIOD       (1 << DUK__SI_PERIOD)
#define DUK__SM_Z            (1 << DUK__SI_Z)
#define DUK__SM_NUL          (1 << DUK__SI_NUL)

/* Rule control flags. */
#define DUK__CF_NEG          (1 << 0)  /* continue matching, set neg_tzoffset flag */
#define DUK__CF_ACCEPT       (1 << 1)  /* accept string */
#define DUK__CF_ACCEPT_NUL   (1 << 2)  /* accept string if next char is NUL (otherwise reject) */

#define DUK__PACK_RULE(partmask,sepmask,nextpart,flags)  \
	((partmask) + ((sepmask) << 9) + ((nextpart) << 17) + ((flags) << 21))

#define DUK__UNPACK_RULE(rule,var_nextidx,var_flags)  do { \
		(var_nextidx) = ((rule) >> 17) & 0x0f; \
		(var_flags) = (rule) >> 21; \
	} while (0)

#define DUK__RULE_MASK_PART_SEP  0x1ffff

/* Matching separator index is used in the control table */
static const char duk__parse_iso8601_seps[] = {
	'+' /*0*/, '-' /*1*/, 'T' /*2*/, ' ' /*3*/,
	':' /*4*/, '.' /*5*/, 'Z' /*6*/, (char) 0 /*7*/
};

/* Rule table: first matching rule is used to determine what to do next. */
static const int duk__parse_iso8601_control[] = {
	DUK__PACK_RULE(DUK__PM_YEAR, DUK__SM_MINUS, DUK__PI_MONTH, 0),
	DUK__PACK_RULE(DUK__PM_MONTH, DUK__SM_MINUS, DUK__PI_DAY, 0),
	DUK__PACK_RULE(DUK__PM_YEAR | DUK__PM_MONTH | DUK__PM_DAY, DUK__SM_T | DUK__SM_SPACE, DUK__PI_HOUR, 0),
	DUK__PACK_RULE(DUK__PM_HOUR, DUK__SM_COLON, DUK__PI_MINUTE, 0),
	DUK__PACK_RULE(DUK__PM_MINUTE, DUK__SM_COLON, DUK__PI_SECOND, 0),
	DUK__PACK_RULE(DUK__PM_SECOND, DUK__SM_PERIOD, DUK__PI_MILLISECOND, 0),
	DUK__PACK_RULE(DUK__PM_TZHOUR, DUK__SM_COLON, DUK__PI_TZMINUTE, 0),
	DUK__PACK_RULE(DUK__PM_YEAR | DUK__PM_MONTH | DUK__PM_DAY | DUK__PM_HOUR /*Note1*/ | DUK__PM_MINUTE | DUK__PM_SECOND | DUK__PM_MILLISECOND, DUK__SM_PLUS, DUK__PI_TZHOUR, 0),
	DUK__PACK_RULE(DUK__PM_YEAR | DUK__PM_MONTH | DUK__PM_DAY | DUK__PM_HOUR /*Note1*/ | DUK__PM_MINUTE | DUK__PM_SECOND | DUK__PM_MILLISECOND, DUK__SM_MINUS, DUK__PI_TZHOUR, DUK__CF_NEG),
	DUK__PACK_RULE(DUK__PM_YEAR | DUK__PM_MONTH | DUK__PM_DAY | DUK__PM_HOUR /*Note1*/ | DUK__PM_MINUTE | DUK__PM_SECOND | DUK__PM_MILLISECOND, DUK__SM_Z, 0, DUK__CF_ACCEPT_NUL),
	DUK__PACK_RULE(DUK__PM_YEAR | DUK__PM_MONTH | DUK__PM_DAY | DUK__PM_HOUR /*Note1*/ | DUK__PM_MINUTE | DUK__PM_SECOND | DUK__PM_MILLISECOND | DUK__PM_TZHOUR /*Note2*/ | DUK__PM_TZMINUTE, DUK__SM_NUL, 0, DUK__CF_ACCEPT)

	/* Note1: the specification doesn't require matching a time form with
	 *        just hours ("HH"), but we accept it here, e.g. "2012-01-02T12Z".
	 *
	 * Note2: the specification doesn't require matching a timezone offset
	 *        with just hours ("HH"), but accept it here, e.g. "2012-01-02T03:04:05+02"
	 */
};

static int duk__parse_string_iso8601_subset(duk_context *ctx, const char *str) {
	int parts[DUK__NUM_ISO8601_PARSER_PARTS];
	double dparts[DUK__NUM_PARTS];
	double d;
	const char *p;
	int part_idx = 0;
	int accum = 0;
	int neg_year = 0;
	int neg_tzoffset = 0;
	int ndigits = 0;
	char ch;
	int i;

	/* During parsing, month and day are one-based; set defaults here. */
	DUK_MEMZERO(parts, sizeof(parts));
	DUK_ASSERT(parts[DUK__IDX_YEAR] == 0);  /* don't care value, year is mandatory */
	parts[DUK__IDX_MONTH] = 1;
	parts[DUK__IDX_DAY] = 1;

	/* Special handling for year sign. */
	p = str;
	ch = p[0];
	if (ch == '+') {
		p++;
	} else if (ch == '-') {
		neg_year = 1;
		p++;
	}

	for (;;) {
		ch = *p++;
		DUK_DDD(DUK_DDDPRINT("parsing, part_idx=%d, char=%d ('%c')", part_idx, (int) ch,
		                     (ch >= 0x20 && ch <= 0x7e) ? ch : '?'));

		if (ch >= '0' && ch <= '9') {
			if (ndigits >= 9) {
				DUK_DDD(DUK_DDDPRINT("too many digits -> reject"));
				goto reject;
			}
			if (part_idx == DUK__PI_MILLISECOND /*msec*/ && ndigits >= 3) {
				/* ignore millisecond fractions after 3 */
			} else {
				accum = accum * 10 + ((int) ch) - ((int) '0') + 0x00;
				ndigits++;
			}
		} else {
			int match_val;
			int sep_idx;

			if (ndigits <= 0) {
				goto reject;
			}
			if (part_idx == DUK__PI_MILLISECOND) {
				/* complete the millisecond field */
				while (ndigits < 3) {
					accum *= 10;
					ndigits++;
				}
			}
			parts[part_idx] = accum;
			DUK_DDD(DUK_DDDPRINT("wrote part %d -> value %d", part_idx, accum));

			accum = 0;
			ndigits = 0;

			for (i = 0; i < (int) sizeof(duk__parse_iso8601_seps); i++) {
				if (duk__parse_iso8601_seps[i] == ch) {
					break;
				}
			}
			if (i == (int) sizeof(duk__parse_iso8601_seps)) {
				DUK_DDD(DUK_DDDPRINT("separator character doesn't match -> reject"));
				goto reject;
			}

			sep_idx = i;
			match_val = (1 << part_idx) + (1 << (sep_idx + 9));  /* match against rule part/sep bits */

			for (i = 0; i < (int) (sizeof(duk__parse_iso8601_control) / sizeof(int)); i++) {
				int rule = duk__parse_iso8601_control[i];
				int nextpart;
				int cflags;

				DUK_DDD(DUK_DDDPRINT("part_idx=%d, sep_idx=%d, match_val=0x%08x, considering rule=0x%08x",
				                     part_idx, sep_idx, match_val, rule));

				if ((rule & match_val) != match_val) {
					continue;
				}

				DUK__UNPACK_RULE(rule, nextpart, cflags);

				DUK_DDD(DUK_DDDPRINT("rule match -> part_idx=%d, sep_idx=%d, match_val=0x%08x, rule=0x%08x -> nextpart=%d, cflags=0x%02x",
				                     part_idx, sep_idx, match_val, rule, nextpart, cflags));

				if (cflags & DUK__CF_NEG) {
					neg_tzoffset = 1;
				}

				if (cflags & DUK__CF_ACCEPT) {
					goto accept;
				}

				if (cflags & DUK__CF_ACCEPT_NUL) {
					DUK_ASSERT(*(p-1) != (char) 0);
					if (*p == '\0') {
						goto accept;
					}
					goto reject;
				}

				part_idx = nextpart;
				break;
			}  /* rule match */

			if (i == (int) (sizeof(duk__parse_iso8601_control) / sizeof(int))) {
				DUK_DDD(DUK_DDDPRINT("no rule matches -> reject"));
				goto reject;
			}

			if (ch == (char) 0) {
				/* This shouldn't be necessary, but check just in case
				 * to avoid any chance of overruns.
				 */
				DUK_DDD(DUK_DDDPRINT("NUL after rule matching (should not happen) -> reject"));
				goto reject;
			}
		}  /* if-digit-else-ctrl */
	}  /* char loop */

	/* We should never exit the loop above, but if we do, reject
	 * by falling through.
	 */
	DUK_DDD(DUK_DDDPRINT("fell out of char loop without explicit accept/reject -> reject"));

 reject:
	DUK_DDD(DUK_DDDPRINT("reject"));
	return 0;

 accept:
	DUK_DDD(DUK_DDDPRINT("accept"));

	/* Apply timezone offset to get the main parts in UTC */
	if (neg_year) {
		parts[DUK__PI_YEAR] = -parts[DUK__PI_YEAR];
	}
	if (neg_tzoffset) {
		parts[DUK__PI_HOUR] += parts[DUK__PI_TZHOUR];
		parts[DUK__PI_MINUTE] += parts[DUK__PI_TZMINUTE];
	} else {
		parts[DUK__PI_HOUR] -= parts[DUK__PI_TZHOUR];
		parts[DUK__PI_MINUTE] -= parts[DUK__PI_TZMINUTE];
	}
	parts[DUK__PI_MONTH] -= 1;  /* zero-based month */
	parts[DUK__PI_DAY] -= 1;  /* zero-based day */

	/* Use double parts, they tolerate unnormalized time.
	 *
	 * Note: DUK__IDX_WEEKDAY is initialized with a bogus value (DUK__PI_TZHOUR)
	 * on purpose.  It won't be actually used by duk__get_timeval_from_dparts(),
	 * but will make the value initialized just in case, and avoid any
	 * potential for Valgrind issues.
	 */
	for (i = 0; i < DUK__NUM_PARTS; i++) {
		DUK_DDD(DUK_DDDPRINT("part[%d] = %d", i, parts[i]));
		dparts[i] = parts[i];
	}

	d = duk__get_timeval_from_dparts(dparts, 0 /*flags*/);
	duk_push_number(ctx, d);
	return 1;
}

/*
 *  Date/time parsing helper.
 *
 *  Parse a datetime string into a time value.  We must first try to parse
 *  the input according to the standard format in E5.1 Section 15.9.1.15.
 *  If that fails, we can try to parse using custom parsing, which can
 *  either be platform neutral (custom code) or platform specific (using
 *  existing platform API calls).
 *
 *  Note in particular that we must parse whatever toString(), toUTCString(),
 *  and toISOString() can produce; see E5.1 Section 15.9.4.2.
 */

/*
 *  FIXME: check standard behavior and also usual behavior in other
 *  implementations.  For instance, V8 parses '2012-01-01' as UTC and
 *  '2012/01/01' as local time.
 */

static int duk__parse_string(duk_context *ctx, const char *str) {
	/* XXX: there is a small risk here: because the ISO 8601 parser is
	 * very loose, it may end up parsing some datetime values which
	 * would be better parsed with a platform specific parser.
	 */

	DUK_ASSERT(str != NULL);
	DUK_DDD(DUK_DDDPRINT("parse datetime from string '%s'", str));

	if (duk__parse_string_iso8601_subset(ctx, str) > 0) {
		return 1;
	}

#if defined(DUK_USE_DATE_PRS_STRPTIME)
	if (duk__parse_string_strptime(ctx, str) > 0) {
		return 1;
	}
#elif defined(DUK_USE_DATE_PRS_GETDATE)
	if (duk__parse_string_getdate(ctx, str) > 0) {
		return 1;
	}
#else
	/* No platform-specific parsing, this is not an error. */
#endif

	duk_push_nan(ctx);
	return 1;
}

/*
 *  Calendar helpers
 *
 *  Some helpers are used for getters and can operate on normalized values
 *  which can be represented with 32-bit signed integers.  Other helpers are
 *  needed by setters and operate on un-normalized double values, must watch
 *  out for non-finite numbers etc.
 */

static unsigned char duk__days_in_month[12] = {
	(unsigned char) 31, (unsigned char) 28, (unsigned char) 31, (unsigned char) 30,
	(unsigned char) 31, (unsigned char) 30, (unsigned char) 31, (unsigned char) 31,
	(unsigned char) 30, (unsigned char) 31, (unsigned char) 30, (unsigned char) 31
};

static int duk__is_leap_year(int year) {
	if ((year % 4) != 0) {
		return 0;
	}
	if ((year % 100) != 0) {
		return 1;
	}
	if ((year % 400) != 0) {
		return 0;
	}
	return 1;
}

static double duk__timeclip(double x) {
	if (!DUK_ISFINITE(x)) {
		return DUK_DOUBLE_NAN;
	}

	if (x > 8.64e15 || x < -8.64e15) {
		return DUK_DOUBLE_NAN;
	}

	x = duk_js_tointeger_number(x);

	/* Here we'd have the option to normalize -0 to +0. */
	return x;
}

/* Integer division which floors also negative values correctly. */
static int duk__div_floor(int a, int b) {
	DUK_ASSERT(b > 0);
	if (a >= 0) {
		return a / b;
	} else {
		/* e.g. a = -4, b = 5  -->  -4 - 5 + 1 / 5  -->  -8 / 5  -->  -1
		 *      a = -5, b = 5  -->  -5 - 5 + 1 / 5  -->  -9 / 5  -->  -1
		 *      a = -6, b = 5  -->  -6 - 5 + 1 / 5  -->  -10 / 5  -->  -2
		 */
		return (a - b + 1) / b;
	}
}

/* Compute day number of the first day of a given year. */
static int duk__day_from_year(int year) {
	/* Note: in integer arithmetic, (x / 4) is same as floor(x / 4) for non-negative
	 * values, but is incorrect for negative ones.
	 */
	return 365 * (year - 1970) + duk__div_floor(year - 1969, 4) - duk__div_floor(year - 1901, 100) + duk__div_floor(year - 1601, 400);
}

/* Given a day number, determine year and day-within-year. */
static int duk__year_from_day(int day, int *out_day_within_year) {
	int year;
	int diff_days;

	/* estimate year upwards (towards positive infinity), then back down;
	 * two iterations should be enough
	 */

	if (day >= 0) {
		year = 1970 + day / 365;
	} else {
		year = 1970 + day / 366;
	}

	for (;;) {
		diff_days = duk__day_from_year(year) - day;
		DUK_DDD(DUK_DDDPRINT("year=%d day=%d, diff_days=%d", year, day, diff_days));
		if (diff_days <= 0) {
			*out_day_within_year = -diff_days;
			DUK_DDD(DUK_DDDPRINT("--> year=%d, day-within-year=%d",
			                     year, *out_day_within_year));
			DUK_ASSERT(*out_day_within_year >= 0);
			DUK_ASSERT(*out_day_within_year <= (duk__is_leap_year(year) ? 366 : 365));
			return year;
		}

		/* Note: this is very tricky; we must never 'overshoot' the
		 * correction downwards.
		 */
		year -= 1 + (diff_days - 1) / 366;  /* conservative */
	}
}

/* Given a (year, month, day-within-month) triple, compute day number.
 * The input triple is un-normalized and may contain non-finite values.
 */
static double duk__make_day(double year, double month, double day) {
	int day_num;
	int is_leap;
	int i;

	/* Assume that year, month, day are all coerced to whole numbers.
	 * They may also be NaN or infinity, in which case this function
	 * must return NaN or infinity to ensure time value becomes NaN.
	 */

	if (!DUK_ISFINITE(year) || !DUK_ISFINITE(month)) {
		return DUK_DOUBLE_NAN;
	}
	
	year += DUK_FLOOR(month / 12);

	month = DUK_FMOD(month, 12);
	if (month < 0) {
		/* handle negative values */
		month += 12;
	}

	/* The algorithm in E5.1 Section 15.9.1.12 normalizes month, but
	 * does not normalize the day-of-month (nor check whether or not
	 * it is finite) because it's not necessary for finding the day
	 * number which matches the (year,month) pair.
	 *
	 * We assume that duk__day_from_year() is exact here.
	 *
	 * Without an explicit infinity / NaN check in the beginning,
	 * day_num would be a bogus integer here.
	 */

	day_num = duk__day_from_year((int) year);
	is_leap = duk__is_leap_year((int) year);
	for (i = 0; i < (int) month; i++) {
		day_num += duk__days_in_month[i];
		if (i == 1 && is_leap) {
			day_num++;
		}
	}

	return (double) day_num + day;
}

/* Split time value into parts.  The time value is assumed to be an internal
 * one, i.e. finite, no fractions.  Possible local time adjustment has already
 * been applied when reading the time value.
 */
static void duk__timeval_to_parts(double d, int *parts, double *dparts, int flags) {
	double d1, d2;
	int t1, t2;
	int year, month, day;
	int dim;
	int i;
	int is_leap;

	DUK_ASSERT(DUK_ISFINITE(d));    /* caller checks */
	DUK_ASSERT(DUK_FLOOR(d) == d);  /* no fractions in internal time */

	/* these computations are guaranteed to be exact for the valid
	 * E5 time value range, assuming milliseconds without fractions.
	 */
	d1 = DUK_FMOD(d, (double) DUK__MS_DAY);
	if (d1 < 0.0) {
		/* deal with negative values */
		d1 += (double) DUK__MS_DAY;
	}
	d2 = DUK_FLOOR(d / (double) DUK__MS_DAY);
	DUK_ASSERT(d2 * ((double) DUK__MS_DAY) + d1 == d);

	/* now expected to fit into a 32-bit integer */
	t1 = (int) d1;
	t2 = (int) d2;
	DUK_ASSERT((double) t1 == d1);
	DUK_ASSERT((double) t2 == d2);

	/* t1 = milliseconds within day, t2 = day number */

	parts[DUK__IDX_MILLISECOND] = t1 % 1000; t1 /= 1000;
	parts[DUK__IDX_SECOND] = t1 % 60; t1 /= 60;
	parts[DUK__IDX_MINUTE] = t1 % 60; t1 /= 60;
	parts[DUK__IDX_HOUR] = t1;
	DUK_ASSERT(parts[DUK__IDX_MILLISECOND] >= 0 && parts[DUK__IDX_MILLISECOND] <= 999);
	DUK_ASSERT(parts[DUK__IDX_SECOND] >= 0 && parts[DUK__IDX_SECOND] <= 59);
	DUK_ASSERT(parts[DUK__IDX_MINUTE] >= 0 && parts[DUK__IDX_MINUTE] <= 59);
	DUK_ASSERT(parts[DUK__IDX_HOUR] >= 0 && parts[DUK__IDX_HOUR] <= 23);

	parts[DUK__IDX_WEEKDAY] = (t2 + 4) % 7;  /* E5.1 Section 15.9.1.6 */
	if (parts[DUK__IDX_WEEKDAY] < 0) {
		/* deal with negative values */
		parts[DUK__IDX_WEEKDAY] += 7;
	}

	year = duk__year_from_day(t2, &day);
	is_leap = duk__is_leap_year(year);
	for (month = 0; month < 12; month++) {
		dim = duk__days_in_month[month];
		if (month == 1 && is_leap) {
			dim++;
		}
		DUK_DDD(DUK_DDDPRINT("month=%d, dim=%d, day=%d", month, dim, day));
		if (day < dim) {
			break;
		}
		day -= dim;
	}
	DUK_DDD(DUK_DDDPRINT("final month=%d", month));
	DUK_ASSERT(month >= 0 && month <= 11);
	DUK_ASSERT(day >= 0 && day <= 31);

	parts[DUK__IDX_YEAR] = year;
	parts[DUK__IDX_MONTH] = month;
	parts[DUK__IDX_DAY] = day;

	if (flags & DUK__FLAG_ONEBASED) {
		parts[DUK__IDX_MONTH]++;  /* zero-based -> one-based */
		parts[DUK__IDX_DAY]++;    /* -""- */
	}

	if (dparts != NULL) {
		for (i = 0; i < DUK__NUM_PARTS; i++) {
			dparts[i] = (double) parts[i];
		}
	}
}

/* Compute time value from (double) parts. */
static double duk__get_timeval_from_dparts(double *dparts, int flags) {
	double tmp_time;
	double tmp_day;
	double d;
	int i;

	/* Expects 'this' at top of stack on entry. */

	/* Coerce all finite parts with ToInteger().  ToInteger() must not
	 * be called for NaN/Infinity because it will convert e.g. NaN to
	 * zero.  If ToInteger() has already been called, this has no side
	 * effects and is idempotent.
	 *
	 * Don't read dparts[DUK__IDX_WEEKDAY]; it will cause Valgrind issues
	 * if the value is uninitialized.
	 */
	for (i = 0; i <= DUK__IDX_MILLISECOND; i++) {
		d = dparts[i];
		if (DUK_ISFINITE(d)) {
			dparts[i] = duk_js_tointeger_number(d);
		}
	}

	/* Use explicit steps in computation to try to ensure that
	 * computation happens with intermediate results coerced to
	 * double values (instead of using something more accurate).
	 * E.g. E5.1 Section 15.9.1.11 requires use of IEEE 754
	 * rules (= Ecmascript '+' and '*' operators).
	 */
	
	/* MakeTime */
	tmp_time = 0;
	tmp_time += dparts[DUK__IDX_HOUR] * ((double) DUK__MS_HOUR);
	tmp_time += dparts[DUK__IDX_MINUTE] * ((double) DUK__MS_MINUTE);
	tmp_time += dparts[DUK__IDX_SECOND] * ((double) DUK__MS_SECOND);
	tmp_time += dparts[DUK__IDX_MILLISECOND];

	/* MakeDay */
	tmp_day = duk__make_day(dparts[DUK__IDX_YEAR], dparts[DUK__IDX_MONTH], dparts[DUK__IDX_DAY]);

	/* MakeDate */
	d = tmp_day * ((double) DUK__MS_DAY) + tmp_time;

	DUK_DDD(DUK_DDDPRINT("time=%lf day=%lf --> timeval=%lf", tmp_time, tmp_day, d));

	/* Optional UTC conversion followed by TimeClip().
	 * Note that this also handles Infinity -> NaN conversion.
	 */
	if (flags & DUK__FLAG_LOCALTIME) {
		/* FIXME: this is now incorrect.  'd' is local time here (as
		 * we're converting to UTC), but DUK__GET_LOCAL_TZOFFSET() should
		 * be called with UTC time.  This needs to be reworked to avoid
		 * the chicken-and-egg problem.
		 *
		 * See E5.1 Section 15.9.1.9:
		 * UTC(t) = t - LocalTZA - DaylightSavingTA(t - LocalTZA)
		 *
		 * For NaN/inf, DUK__GET_LOCAL_TZOFFSET() returns 0.
		 */

		d -= DUK__GET_LOCAL_TZOFFSET(d) * 1000;
	}
	d = duk__timeclip(d);

	return d;
}

/*
 *  API oriented helpers
 */

/* Push 'this' binding, check that it is a Date object; then push the
 * internal time value.  At the end, stack is: [ ... this timeval ].
 * Returns the time value.  Local time adjustment is done if requested.
 */
static double duk__push_this_get_timeval_tzoffset(duk_context *ctx, int flags, int *out_tzoffset) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;
	double d;
	int tzoffset = 0;

	duk_push_this(ctx);
	h = duk_get_hobject(ctx, -1);  /* FIXME: getter with class check, useful in built-ins */
	if (h == NULL || DUK_HOBJECT_GET_CLASS_NUMBER(h) != DUK_HOBJECT_CLASS_DATE) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "expected Date");
	}

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_VALUE);
	d = duk_to_number(ctx, -1);
	duk_pop(ctx);

	if (DUK_ISNAN(d)) {
		if (flags & DUK__FLAG_NAN_TO_ZERO) {
			d = 0.0;
		}
		if (flags & DUK__FLAG_NAN_TO_RANGE_ERROR) {
			DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, "Invalid Date");
		}
	}
	/* if no NaN handling flag, may still be NaN here, but not Inf */
	DUK_ASSERT(!DUK_ISINF(d));

	if (flags & DUK__FLAG_LOCALTIME) {
		/* Note: DST adjustment is determined using UTC time.
		 * If 'd' is NaN, tzoffset will be 0.
		 */
		tzoffset = DUK__GET_LOCAL_TZOFFSET(d);  /* seconds */
		d += tzoffset * 1000;
	}
	if (out_tzoffset) {
		*out_tzoffset = tzoffset;
	}

	/* [ ... this ] */
	return d;
}

static double duk__push_this_get_timeval(duk_context *ctx, int flags) {
	return duk__push_this_get_timeval_tzoffset(ctx, flags, NULL);
}

/* Set timeval to 'this' from dparts, push the new time value onto the
 * value stack and return 1 (caller can then tailcall us).  Expects
 * the value stack to contain 'this' on the stack top.
 */
static int duk__set_this_timeval_from_dparts(duk_context *ctx, double *dparts, int flags) {
	double d;

	/* [ ... this ] */

	d = duk__get_timeval_from_dparts(dparts, flags);
	duk_push_number(ctx, d);  /* -> [ ... this timeval_new ] */
	duk_dup_top(ctx);         /* -> [ ... this timeval_new timeval_new ] */
	duk_put_prop_stridx(ctx, -3, DUK_STRIDX_INT_VALUE);

	/* stack top: new time value */
	return 1;
}

/* 'out_buf' must be at least DUK_BI_DATE_ISO8601_BUFSIZE long. */
static void duk__format_parts_iso8601(int *parts, int tzoffset, int flags, duk_uint8_t *out_buf) {
	char yearstr[8];   /* "-123456\0" */
	char tzstr[8];     /* "+11:22\0" */
	char sep = (flags & DUK__FLAG_SEP_T) ? 'T' : ' ';

	DUK_ASSERT(parts[DUK__IDX_MONTH] >= 1 && parts[DUK__IDX_MONTH] <= 12);
	DUK_ASSERT(parts[DUK__IDX_DAY] >= 1 && parts[DUK__IDX_DAY] <= 31);
	DUK_ASSERT(parts[DUK__IDX_YEAR] >= -999999 && parts[DUK__IDX_YEAR] <= 999999);

	/* Note: %06d for positive value, %07d for negative value to include
	 * sign and 6 digits.
	 */
	DUK_SNPRINTF(yearstr,
	             sizeof(yearstr),
	             (parts[DUK__IDX_YEAR] >= 0 && parts[DUK__IDX_YEAR] <= 9999) ? "%04d" :
	                    ((parts[DUK__IDX_YEAR] >= 0) ? "+%06d" : "%07d"),
	             parts[DUK__IDX_YEAR]);
	yearstr[sizeof(yearstr) - 1] = (char) 0;

	if (flags & DUK__FLAG_LOCALTIME) {
		/* tzoffset seconds are dropped */
		if (tzoffset >= 0) {
			int tmp = tzoffset / 60;
			DUK_SNPRINTF(tzstr, sizeof(tzstr), "+%02d:%02d", tmp / 60, tmp % 60);
		} else {
			int tmp = -tzoffset / 60;
			DUK_SNPRINTF(tzstr, sizeof(tzstr), "-%02d:%02d", tmp / 60, tmp % 60);
		}
		tzstr[sizeof(tzstr) - 1] = (char) 0;
	} else {
		tzstr[0] = 'Z';
		tzstr[1] = (char) 0;
	}

	if ((flags & DUK__FLAG_TOSTRING_DATE) && (flags & DUK__FLAG_TOSTRING_TIME)) {
		DUK_SPRINTF((char *) out_buf, "%s-%02d-%02d%c%02d:%02d:%02d.%03d%s",
		            yearstr, parts[DUK__IDX_MONTH], parts[DUK__IDX_DAY], sep,
		            parts[DUK__IDX_HOUR], parts[DUK__IDX_MINUTE], parts[DUK__IDX_SECOND],
		            parts[DUK__IDX_MILLISECOND], tzstr);
	} else if (flags & DUK__FLAG_TOSTRING_DATE) {
		DUK_SPRINTF((char *) out_buf, "%s-%02d-%02d", yearstr, parts[DUK__IDX_MONTH], parts[DUK__IDX_DAY]);
	} else {
		DUK_ASSERT(flags & DUK__FLAG_TOSTRING_TIME);
		DUK_SPRINTF((char *) out_buf, "%02d:%02d:%02d.%03d%s", parts[DUK__IDX_HOUR], parts[DUK__IDX_MINUTE],
		            parts[DUK__IDX_SECOND], parts[DUK__IDX_MILLISECOND], tzstr);
	}
}

/* Helper for string conversion calls: check 'this' binding, get the
 * internal time value, and format date and/or time in a few formats.
 */
static int duk__to_string_helper(duk_context *ctx, int flags) {
	double d;
	int parts[DUK__NUM_PARTS];
	int tzoffset;  /* seconds */
	int rc;
	duk_uint8_t buf[DUK_BI_DATE_ISO8601_BUFSIZE];

	DUK_UNREF(rc);  /* unreferenced with some options */

	d = duk__push_this_get_timeval_tzoffset(ctx, flags, &tzoffset);
	if (DUK_ISNAN(d)) {
		duk_push_hstring_stridx(ctx, DUK_STRIDX_INVALID_DATE);
		return 1;
	}
	DUK_ASSERT(DUK_ISFINITE(d));

	/* formatters always get one-based month/day-of-month */
	duk__timeval_to_parts(d, parts, NULL, DUK__FLAG_ONEBASED);
	DUK_ASSERT(parts[DUK__IDX_MONTH] >= 1 && parts[DUK__IDX_MONTH] <= 12);
	DUK_ASSERT(parts[DUK__IDX_DAY] >= 1 && parts[DUK__IDX_DAY] <= 31);

	if (flags & DUK__FLAG_TOSTRING_LOCALE) {
		/* try locale specific formatter; if it refuses to format the
		 * string, fall back to an ISO 8601 formatted value in local
		 * time.
		 */
#ifdef DUK_USE_DATE_FMT_STRFTIME
		rc = duk__format_parts_strftime(ctx, parts, tzoffset, flags);
		if (rc == 1) {
			return rc;
		}
#else
		/* No locale specific formatter; this is OK, we fall back
		 * to ISO 8601.
		 */
#endif
	}

	/* Different calling convention than above used because the helper
	 * is shared.
	 */
	duk__format_parts_iso8601(parts, tzoffset, flags, buf);
	duk_push_string(ctx, (const char *) buf);
	return 1;
}

/* Helper for component getter calls: check 'this' binding, get the
 * internal time value, split it into parts (either as UTC time or
 * local time), push a specified component as a return value to the
 * value stack and return 1 (caller can then tailcall us).
 */
static int duk__get_part_helper(duk_context *ctx, int flags_and_idx) {
	double d;
	int parts[DUK__NUM_PARTS];
	int idx_part = flags_and_idx >> 12;  /* unpack args */

	DUK_ASSERT(idx_part >= 0 && idx_part < DUK__NUM_PARTS);

	d = duk__push_this_get_timeval(ctx, flags_and_idx);
	if (DUK_ISNAN(d)) {
		duk_push_nan(ctx);
		return 1;
	}
	DUK_ASSERT(DUK_ISFINITE(d));

	duk__timeval_to_parts(d, parts, NULL, flags_and_idx);

	/* Setter APIs detect special year numbers (0...99) and apply a +1900
	 * only in certain cases.  The legacy getYear() getter applies -1900
	 * unconditionally.
	 */
	duk_push_int(ctx, (flags_and_idx & DUK__FLAG_SUB1900) ? parts[idx_part] - 1900 : parts[idx_part]);
	return 1;
}

/* Helper for component setter calls: check 'this' binding, get the
 * internal time value, split it into parts (either as UTC time or
 * local time), modify one or more components as specified, recompute
 * the time value, set it as the internal value.  Finally, push the
 * new time value as a return value to the value stack and return 1
 * (caller can then tailcall us).
 */
static int duk__set_part_helper(duk_context *ctx, int flags_and_maxnargs) {
	double d;
	int parts[DUK__NUM_PARTS];
	double dparts[DUK__NUM_PARTS];
	int nargs;
	int maxnargs = flags_and_maxnargs >> 12;  /* unpack args */
	int idx_first, idx;
	int i;

	nargs = duk_get_top(ctx);
	d = duk__push_this_get_timeval(ctx, flags_and_maxnargs);
	DUK_ASSERT(DUK_ISFINITE(d) || DUK_ISNAN(d));

	if (DUK_ISFINITE(d)) {
		duk__timeval_to_parts(d, parts, dparts, flags_and_maxnargs);
	} else {
		/* NaN timevalue: we need to coerce the arguments, but
		 * the resulting internal timestamp needs to remain NaN.
		 * This works but is not pretty: parts and dparts will
		 * be partially uninitialized, but we only write to it.
		 */
	}

	/*
	 *  Determining which datetime components to overwrite based on
	 *  stack arguments is a bit complicated, but important to factor
	 *  out from setters themselves for compactness.
	 *
	 *  If DUK__FLAG_TIMESETTER, maxnargs indicates setter type:
	 *
	 *   1 -> millisecond
	 *   2 -> second, [millisecond]
	 *   3 -> minute, [second], [millisecond]
	 *   4 -> hour, [minute], [second], [millisecond]
	 *
	 *  Else:
	 *
	 *   1 -> date
	 *   2 -> month, [date]
	 *   3 -> year, [month], [date]
	 *
	 *  By comparing nargs and maxnargs (and flags) we know which
	 *  components to override.  We rely on part index ordering.
	 */

	if (flags_and_maxnargs & DUK__FLAG_TIMESETTER) {
		DUK_ASSERT(maxnargs >= 1 && maxnargs <= 4);
		idx_first = DUK__IDX_MILLISECOND - (maxnargs - 1);
	} else {
		DUK_ASSERT(maxnargs >= 1 && maxnargs <= 3);
		idx_first = DUK__IDX_DAY - (maxnargs - 1);
	}
	DUK_ASSERT(idx_first >= 0 && idx_first < DUK__NUM_PARTS);

	for (i = 0; i < maxnargs; i++) {
		if (i >= nargs) {
			/* no argument given -> leave components untouched */
			break;
		}
		idx = idx_first + i;
		DUK_ASSERT(idx >= 0 && idx < DUK__NUM_PARTS);

		if (idx == DUK__IDX_YEAR && (flags_and_maxnargs & DUK__FLAG_YEAR_FIXUP)) {
			duk__twodigit_year_fixup(ctx, i);
		}

		dparts[idx] = duk_to_number(ctx, i);

		if (idx == DUK__IDX_DAY) {
			/* Day-of-month is one-based in the API, but zero-based
			 * internally, so fix here.  Note that month is zero-based
			 * both in the API and internally.
			 */
			dparts[idx] -= 1.0;
		}
	}

	/* Leaves new timevalue on stack top and returns 1, which is correct
	 * for part setters.
	 */
	if (DUK_ISFINITE(d)) {
		return duk__set_this_timeval_from_dparts(ctx, dparts, flags_and_maxnargs);
	} else {
		/* Internal timevalue is already NaN, so don't touch it. */
		duk_push_nan(ctx);
		return 1;
	}
}

/* Apply ToNumber() to specified index; if ToInteger(val) in [0,99], add
 * 1900 and replace value at idx_val.
 */
static void duk__twodigit_year_fixup(duk_context *ctx, int idx_val) {
	double d;

	/* E5 Sections 15.9.3.1, B.2.4, B.2.5 */
	duk_to_number(ctx, idx_val);
	if (duk_is_nan(ctx, idx_val)) {
		return;
	}
	duk_dup(ctx, idx_val);
	duk_to_int(ctx, -1);
	d = duk_get_number(ctx, -1);  /* get as double to handle huge numbers correctly */
	if (d >= 0.0 && d <= 99.0) {
		d += 1900.0;
		duk_push_number(ctx, d);
		duk_replace(ctx, idx_val);
	}
	duk_pop(ctx);
}

/* Set datetime parts from stack arguments, defaulting any missing values.
 * Day-of-week is not set; it is not required when setting the time value.
 */
static void duk__set_parts_from_args(duk_context *ctx, double *dparts, int nargs) {
	double d;
	int i;
	int idx;

	/* Causes a ToNumber() coercion, but doesn't break coercion order since
	 * year is coerced first anyway.
	 */
	duk__twodigit_year_fixup(ctx, 0);

	/* There are at most 7 args, but we use 8 here so that also
	 * DUK__IDX_WEEKDAY gets initialized (to zero) to avoid the potential
	 * for any Valgrind gripes later.
	 */
	for (i = 0; i < 8; i++) {
		/* Note: rely on index ordering */
		idx = DUK__IDX_YEAR + i;
		if (i < nargs) {
			d = duk_to_number(ctx, i);
			if (idx == DUK__IDX_DAY) {
				/* Convert day from one-based to zero-based (internal).  This may
				 * cause the day part to be negative, which is OK.
				 */
				d -= 1.0;
			}
		} else {
			/* All components default to 0 except day-of-month which defaults
			 * to 1.  However, because our internal day-of-month is zero-based,
			 * it also defaults to zero here.
			 */
			d = 0.0;
		}
		dparts[idx] = d;
	}

	DUK_DDD(DUK_DDDPRINT("parts from args -> %lf %lf %lf %lf %lf %lf %lf %lf",
	                     dparts[0], dparts[1], dparts[2], dparts[3],
	                     dparts[4], dparts[5], dparts[6], dparts[7]));
}

/*
 *  Helper to format a time value into caller buffer, used by logging.
 *  'out_buf' must be at least DUK_BI_DATE_ISO8601_BUFSIZE long.
 */

void duk_bi_date_format_timeval(duk_double_t timeval, duk_uint8_t *out_buf) {
	int parts[DUK__NUM_PARTS];

	duk__timeval_to_parts(timeval,
	                      parts,
	                      NULL,
	                      DUK__FLAG_ONEBASED);

	duk__format_parts_iso8601(parts,
	                          0 /*tzoffset*/,
	                          DUK__FLAG_TOSTRING_DATE |
	                          DUK__FLAG_TOSTRING_TIME |
	                          DUK__FLAG_SEP_T /*flags*/,
	                          out_buf);
}

/*
 *  Constructor calls
 */

int duk_bi_date_constructor(duk_context *ctx) {
	int nargs = duk_get_top(ctx);
	int is_cons = duk_is_constructor_call(ctx);
	double dparts[DUK__NUM_PARTS];
	double d;

	DUK_DDD(DUK_DDDPRINT("Date constructor, nargs=%d, is_cons=%d", nargs, is_cons));

	duk_push_object_helper(ctx,
	                       DUK_HOBJECT_FLAG_EXTENSIBLE |
	                       DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_DATE),
	                       DUK_BIDX_DATE_PROTOTYPE);

	/* Unlike most built-ins, the internal [[PrimitiveValue]] of a Date
	 * is mutable.
	 */

	if (nargs == 0 || !is_cons) {
		d = duk__timeclip(DUK__GET_NOW_TIMEVAL(ctx));
		duk_push_number(ctx, d);
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_W);
		if (!is_cons) {
			/* called as a normal function: return new Date().toString() */
			duk_to_string(ctx, -1);
		}
		return 1;
	} else if (nargs == 1) {
		duk_to_primitive(ctx, 0, DUK_HINT_NONE);
		if (duk_is_string(ctx, 0)) {
			duk__parse_string(ctx, duk_to_string(ctx, 0));
			duk_replace(ctx, 0);  /* may be NaN */
		}
		d = duk__timeclip(duk_to_number(ctx, 0));
		duk_push_number(ctx, d);
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_W);
		return 1;
	}

	duk__set_parts_from_args(ctx, dparts, nargs);

	/* Parts are in local time, convert when setting. */

	duk__set_this_timeval_from_dparts(ctx, dparts, DUK__FLAG_LOCALTIME /*flags*/);  /* -> [ ... this timeval ] */
	duk_pop(ctx);  /* -> [ ... this ] */
	return 1;
}

int duk_bi_date_constructor_parse(duk_context *ctx) {
	return duk__parse_string(ctx, duk_to_string(ctx, 0));
}

int duk_bi_date_constructor_utc(duk_context *ctx) {
	int nargs = duk_get_top(ctx);
	double dparts[DUK__NUM_PARTS];
	double d;

	/* Behavior for nargs < 2 is implementation dependent: currently we'll
	 * set a NaN time value (matching V8 behavior) in this case.
	 */

	if (nargs < 2) {
		duk_push_nan(ctx);
	} else {
		duk__set_parts_from_args(ctx, dparts, nargs);
		d = duk__get_timeval_from_dparts(dparts, 0 /*flags*/);
		duk_push_number(ctx, d);
	}
	return 1;
}

int duk_bi_date_constructor_now(duk_context *ctx) {
	double d;

	d = DUK__GET_NOW_TIMEVAL(ctx);
	DUK_ASSERT(duk__timeclip(d) == d);  /* TimeClip() should never be necessary */
	duk_push_number(ctx, d);
	return 1;
}

/*
 *  String/JSON conversions
 *
 *  Human readable conversions are now basically ISO 8601 with a space
 *  (instead of 'T') as the date/time separator.  This is a good baseline
 *  and is platform independent.
 *
 *  A shared native helper to provide many conversions.  Magic value contains
 *  a set of flags.  The helper provides:
 *
 *    toString()
 *    toDateString()
 *    toTimeString()
 *    toLocaleString()
 *    toLocaleDateString()
 *    toLocaleTimeString()
 *    toUTCString()
 *    toISOString()
 *
 *  Notes:
 *
 *    - Date.prototype.toGMTString() and Date.prototype.toUTCString() are
 *      required to be the same Ecmascript function object (!), so it is
 *      omitted from here.
 *
 *    - Date.prototype.toUTCString(): E5.1 specification does not require a
 *      specific format, but result should be human readable.  The
 *      specification suggests using ISO 8601 format with a space (instead
 *      of 'T') separator if a more human readable format is not available.
 *
 *    - Date.prototype.toISOString(): unlike other conversion functions,
 *      toISOString() requires a RangeError for invalid date values.
 */

int duk_bi_date_prototype_tostring_shared(duk_context *ctx) {
	int flags = duk_get_magic(ctx);
	return duk__to_string_helper(ctx, flags);
}

int duk_bi_date_prototype_value_of(duk_context *ctx) {
	/* This native function is also used for Date.prototype.getTime()
	 * as their behavior is identical.
	 */

	double d = duk__push_this_get_timeval(ctx, 0 /*flags*/);  /* -> [ this ] */
	DUK_ASSERT(DUK_ISFINITE(d) || DUK_ISNAN(d));
	duk_push_number(ctx, d);
	return 1;
}

int duk_bi_date_prototype_to_json(duk_context *ctx) {
	/* Note: toJSON() is a generic function which works even if 'this'
	 * is not a Date.  The sole argument is ignored.
	 */

	duk_push_this(ctx);
	duk_to_object(ctx, -1);

	duk_dup_top(ctx);
	duk_to_primitive(ctx, -1, DUK_HINT_NUMBER);
	if (duk_is_number(ctx, -1)) {
		double d = duk_get_number(ctx, -1);
		if (!DUK_ISFINITE(d)) {
			duk_push_null(ctx);
			return 1;
		}
	}
	duk_pop(ctx);

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_TO_ISO_STRING);
	duk_dup(ctx, -2);  /* -> [ O toIsoString O ] */
	duk_call_method(ctx, 0);
	return 1;
}

/*
 *  Getters.
 *
 *  Implementing getters is quite easy.  The internal time value is either
 *  NaN, or represents milliseconds (without fractions) from Jan 1, 1970.
 *  The internal time value can be converted to integer parts, and each
 *  part will be normalized and will fit into a 32-bit signed integer.
 *
 *  A shared native helper to provide all getters.  Magic value contains
 *  a set of flags and also packs the date component index argument.  The
 *  helper provides:
 *
 *    getFullYear()
 *    getUTCFullYear()
 *    getMonth()
 *    getUTCMonth()
 *    getDate()
 *    getUTCDate()
 *    getDay()
 *    getUTCDay()
 *    getHours()
 *    getUTCHours()
 *    getMinutes()
 *    getUTCMinutes()
 *    getSeconds()
 *    getUTCSeconds()
 *    getMilliseconds()
 *    getUTCMilliseconds()
 *    getYear()
 *
 *  Notes:
 *
 *    - Date.prototype.getDate(): 'date' means day-of-month, and is
 *      zero-based in internal calculations but public API expects it to
 *      be one-based.
 *
 *    - Date.prototype.getTime() and Date.prototype.valueOf() have identical
 *      behavior.  They have separate function objects, but share the same C
 *      function (duk_bi_date_prototype_value_of).
 */

int duk_bi_date_prototype_get_shared(duk_context *ctx) {
	int flags_and_idx = duk_get_magic(ctx);
	return duk__get_part_helper(ctx, flags_and_idx);
}

int duk_bi_date_prototype_get_timezone_offset(duk_context *ctx) {
	/*
	 *  Return (t - LocalTime(t)) in minutes:
	 *
	 *    t - LocalTime(t) = t - (t + LocalTZA + DaylightSavingTA(t))
	 *                     = -(LocalTZA + DaylightSavingTA(t))
	 *
	 *  where DaylightSavingTA() is checked for time 't'.
	 *
	 *  Note that the sign of the result is opposite to common usage,
	 *  e.g. for EE(S)T which normally is +2h or +3h from UTC, this
	 *  function returns -120 or -180.
	 *
	 */

	double d;
	int tzoffset;

	/* Note: DST adjustment is determined using UTC time. */
	d = duk__push_this_get_timeval(ctx, 0 /*flags*/);
	DUK_ASSERT(DUK_ISFINITE(d) || DUK_ISNAN(d));
	if (DUK_ISNAN(d)) {
		duk_push_nan(ctx);
	} else {
		DUK_ASSERT(DUK_ISFINITE(d));
		tzoffset = DUK__GET_LOCAL_TZOFFSET(d);
		duk_push_int(ctx, -tzoffset / 60);
	}
	return 1;
}

/*
 *  Setters.
 *
 *  Setters are a bit more complicated than getters.  Component setters
 *  break down the current time value into its (normalized) component
 *  parts, replace one or more components with -unnormalized- new values,
 *  and the components are then converted back into a time value.  As an
 *  example of using unnormalized values:
 *
 *    var d = new Date(1234567890);
 *
 *  is equivalent to:
 *
 *    var d = new Date(0);
 *    d.setUTCMilliseconds(1234567890);
 *
 *  A shared native helper to provide almost all setters.  Magic value
 *  contains a set of flags and also packs the "maxnargs" argument.  The
 *  helper provides:
 *
 *    setMilliseconds()
 *    setUTCMilliseconds()
 *    setSeconds()
 *    setUTCSeconds()
 *    setMinutes()
 *    setUTCMinutes()
 *    setHours()
 *    setUTCHours()
 *    setDate()
 *    setUTCDate()
 *    setMonth()
 *    setUTCMonth()
 *    setFullYear()
 *    setUTCFullYear()
 *    setYear()
 *
 *  Notes:
 *
 *    - Date.prototype.setYear() (Section B addition): special year check
 *      is omitted.  NaN / Infinity will just flow through and ultimately
 *      result in a NaN internal time value.
 *
 *    - Date.prototype.setYear() does not have optional arguments for
 *      setting month and day-in-month (like setFullYear()), but we indicate
 *      'maxnargs' to be 3 to get the year written to the correct component
 *      index in duk__set_part_helper().  The function has nargs == 1, so only
 *      the year will be set regardless of actual argument count.
 */

int duk_bi_date_prototype_set_shared(duk_context *ctx) {
	int flags_and_maxnargs = duk_get_magic(ctx);
	return duk__set_part_helper(ctx, flags_and_maxnargs);
}

int duk_bi_date_prototype_set_time(duk_context *ctx) {
	double d;

	(void) duk__push_this_get_timeval(ctx, 0 /*flags*/); /* -> [ timeval this ] */
	d = duk__timeclip(duk_to_number(ctx, 0));
	duk_push_number(ctx, d);
	duk_dup_top(ctx);
	duk_put_prop_stridx(ctx, -3, DUK_STRIDX_INT_VALUE); /* -> [ timeval this timeval ] */

	return 1;
}

