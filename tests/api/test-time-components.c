/*
 *  Time component handling.
 */

/*===
*** test_1 (duk_safe_call)
1451703845006
year 2016
month 0
day 2
hours 3
minutes 4
seconds 5
milliseconds 6
weekday 6
year: 2016
month: 0
day: 2
hours: 3
minutes: 4
seconds: 5
milliseconds: 6.000000
weekday: 6
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	duk_time_components comp;

	(void) udata;

	/* ECMAScript time-to-components (UTC):
	 *
	 *   - d.getUTCMonth() is zero-based.
	 *   - d.getUTCDate() (day in month) is one-based.
	 *   - d.getUTCDay() (weekday) is zero-based.
	 */

	duk_eval_string_noresult(ctx,
		"(function () {\n"
		"    var d = new Date(1451703845006);\n"  /* 2016-01-02 03:04:05.006Z */
		"    print(d.valueOf());\n"
		"    print('year', d.getUTCFullYear());\n"
		"    print('month', d.getUTCMonth());\n"
		"    print('day', d.getUTCDate());\n"
		"    print('hours', d.getUTCHours());\n"
		"    print('minutes', d.getUTCMinutes());\n"
		"    print('seconds', d.getUTCSeconds());\n"
		"    print('milliseconds', d.getUTCMilliseconds());\n"
		"    print('weekday', d.getUTCDay());\n"
		"})();");

	/* C API equivalent. */

	duk_time_to_components(ctx, 1451703845006.0, &comp);
	printf("year: %d\n", (int) comp.year);
	printf("month: %d\n", (int) comp.month);
	printf("day: %d\n", (int) comp.day);
	printf("hours: %d\n", (int) comp.hours);
	printf("minutes: %d\n", (int) comp.minutes);
	printf("seconds: %d\n", (int) comp.seconds);
	printf("milliseconds: %lf\n", (double) comp.milliseconds);
	printf("weekday: %d\n", (int) comp.weekday);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_2 (duk_safe_call)
1451703845006
1451703845006
time: 1451703845006.000000
time: 1451703845006.000000
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_2(duk_context *ctx, void *udata) {
	duk_time_components comp;
	duk_double_t t;

	(void) udata;

	/* ECMAScript components-to-time (UTC):
	 *
	 *   - Year argument has a hack for two-digit years (e.g. Date.UTC(99, ...)
	 *     creates a time value for year 1999.
	 *   - Month argument is zero-based.
	 *   - Day-of-month argument is one-based.
	 *   - Values are normalized automatically, so e.g. specifying
	 *     minutes as 120 is interpreted as 2 hours etc.
	 */

	duk_eval_string_noresult(ctx,
		"(function () {\n"
		"    var d = new Date(Date.UTC(2016, 0, 2, 3, 4, 5, 6));\n"  /* 2016-01-02 03:04:05.006Z */
		"    print(d.valueOf())\n"
		"    d = new Date(Date.UTC(2016, 0, 2, 3, 0, 4 * 60 + 5, 6));\n"  /* 2016-01-02 03:04:05.006Z */
		"    print(d.valueOf())\n"
		"})();");

	/* C API equivalent; note that:
	 *
	 *   - No special handling for two-digit years.
	 */

	memset((void *) &comp, 0, sizeof(comp));
	comp.year = 2016;
	comp.month = 0;
	comp.day = 2;
	comp.hours = 3;
	comp.minutes = 4;
	comp.seconds = 5;
	comp.milliseconds = 6.0;
	comp.weekday = 0;  /* ignored */

	t = duk_components_to_time(ctx, &comp);
	printf("time: %lf\n", t);

	memset((void *) &comp, 0, sizeof(comp));
	comp.year = 2016;
	comp.month = 0;
	comp.day = 2;
	comp.hours = 3;
	comp.minutes = 0;
	comp.seconds = 4 * 60 + 5;  /* wrapping: 4 minutes, 5 seconds */
	comp.milliseconds = 6.0;
	comp.weekday = 0;  /* ignored */

	t = duk_components_to_time(ctx, &comp);
	printf("time: %lf\n", t);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
}
