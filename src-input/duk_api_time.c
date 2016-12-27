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

	DUK_ASSERT(dparts[DUK_DATE_IDX_MONTH] >= 1.0 && dparts[DUK_DATE_IDX_MONTH] <= 12.0);
	comp->year = dparts[DUK_DATE_IDX_YEAR];
	comp->month = dparts[DUK_DATE_IDX_MONTH] - 1.0;
	comp->day = dparts[DUK_DATE_IDX_DAY];
	comp->hours = dparts[DUK_DATE_IDX_HOUR];
	comp->minutes = dparts[DUK_DATE_IDX_MINUTE];
	comp->seconds = dparts[DUK_DATE_IDX_SECOND];
	comp->milliseconds = dparts[DUK_DATE_IDX_MILLISECOND];
	comp->weekday = dparts[DUK_DATE_IDX_WEEKDAY];
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

	dparts[DUK_DATE_IDX_YEAR] = comp->year;
	dparts[DUK_DATE_IDX_MONTH] = comp->month;
	dparts[DUK_DATE_IDX_DAY] = comp->day - 1.0;
	dparts[DUK_DATE_IDX_HOUR] = comp->hours;
	dparts[DUK_DATE_IDX_MINUTE] = comp->minutes;
	dparts[DUK_DATE_IDX_SECOND] = comp->seconds;
	dparts[DUK_DATE_IDX_MILLISECOND] = comp->milliseconds;
	dparts[DUK_DATE_IDX_WEEKDAY] = 0;  /* ignored */

	d = duk_bi_date_get_timeval_from_dparts(dparts, flags);

	return d;
}
