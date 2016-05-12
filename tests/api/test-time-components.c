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
year: 2016
month: 1
day: 2
hour: 3
minute: 4
second: 5
millisecond: 6.000000
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	duk_time_components comp;

	(void) udata;

	/* Ecmascript time-to-components (UTC):
	 *
	 *   - d.getUTCMonth() is zero-based.
	 *   - d.getUTCDate() is one-based.
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
		"})();");

	/* C API equivalent:
	 *
	 *   - comp.month is one-based.
	 *   - comp.day is one-based.
	 */

	duk_time_to_components(ctx, 1451703845006.0, &comp);
	printf("year: %d\n", (int) comp.year);
	printf("month: %d\n", (int) comp.month);
	printf("day: %d\n", (int) comp.day);
	printf("hour: %d\n", (int) comp.hour);
	printf("minute: %d\n", (int) comp.minute);
	printf("second: %d\n", (int) comp.second);
	printf("millisecond: %lf\n", (double) comp.millisecond);

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

	/* Ecmascript components-to-time (UTC):
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

	/* C API equivalent:
	 *
	 *   - No special handling for two-digit years.
	 *   - comp.month is one-based.
	 *   - comp.day is one-based.
	 */

	memset((void *) &comp, 0, sizeof(comp));
	comp.year = 2016;
	comp.month = 1;
	comp.day = 2;
	comp.hour = 3;
	comp.minute = 4;
	comp.second = 5;
	comp.weekday = 0;  /* ignored */
	comp.millisecond = 6.0;

	t = duk_components_to_time(ctx, &comp);
	printf("time: %lf\n", t);

	memset((void *) &comp, 0, sizeof(comp));
	comp.year = 2016;
	comp.month = 1;
	comp.day = 2;
	comp.hour = 3;
	comp.minute = 0;
	comp.second = 4 * 60 + 5;  /* wrapping: 4 minutes, 5 seconds */
	comp.weekday = 0;  /* ignored */
	comp.millisecond = 6.0;

	t = duk_components_to_time(ctx, &comp);
	printf("time: %lf\n", t);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
}
