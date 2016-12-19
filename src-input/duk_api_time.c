/*
 *  Date/time.
 */

#include "duk_internal.h"

DUK_EXTERNAL duk_double_t duk_get_now(duk_context *ctx) {
	return ((duk_double_t) DUK_USE_DATE_GET_NOW((ctx)));
}

DUK_EXTERNAL void duk_time_to_components(duk_context *ctx, duk_double_t timeval, duk_time_components *comp) {
	duk_int_t parts[DUK_DATE_IDX_NUM_PARTS];
	duk_double_t dparts[DUK_DATE_IDX_NUM_PARTS];
	duk_uint_t flags;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(comp != NULL);  /* XXX: or check? */
	DUK_UNREF(ctx);

	/* Convert as one-based, but change month to zero-based to match the
	 * Ecmascript Date built-in behavior 1:1.
	 */
	flags = DUK_DATE_FLAG_ONEBASED | DUK_DATE_FLAG_NAN_TO_ZERO;

	duk_bi_date_timeval_to_parts(timeval, parts, dparts, flags);

	/* XXX: expensive conversion */
	DUK_ASSERT(comp->month >= 1 && comp->month <= 12);
	comp->year = (duk_uint_t) parts[DUK_DATE_IDX_YEAR];
	comp->month = (duk_uint_t) parts[DUK_DATE_IDX_MONTH] - 1;
	comp->day = (duk_uint_t) parts[DUK_DATE_IDX_DAY];
	comp->hour = (duk_uint_t) parts[DUK_DATE_IDX_HOUR];
	comp->minute = (duk_uint_t) parts[DUK_DATE_IDX_MINUTE];
	comp->second = (duk_uint_t) parts[DUK_DATE_IDX_SECOND];
	comp->weekday = (duk_uint_t) parts[DUK_DATE_IDX_WEEKDAY];
	comp->millisecond = (duk_double_t) parts[DUK_DATE_IDX_MILLISECOND];
}

DUK_EXTERNAL duk_double_t duk_components_to_time(duk_context *ctx, duk_time_components *comp) {
	duk_double_t d;
	duk_double_t dparts[DUK_DATE_IDX_NUM_PARTS];
	duk_uint_t flags;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(comp != NULL);  /* XXX: or check? */
	DUK_UNREF(ctx);

	/* Match Date constructor behavior (with UTC time).  Month is given
	 * as zero-based.  Day-of-month is given as one-based so normalize
	 * it to zero-based as the internal conversion helpers expects all
	 * components to be zero-based.
	 */
	flags = 0;

	/* XXX: expensive conversion; use array format in API instead, or unify
	 * time provider and time API to use same struct?
	 */

	dparts[DUK_DATE_IDX_YEAR] = (duk_double_t) comp->year;
	dparts[DUK_DATE_IDX_MONTH] = (duk_double_t) comp->month;
	dparts[DUK_DATE_IDX_DAY] = (duk_double_t) comp->day - 1.0;
	dparts[DUK_DATE_IDX_HOUR] = (duk_double_t) comp->hour;
	dparts[DUK_DATE_IDX_MINUTE] = (duk_double_t) comp->minute;
	dparts[DUK_DATE_IDX_SECOND] = (duk_double_t) comp->second;
	dparts[DUK_DATE_IDX_MILLISECOND] = comp->millisecond;
	dparts[DUK_DATE_IDX_WEEKDAY] = 0;  /* ignored */

	d = duk_bi_date_get_timeval_from_dparts(dparts, flags);

	return d;
}
