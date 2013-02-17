/*
 *  Date built-ins
 *
 *  An Ecmascript time value is essentially UNIX/Posix (UTC) time in
 *  milliseconds.  A time value has a simple arithmetic relationship
 *  with UTC datetime values; leap years are taken into account, but
 *  leap seconds are not.  As a side effect when a leap second is
 *  inserted, the Ecmascript time value jumps backwards by one second.
 *
 *  http://www.ecma-international.org/ecma-262/5.1/#sec-15.9.1.1
 *  http://en.wikipedia.org/wiki/Unix_time
 *
 *  Implementation notes:
 *
 *    - Almost all API calls require a Date instance as the 'this'
 *      binding (a TypeError is thrown otherwise).  Exceptions are
 *      noted in the specification (e.g. toJSON()).
 *
 *    - The internal time value always exists for a Date instance,
 *      and is always a number.  The number value is either NaN, or
 *      a finite number in the valid E5 range.  The millisecond count
 *      has no fractions.  The internal component representation uses
 *      zero-based day and month, while the Ecmascript uses one-based
 *      day and zero-based month.
 *
 *    - When the internal time value is broken into components, each
 *      component will be normalized, and will fit into a 32-bit signed
 *      integer.  When using setter calls, one or more components are
 *      replaced with unnormalized values (which will not necessarily
 *      fit into a 32-bit signed integer) before converting back to an
 *      internal time value.  The setter values may be huge (even out
 *      of 64-bit range) without resulting in an invalid result date,
 *      if multiple cancelling values are given (e.g. 1e100 seconds and
 *      -1e103 milliseconds).
 *
 *    - The current implementation uses an ISO 8601 -based format for
 *      representing Dates in a "human readable" form.  It might be
 *      preferable to use a platform-dependent format.  There is also
 *      no locale support now.  E5/E5.1 does not require a specific
 *      format for human readable time stamps, so this is compliant.
 *
 *    - Most internal algorithms are trivial.  Algorithms converting
 *      between a day number or time and year are a bit difficult
 *      because of leap years.  We resort to iteration in some cases.
 *
 *    - Setters and getters are optimized for size, to use a single
 *      helper with a set of flags and arguments to keep each getter
 *      and setter itself very small.  This makes them a bit cryptic;
 *      see e.g. handling of setters with optional parameters.
 *
 *    - Local time handling is difficult, e.g. DST depends on locale
 *      information.  Locale information for timestamps very far in
 *      the past or the future may not be available (e.g. Y2038 in
 *      some UNIX implementations).
 *
 *    - The LocalTZA() and DaylightSavingTA(t) specification functions
 *      are not needed as such.  They are only used, together, when
 *      converting between local time and UTC time.
 */

#include "duk_internal.h"

#include <sys/time.h>
#include <time.h>

static void timeval_to_parts(duk_context *ctx, double d, int *parts, double *dparts, int flags);
static double get_timeval_from_dparts(duk_context *ctx, double *dparts, int flags);

/* Millisecond count constants. */
#define  MS_SECOND          1000
#define  MS_MINUTE          (60 * 1000)
#define  MS_HOUR            (60 * 60 * 1000)
#define  MS_DAY             (24 * 60 * 60 * 1000)

/* Part indices for internal breakdowns.  Part order from IDX_YEAR to
 * IDX_MILLISECOND matches argument ordering of Ecmascript API calls
 * (like Date constructor call).  A few functions in this file depend
 * on the specific ordering, so change with care.
 */
#define  IDX_YEAR           0  /* year */
#define  IDX_MONTH          1  /* month: 0 to 11 */
#define  IDX_DAY            2  /* day within month: 0 to 30 */
#define  IDX_HOUR           3
#define  IDX_MINUTE         4
#define  IDX_SECOND         5
#define  IDX_MILLISECOND    6
#define  IDX_WEEKDAY        7  /* weekday: 0 to 6, 0=sunday, 1=monday, etc */
#define  NUM_PARTS          8

/* Internal API call flags, used for various functions in this file.
 * Certain flags are used by only certain functions, but since the flags
 * don't overlap, a single flags value can be passed around to multiple
 * functions.
 *
 * The unused top bits of the flags field are also used to pass values
 * to helpers (get_part_helper() and set_part_helper()).
 */
#define  FLAG_NAN_TO_ZERO          (1 << 0)  /* timeval breakdown: internal time value NaN -> zero */
#define  FLAG_NAN_TO_RANGE_ERROR   (1 << 1)  /* timeval breakdown: internal time value NaN -> RangeError (toISOString) */
#define  FLAG_ONEBASED             (1 << 2)  /* timeval breakdown: convert month and day-of-month parts to one-based (default is zero-based) */
#define  FLAG_LOCALTIME            (1 << 3)  /* convert time value to local time */
#define  FLAG_SUB1900              (1 << 4)  /* getter: subtract 1900 from year when getting year part */
#define  FLAG_TOSTRING_DATE        (1 << 5)  /* include date part in string conversion result */
#define  FLAG_TOSTRING_TIME        (1 << 6)  /* include time part in string conversion result */
#define  FLAG_TIMESETTER           (1 << 7)  /* setter: call is a time setter (affects hour, min, sec, ms); otherwise date setter (affects year, month, day-in-month) */

/*
 *  Platform specific helpers
 */

/* Current Ecmascript (= UNIX) time */
static double get_now_timeval(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	struct timeval tv;
	double d;

	if (gettimeofday(&tv, NULL) != 0) {
		DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "gettimeofday failed");
	}

	d = ((double) tv.tv_sec) * 1000.0 +
	    ((double) (tv.tv_usec / 1000));
	DUK_ASSERT(floor(d) == d);  /* no fractions */

	return d;
}

/* Get local time offset (in seconds) for a certain instant 'd'. */
static int get_local_tzoffset(duk_context *ctx, double d) {
	duk_hthread *thr = (duk_hthread *) ctx;
	time_t t, t1, t2;
	int parts[NUM_PARTS];
	double dparts[NUM_PARTS];
	struct tm tms[2];

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
	 *    in January 1970 start to fail (verified).
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

	timeval_to_parts(ctx, d, parts, dparts, 0 /*flags*/);

	if (parts[IDX_YEAR] < 1971) {
		dparts[IDX_YEAR] = 1971.0;
	} else if (parts[IDX_YEAR] > 2037) {
		dparts[IDX_YEAR] = 2037.0;
	}

	d = get_timeval_from_dparts(ctx, dparts, 0 /*flags*/);
	DUK_ASSERT(d >= 0 && d < 2147483648.0 * 1000.0);  /* unsigned 31-bit range */
	t = (size_t) (d / 1000.0);
	DUK_DPRINT("timeval: %lf -> time_t %d", d, (int) t);

	t1 = t;

	memset((void *) tms, 0, sizeof(struct tm) * 2);

	(void) gmtime_r(&t, &tms[0]);
	memcpy((void *) &tms[1], &tms[0], sizeof(struct tm));
	DUK_DPRINT("before mktime: tm={sec:%d,min:%d,hour:%d,mday:%d,mon:%d,year:%d,"
	           "wday:%d,yday:%d,isdst:%d}",
	           (int) tms[0].tm_sec, (int) tms[0].tm_min, (int) tms[0].tm_hour,
	           (int) tms[0].tm_mday, (int) tms[0].tm_mon, (int) tms[0].tm_year,
	           (int) tms[0].tm_wday, (int) tms[0].tm_yday, (int) tms[0].tm_isdst);

	(void) mktime(&tms[0]);
	tms[1].tm_isdst = tms[0].tm_isdst;
	t2 = mktime(&tms[1]);
	DUK_ASSERT(t2 >= 0);
	if (t2 < 0) {
		goto error;
	}

	DUK_DPRINT("after mktime: tm={sec:%d,min:%d,hour:%d,mday:%d,mon:%d,year:%d,"
	           "wday:%d,yday:%d,isdst:%d}",
	           (int) tms[1].tm_sec, (int) tms[1].tm_min, (int) tms[1].tm_hour,
	           (int) tms[1].tm_mday, (int) tms[1].tm_mon, (int) tms[1].tm_year,
	           (int) tms[1].tm_wday, (int) tms[1].tm_yday, (int) tms[1].tm_isdst);
	DUK_DPRINT("t2=%d", (int) t2);

	/* Positive if local time ahead of UTC. */
	return t1 - t2;

 error:
	/* FIXME: this can be replaced with an assert if there is a guarantee
	 * that mktime() can never fail in the valid range.  Is this the case,
	 * when dealing with locales?
	 */
	DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "mktime failed");
	return 0;
}

/*
 *  Helpers
 *
 *  Some helpers are used for getters and can operate on normalized values
 *  which can be represented with 32-bit signed integers.  Other helpers are
 *  needed by setters and operate on un-normalized double values, must watch
 *  out for non-finite numbers etc.
 */

static unsigned char days_in_month[12] = {
	(unsigned char) 31, (unsigned char) 28, (unsigned char) 31, (unsigned char) 30,
	(unsigned char) 31, (unsigned char) 30, (unsigned char) 31, (unsigned char) 31,
	(unsigned char) 30, (unsigned char) 31, (unsigned char) 30, (unsigned char) 31
};

static int is_leap_year(int year) {
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

static double timeclip(double x) {
	if (!isfinite(x)) {
		return NAN;
	}

	if (x > 8.64e15 || x < -8.64e15) {
		return NAN;
	}

	x = duk_js_tointeger_number(x);

	/* Here we'd have the option to normalize -0 to +0. */
	return x;
}

/* Integer division which floors also negative values correctly. */
static int div_floor(int a, int b) {
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
static int day_from_year(int year) {
	/* Note: in integer arithmetic, (x / 4) is same as floor(x / 4) for non-negative
	 * values, but is incorrect for negative ones.
	 */
	return 365 * (year - 1970) + div_floor(year - 1969, 4) - div_floor(year - 1901, 100) + div_floor(year - 1601, 400);
}

/* Given a day number, determine year and day-within-year. */
static int year_from_day(int day, int *out_day_within_year) {
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
		diff_days = day_from_year(year) - day;
		DUK_DPRINT("year=%d day=%d, diff_days=%d", year, day, diff_days);
		if (diff_days <= 0) {
			*out_day_within_year = -diff_days;
			DUK_DPRINT("--> year=%d, day-within-year=%d",
			           year, *out_day_within_year);
			DUK_ASSERT(*out_day_within_year >= 0);
			DUK_ASSERT(*out_day_within_year <= (is_leap_year(year) ? 366 : 365));
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
static double make_day(double year, double month, double day) {
	int day_num;
	int is_leap;
	int i;

	/* Assume that year, month, day are all coerced to whole numbers.
	 * They may also be NaN or infinity, in which case this function
	 * must return NaN or infinity to ensure time value becomes NaN.
	 */

	if (!isfinite(year) || !isfinite(month)) {
		return NAN;
	}
	
	year += floor(month / 12);

	month = fmod(month, 12);
	if (month < 0) {
		/* handle negative values */
		month += 12;
	}

	/* The algorithm in E5.1 Section 15.9.1.12 normalizes month, but
	 * does not normalize the day-of-month (nor check whether or not
	 * it is finite) because it's not necessary for finding the day
	 * number which matches the (year,month) pair.
	 *
	 * We assume that day_from_year() is exact here.
	 *
	 * Without an explicit infinity / NaN check in the beginning,
	 * day_num would be a bogus integer here.
	 */

	day_num = day_from_year((int) year);
	is_leap = is_leap_year((int) year);
	for (i = 0; i < (int) month; i++) {
		day_num += days_in_month[i];
		if (i == 1 && is_leap) {
			day_num++;
		}
	}

	return (double) day_num + day;
}

/* Push 'this' binding, check that it is a Date object; then push the
 * internal time value.  At the end, stack is: [ ... this timeval ].
 * Returns the time value.  Local time adjustment is done if requested.
 */
static double push_this_and_get_timeval(duk_context *ctx, int flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;
	double d;

	duk_push_this(ctx);
	h = duk_get_hobject(ctx, -1);  /* FIXME: getter with class check, useful in built-ins */
	if (h == NULL || DUK_HOBJECT_GET_CLASS_NUMBER(h) != DUK_HOBJECT_CLASS_DATE) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "expected Date");
	}

	duk_get_prop_stridx(ctx, -1, DUK_HEAP_STRIDX_INT_VALUE);
	d = duk_to_number(ctx, -1);
	duk_pop(ctx);

	if (isnan(d)) {
		if (flags & FLAG_NAN_TO_ZERO) {
			d = 0.0;
		}
		if (flags & FLAG_NAN_TO_RANGE_ERROR) {
			DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, "Invalid Date");
		}
	}
	if (flags & FLAG_LOCALTIME) {
		/* Note: DST adjustment is determined using UTC time. */
		d += get_local_tzoffset(ctx, d) * 1000;
	}

	/* [ ... this ] */
	return d;
}

/* Split time value into parts.  The time value is assumed to be an internal
 * one, i.e. finite, no fractions.  Possible local time adjustment has already
 * been applied when reading the time value.
 */
static void timeval_to_parts(duk_context *ctx, double d, int *parts, double *dparts, int flags) {
	double d1, d2;
	int t1, t2;
	int year, month, day;
	int dim;
	int i;
	int is_leap;

	DUK_ASSERT(isfinite(d));    /* caller checks */
	DUK_ASSERT(floor(d) == d);  /* no fractions in internal time */

	/* these computations are guaranteed to be exact for the valid
	 * E5 time value range, assuming milliseconds without fractions.
	 */
	d1 = fmod(d, (double) MS_DAY);
	if (d1 < 0.0) {
		/* deal with negative values */
		d1 += (double) MS_DAY;
	}
	d2 = floor(d / (double) MS_DAY);
	DUK_ASSERT(d2 * ((double) MS_DAY) + d1 == d);

	/* now expected to fit into a 32-bit integer */
	t1 = (int) d1;
	t2 = (int) d2;
	DUK_ASSERT((double) t1 == d1);
	DUK_ASSERT((double) t2 == d2);

	/* t1 = milliseconds within day, t2 = day number */

	parts[IDX_MILLISECOND] = t1 % 1000; t1 /= 1000;
	parts[IDX_SECOND] = t1 % 60; t1 /= 60;
	parts[IDX_MINUTE] = t1 % 60; t1 /= 60;
	parts[IDX_HOUR] = t1;
	DUK_ASSERT(parts[IDX_MILLISECOND] >= 0 && parts[IDX_MILLISECOND] <= 999);
	DUK_ASSERT(parts[IDX_SECOND] >= 0 && parts[IDX_SECOND] <= 59);
	DUK_ASSERT(parts[IDX_MINUTE] >= 0 && parts[IDX_MINUTE] <= 59);
	DUK_ASSERT(parts[IDX_HOUR] >= 0 && parts[IDX_HOUR] <= 23);

	parts[IDX_WEEKDAY] = (t2 + 4) % 7;  /* E5.1 Section 15.9.1.6 */
	if (parts[IDX_WEEKDAY] < 0) {
		/* deal with negative values */
		parts[IDX_WEEKDAY] += 7;
	}

	year = year_from_day(t2, &day);
	is_leap = is_leap_year(year);
	for (month = 0; month < 12; month++) {
		dim = days_in_month[month];
		if (month == 1 && is_leap) {
			dim++;
		}
		DUK_DPRINT("month=%d, dim=%d, day=%d", month, dim, day);
		if (day < dim) {
			break;
		}
		day -= dim;
	}
	DUK_DPRINT("final month=%d", month);
	DUK_ASSERT(month >= 0 && month <= 11);
	DUK_ASSERT(day >= 0 && day <= 31);

	parts[IDX_YEAR] = year;
	parts[IDX_MONTH] = month;
	parts[IDX_DAY] = day;

	if (flags & FLAG_ONEBASED) {
		parts[IDX_MONTH]++;  /* zero-based -> one-based */
		parts[IDX_DAY]++;    /* -""- */
	}

	if (dparts != NULL) {
		for (i = 0; i < NUM_PARTS; i++) {
			dparts[i] = (double) parts[i];
		}
	}
}

/* Compute time value from (double) parts. */
static double get_timeval_from_dparts(duk_context *ctx, double *dparts, int flags) {
	double tmp_time;
	double tmp_day;
	double d;
	int i;

	/* Expects 'this' at top of stack on entry. */

	/* Coerce all finite parts with ToInteger().  ToInteger() must not
	 * be called for NaN/Infinity because it will convert e.g. NaN to
	 * zero.  If ToInteger() has already been called, this has no side
	 * effects and is idempotent.
	 */
	for (i = 0; i < NUM_PARTS; i++) {
		d = dparts[i];
		if (isfinite(d)) {
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
	tmp_time += dparts[IDX_HOUR] * ((double) MS_HOUR);
	tmp_time += dparts[IDX_MINUTE] * ((double) MS_MINUTE);
	tmp_time += dparts[IDX_SECOND] * ((double) MS_SECOND);
	tmp_time += dparts[IDX_MILLISECOND];

	/* MakeDay */
	tmp_day = make_day(dparts[IDX_YEAR], dparts[IDX_MONTH], dparts[IDX_DAY]);

	/* MakeDate */
	d = tmp_day * ((double) MS_DAY) + tmp_time;

	DUK_DPRINT("time=%lf day=%lf --> timeval=%lf", tmp_time, tmp_day, d);

	/* Optional UTC conversion followed by TimeClip().
	 * Note that this also handles Infinity -> NaN conversion.
	 */
	if (flags & FLAG_LOCALTIME) {
		/* Note: DST adjustment is determined using local time. */
		d -= get_local_tzoffset(ctx, d) * 1000;
	}
	d = timeclip(d);

	return d;
}

/* Set timeval to 'this' from dparts, push the new time value onto the
 * value stack and return 1 (caller can then tailcall us).  Expects
 * the value stack to contain 'this' on the stack top.
 */
static int set_this_timeval_from_dparts(duk_context *ctx, double *dparts, int flags) {
	double d;

	/* [ ... this ] */

	d = get_timeval_from_dparts(ctx, dparts, flags);
	duk_push_number(ctx, d);  /* -> [ ... this timeval_new ] */
	duk_dup_top(ctx);         /* -> [ ... this timeval_new timeval_new ] */
	duk_put_prop_stridx(ctx, -3, DUK_HEAP_STRIDX_INT_VALUE);

	/* stack top: new time value */
	return 1;
}

/* Helper for string conversion calls: check 'this' binding, get the
 * internal time value, and format date and/or time in a few formats.
 */
static int to_string_helper(duk_context *ctx, int flags_and_sep) {
	double d;
	char yearstr[8];
	int parts[NUM_PARTS];
	char sep = (char) (flags_and_sep >> 16);
	const char *tzchar = "Z";

	d = push_this_and_get_timeval(ctx, flags_and_sep);
	if (isnan(d)) {
		duk_push_hstring_stridx(ctx, DUK_HEAP_STRIDX_INVALID_DATE);
		return 1;
	}

	timeval_to_parts(ctx, d, parts, NULL, FLAG_ONEBASED);
	DUK_ASSERT(parts[IDX_MONTH] >= 1 && parts[IDX_MONTH] <= 12);
	DUK_ASSERT(parts[IDX_DAY] >= 1 && parts[IDX_DAY] <= 31);

	/* Note: %06d for positive value, %07d for negative value to include sign and
	 * 6 digits.
	 */
	sprintf(yearstr,
	        parts[IDX_YEAR] >= 0 && parts[IDX_YEAR] <= 9999 ? "%04d" :
		        (parts[IDX_YEAR] >= 0 ? "+%06d" : "%07d"),
	        parts[IDX_YEAR]);

	if (flags_and_sep & FLAG_LOCALTIME) {
		tzchar++;  /* "Z" -> "" */
	}

	if ((flags_and_sep & FLAG_TOSTRING_DATE) && (flags_and_sep & FLAG_TOSTRING_TIME)) {
		duk_push_sprintf(ctx, "%s-%02d-%02d%c%02d:%02d:%02d.%03d%s",
		                 yearstr, parts[IDX_MONTH], parts[IDX_DAY], sep,
		                 parts[IDX_HOUR], parts[IDX_MINUTE], parts[IDX_SECOND],
		                 parts[IDX_MILLISECOND], tzchar);
	} else if (flags_and_sep & FLAG_TOSTRING_DATE) {
		duk_push_sprintf(ctx, "%s-%02d-%02d", yearstr, parts[IDX_MONTH], parts[IDX_DAY]);
	} else {
		DUK_ASSERT(flags_and_sep & FLAG_TOSTRING_TIME);
		duk_push_sprintf(ctx, "%02d:%02d:%02d.%03d%s", parts[IDX_HOUR], parts[IDX_MINUTE],
		                 parts[IDX_SECOND], parts[IDX_MILLISECOND], tzchar);
	}

	return 1;
}

/* Helper for component getter calls: check 'this' binding, get the
 * internal time value, split it into parts (either as UTC time or
 * local time), push a specified component as a return value to the
 * value stack and return 1 (caller can then tailcall us).
 */
static int get_part_helper(duk_context *ctx, int flags_and_idx) {
	double d;
	int parts[NUM_PARTS];
	int idx_part = flags_and_idx >> 16;

	DUK_ASSERT(idx_part >= 0 && idx_part < NUM_PARTS);

	d = push_this_and_get_timeval(ctx, flags_and_idx);
	if (isnan(d)) {
		duk_push_nan(ctx);
		return 1;
	}

	timeval_to_parts(ctx, d, parts, NULL, flags_and_idx);
	DUK_ASSERT(parts[IDX_MONTH] >= 1 && parts[IDX_MONTH] <= 12);
	DUK_ASSERT(parts[IDX_DAY] >= 1 && parts[IDX_DAY] <= 31);

	/* Setter APIs detect special year numbers (0...99) and apply a +1900
	 * only in certain cases.  The legacy getYear() getter applies -1900
	 * unconditionally.
	 */
	duk_push_int(ctx, (flags_and_idx & FLAG_SUB1900) ? parts[idx_part] - 1900 : parts[idx_part]);
	return 1;
}

/* Helper for component setter calls: check 'this' binding, get the
 * internal time value, split it into parts (either as UTC time or
 * local time), modify one or more components as specified, recompute
 * the time value, set it as the internal value.  Finally, push the
 * new time value as a return value to the value stack and return 1
 * (caller can then tailcall us).
 */
static int set_part_helper(duk_context *ctx, int flags_and_maxnargs) {
	double d;
	int parts[NUM_PARTS];
	double dparts[NUM_PARTS];
	int nargs;
	int maxnargs = flags_and_maxnargs >> 16;
	int idx_first, idx;
	int i;

	nargs = duk_get_top(ctx);
	d = push_this_and_get_timeval(ctx, flags_and_maxnargs);
	timeval_to_parts(ctx, d, parts, dparts, flags_and_maxnargs);

	/*
	 *  Determining which datetime components to overwrite based on
	 *  stack arguments is a bit complicated, but important to factor
	 *  out from setters themselves for compactness.
	 *
	 *  If FLAG_TIMESETTER, maxnargs indicates setter type:
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

	if (flags_and_maxnargs & FLAG_TIMESETTER) {
		DUK_ASSERT(maxnargs >= 1 && maxnargs <= 4);
		idx_first = IDX_MILLISECOND - (maxnargs - 1);
	} else {
		DUK_ASSERT(maxnargs >= 1 && maxnargs <= 3);
		idx_first = IDX_DAY - (maxnargs - 1);
	}
	DUK_ASSERT(idx_first >= 0 && idx_first < NUM_PARTS);

	for (i = 0; i < maxnargs; i++) {
		if (i >= nargs) {
			/* no argument given -> leave components untouched */
			break;
		}
		idx = idx_first + i;
		DUK_ASSERT(idx >= 0 && idx < NUM_PARTS);

		dparts[idx] = duk_to_number(ctx, i);

		if (idx == IDX_DAY) {
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
	return set_this_timeval_from_dparts(ctx, dparts, flags_and_maxnargs);
}

/* Apply ToNumber() to specified index; if ToInteger(val) in [0,99], add
 * 1900 and replace value at idx_val.
 */
static void twodigit_year_fixup(duk_context *ctx, int idx_val) {
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
static void set_parts_from_args(duk_context *ctx, double *dparts, int nargs) {
	double d;
	int i;
	int idx;

	/* FIXME: coercion order is wrong */
	twodigit_year_fixup(ctx, 0);  /* applies additional ToNumber(), no harm */

	for (i = 0; i < 7; i++) {
		/* Note: rely on index ordering */
		idx = IDX_YEAR + i;
		if (i < nargs) {
			d = duk_to_number(ctx, i);
			if (idx == IDX_DAY) {
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

	DUK_DPRINT("parts from args -> %lf %lf %lf %lf %lf %lf %lf %lf",
	           dparts[0], dparts[1], dparts[2], dparts[3],
	           dparts[4], dparts[5], dparts[6], dparts[7]);
}

/*
 *  Constructor calls
 */

int duk_builtin_date_constructor(duk_context *ctx) {
	int nargs = duk_get_top(ctx);
	int is_cons = duk_is_constructor_call(ctx);
	double dparts[NUM_PARTS];
	double d;

	DUK_DPRINT("Date constructor, nargs=%d, is_cons=%d", nargs, is_cons);

	duk_push_new_object_helper(ctx,
	                           DUK_HOBJECT_FLAG_EXTENSIBLE |
	                           DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_DATE),
	                           DUK_BIDX_DATE_PROTOTYPE);

	if (nargs == 0 || !is_cons) {
		d = timeclip(get_now_timeval(ctx));
		duk_push_number(ctx, d);
		duk_def_prop_stridx(ctx, -2, DUK_HEAP_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_W);
		if (!is_cons) {
			/* called as a normal function: return new Date().toString() */
			duk_to_string(ctx, -1);
		}
		return 1;
	} else if (nargs == 1) {
		duk_to_primitive(ctx, 0, DUK_HINT_NONE);
		if (duk_is_string(ctx, 0)) {
			/* FIXME: Parse */
			return -1;
		}
		d = timeclip(duk_to_number(ctx, 0));
		duk_push_number(ctx, d);
		duk_def_prop_stridx(ctx, -2, DUK_HEAP_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_W);
		return 1;
	}

	set_parts_from_args(ctx, dparts, nargs);

	/* Parts are in local time, convert when setting. */

	set_this_timeval_from_dparts(ctx, dparts, FLAG_LOCALTIME /*flags*/);  /* -> [ ... this timeval ] */
	duk_pop(ctx);  /* -> [ ... this ] */
	return 1;
}

int duk_builtin_date_constructor_parse(duk_context *ctx) {
	/* FIXME: check standard behavior and also usual behavior in other
	 * implementations.  For instance, V8 parses '2012-01-01' as UTC
	 * and '2012/01/01' as local time (?).
	 */
	return DUK_RET_UNIMPLEMENTED_ERROR;
}

int duk_builtin_date_constructor_utc(duk_context *ctx) {
	int nargs = duk_get_top(ctx);
	double dparts[NUM_PARTS];
	double d;

	/* Behavior for nargs < 2 is implementation dependent: currently we'll
	 * set a NaN time value (matching V8 behavior) in this case.
	 */

	if (nargs < 2) {
		duk_push_nan(ctx);
	} else {
		set_parts_from_args(ctx, dparts, nargs);
		d = get_timeval_from_dparts(ctx, dparts, 0 /*flags*/);
		duk_push_number(ctx, d);
	}
	return 1;
}

int duk_builtin_date_constructor_now(duk_context *ctx) {
	double d;

	d = get_now_timeval(ctx);
	DUK_ASSERT(timeclip(d) == d);  /* TimeClip() should never be necessary */
	duk_push_number(ctx, d);
	return 1;
}

/*
 *  Conversions
 *
 *  Human readable conversions are now basically ISO 8601 with a space
 *  (instead of 'T') as the date/time separator.  This is a good baseline
 *  and is platform independent.
 *
 *  FIXME: allow better string conversion on most common platforms.
 *  FIXME: no difference between locale and other string conversions now.
 */

#define  TOSTRING(ctx,flags,sep)  \
	to_string_helper((ctx),(flags) | (((int) (sep)) << 16))

int duk_builtin_date_prototype_to_string(duk_context *ctx) {
	return TOSTRING(ctx, FLAG_TOSTRING_DATE |
	                     FLAG_TOSTRING_TIME |
	                     FLAG_LOCALTIME, ' ');
}

int duk_builtin_date_prototype_to_date_string(duk_context *ctx) {
	return TOSTRING(ctx, FLAG_TOSTRING_DATE |
	                     FLAG_LOCALTIME, ' ');
}

int duk_builtin_date_prototype_to_time_string(duk_context *ctx) {
	return TOSTRING(ctx, FLAG_TOSTRING_TIME |
	                     FLAG_LOCALTIME, ' ');
}

int duk_builtin_date_prototype_to_locale_string(duk_context *ctx) {
	return duk_builtin_date_prototype_to_string(ctx);
}

int duk_builtin_date_prototype_to_locale_date_string(duk_context *ctx) {
	return duk_builtin_date_prototype_to_date_string(ctx);
}

int duk_builtin_date_prototype_to_locale_time_string(duk_context *ctx) {
	return duk_builtin_date_prototype_to_time_string(ctx);
}

int duk_builtin_date_prototype_value_of(duk_context *ctx) {
	/* This native function is also used for Date.prototype.getTime()
	 * as their behavior is identical.
	 */

	double d = push_this_and_get_timeval(ctx, 0 /*flags*/);  /* -> [ this ] */
	duk_push_number(ctx, d);
	return 1;
}

int duk_builtin_date_prototype_to_utc_string(duk_context *ctx) {
	/* E5.1 specification does not require a specific format, but
	 * result should be human readable.  The specification suggests
	 * using ISO 8601 format with a space (instead of 'T') separator
	 * if a more human readable format is not available.
	 */
	return TOSTRING(ctx, FLAG_TOSTRING_DATE |
	                     FLAG_TOSTRING_TIME, ' ');
}

int duk_builtin_date_prototype_to_iso_string(duk_context *ctx) {
	/* Unlike other conversion functions, toISOString() requires a
	 * RangeError for invalid date values.
	 */
	return TOSTRING(ctx, FLAG_NAN_TO_RANGE_ERROR |
	                     FLAG_TOSTRING_DATE |
	                     FLAG_TOSTRING_TIME, 'T');
}

int duk_builtin_date_prototype_to_json(duk_context *ctx) {
	/* Note: toJSON() is a generic function which works even if 'this'
	 * is not a Date.
	 */

	/* FIXME: untested */

	duk_push_this(ctx);
	duk_to_object(ctx, -1);

	duk_dup_top(ctx);
	duk_to_primitive(ctx, -1, DUK_HINT_NUMBER);
	if (duk_is_number(ctx, -1)) {
		double d = duk_get_number(ctx, -1);
		if (!isfinite(d)) {
			duk_push_null(ctx);
			return 1;
		}
	}
	duk_pop(ctx);

	duk_get_prop_stridx(ctx, -1, DUK_HEAP_STRIDX_TO_ISO_STRING);
	duk_dup(ctx, -2);  /* -> [ O toIsoString O ] */
	duk_call_method(ctx, 0);
	return 1;
}

/*
 *  Getters not related to component access.
 */

int duk_builtin_date_prototype_get_timezone_offset(duk_context *ctx) {
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

	d = push_this_and_get_timeval(ctx, 0 /*flags*/);
	tzoffset = get_local_tzoffset(ctx, d);
	duk_push_int(ctx, -tzoffset / 60);
	return 1;
}

/* Date.prototype.getTime() and Date.prototype.valueOf() have identical
 * behavior.  They have separate function objects, but share the same C
 * function (duk_builtin_date_prototype_value_of).
 */

/*
 *  Getters.
 *
 *  Implementing getters is quite easy.  The internal time value is either
 *  NaN, or represents milliseconds (without fractions) from Jan 1, 1970.
 *  The internal time value can be converted to integer parts, and each
 *  part will be normalized and will fit into a 32-bit signed integer.
 */

/* part index is encoded into flags field to reduce argument count */
#define  GET_PART(ctx,flags,partidx)  \
	get_part_helper((ctx), (flags) | ((partidx) << 16))

int duk_builtin_date_prototype_get_full_year(duk_context *ctx) {
	return GET_PART(ctx, FLAG_LOCALTIME, IDX_YEAR);
}

int duk_builtin_date_prototype_get_utc_full_year(duk_context *ctx) {
	return GET_PART(ctx, 0, IDX_YEAR);
}

int duk_builtin_date_prototype_get_month(duk_context *ctx) {
	return GET_PART(ctx, FLAG_LOCALTIME, IDX_MONTH);
}

int duk_builtin_date_prototype_get_utc_month(duk_context *ctx) {
	return GET_PART(ctx, 0, IDX_MONTH);
}

int duk_builtin_date_prototype_get_date(duk_context *ctx) {
	/* Note: 'date' means day-of-month, and is zero-based in internal
	 * calculations but public API expects it to be one-based.
	 */
	return GET_PART(ctx, FLAG_ONEBASED | FLAG_LOCALTIME, IDX_DAY);
}

int duk_builtin_date_prototype_get_utc_date(duk_context *ctx) {
	/* Note: 'date' means day-of-month.  Result should be one-based. */
	return GET_PART(ctx, FLAG_ONEBASED, IDX_DAY);
}

int duk_builtin_date_prototype_get_day(duk_context *ctx) {
	/* Note: 'day' means day-of-week */
	return GET_PART(ctx, FLAG_LOCALTIME, IDX_WEEKDAY);
}

int duk_builtin_date_prototype_get_utc_day(duk_context *ctx) {
	/* Note: 'day' means day-of-week */
	return GET_PART(ctx, 0, IDX_WEEKDAY);
}

int duk_builtin_date_prototype_get_hours(duk_context *ctx) {
	return GET_PART(ctx, FLAG_LOCALTIME, IDX_HOUR);
}

int duk_builtin_date_prototype_get_utc_hours(duk_context *ctx) {
	return GET_PART(ctx, 0, IDX_HOUR);
}

int duk_builtin_date_prototype_get_minutes(duk_context *ctx) {
	return GET_PART(ctx, FLAG_LOCALTIME, IDX_MINUTE);
}

int duk_builtin_date_prototype_get_utc_minutes(duk_context *ctx) {
	return GET_PART(ctx, 0, IDX_MINUTE);
}

int duk_builtin_date_prototype_get_seconds(duk_context *ctx) {
	return GET_PART(ctx, FLAG_LOCALTIME, IDX_SECOND);
}

int duk_builtin_date_prototype_get_utc_seconds(duk_context *ctx) {
	return GET_PART(ctx, 0, IDX_SECOND);
}

int duk_builtin_date_prototype_get_milliseconds(duk_context *ctx) {
	return GET_PART(ctx, FLAG_LOCALTIME, IDX_MILLISECOND);
}

int duk_builtin_date_prototype_get_utc_milliseconds(duk_context *ctx) {
	return GET_PART(ctx, 0, IDX_MILLISECOND);
}

/* Section B */
int duk_builtin_date_prototype_get_year(duk_context *ctx) {
	return GET_PART(ctx, FLAG_LOCALTIME | FLAG_SUB1900, IDX_YEAR);
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
 */

#define  SET_PART(ctx,flags,maxnargs) \
	set_part_helper((ctx), (flags) | ((maxnargs) << 16))

int duk_builtin_date_prototype_set_time(duk_context *ctx) {
	double d;

	(void) push_this_and_get_timeval(ctx, 0 /*flags*/); /* -> [ timeval this ] */
	d = timeclip(duk_to_number(ctx, 0));
	duk_push_number(ctx, d);
	duk_dup_top(ctx);
	duk_put_prop_stridx(ctx, -3, DUK_HEAP_STRIDX_INT_VALUE); /* -> [ timeval this timeval ] */

	return 1;
}

int duk_builtin_date_prototype_set_milliseconds(duk_context *ctx) {
	return SET_PART(ctx, FLAG_TIMESETTER | FLAG_LOCALTIME, 1);
}

int duk_builtin_date_prototype_set_utc_milliseconds(duk_context *ctx) {
	return SET_PART(ctx, FLAG_TIMESETTER, 1);
}

int duk_builtin_date_prototype_set_seconds(duk_context *ctx) {
	return SET_PART(ctx, FLAG_TIMESETTER | FLAG_LOCALTIME, 2);
}

int duk_builtin_date_prototype_set_utc_seconds(duk_context *ctx) {
	return SET_PART(ctx, FLAG_TIMESETTER, 2);
}

int duk_builtin_date_prototype_set_minutes(duk_context *ctx) {
	return SET_PART(ctx, FLAG_TIMESETTER | FLAG_LOCALTIME, 3);
}

int duk_builtin_date_prototype_set_utc_minutes(duk_context *ctx) {
	return SET_PART(ctx, FLAG_TIMESETTER, 3);
}

int duk_builtin_date_prototype_set_hours(duk_context *ctx) {
	return SET_PART(ctx, FLAG_TIMESETTER | FLAG_LOCALTIME, 4);
}

int duk_builtin_date_prototype_set_utc_hours(duk_context *ctx) {
	return SET_PART(ctx, FLAG_TIMESETTER, 4);
}

int duk_builtin_date_prototype_set_date(duk_context *ctx) {
	return SET_PART(ctx, FLAG_LOCALTIME, 1);
}

int duk_builtin_date_prototype_set_utc_date(duk_context *ctx) {
	return SET_PART(ctx, 0, 1);
}

int duk_builtin_date_prototype_set_month(duk_context *ctx) {
	return SET_PART(ctx, FLAG_LOCALTIME, 2);
}

int duk_builtin_date_prototype_set_utc_month(duk_context *ctx) {
	return SET_PART(ctx, 0, 2);
}

int duk_builtin_date_prototype_set_full_year(duk_context *ctx) {
	return SET_PART(ctx, FLAG_NAN_TO_ZERO | FLAG_LOCALTIME, 3);
}

int duk_builtin_date_prototype_set_utc_full_year(duk_context *ctx) {
	return SET_PART(ctx, FLAG_NAN_TO_ZERO, 3);
}

/* Section B */
int duk_builtin_date_prototype_set_year(duk_context *ctx) {
	/* Special year check.  NaN / Infinity will just flow through and
	 * ultimately result in a NaN internal time value.
	 */

	/* FIXME: coercion order */

	twodigit_year_fixup(ctx, 0);

	/* setYear() does not have optional arguments for setting month and
	 * day-in-month, but we indicate 'maxnargs' to be 3 to get the year
	 * written to the correct component index in set_part_helper().
	 * Because there are never optional arguments here (this is not a
	 * varargs function) only the year will be set.
	 */
	 
	DUK_ASSERT(duk_get_top(ctx) == 1);
	return SET_PART(ctx, FLAG_NAN_TO_ZERO, 3);
}

/* Date.prototype.toGMTString() and Date.prototype.toUTCString() are
 * required to be the same Ecmascript function object (!), so it is omitted
 * from here.
 */

