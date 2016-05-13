/*
 *  duk_get_now()
 */

/*===
timestamp is between [2010-01-01,2030-01-01[: yes
===*/

void test(duk_context *ctx) {
	duk_double_t now;

	now = duk_get_now(ctx);

	/*
	> new Date(2010,1,1).valueOf()
	1264975200000
	> new Date(2030,1,1).valueOf()
	1896127200000
	*/

	printf("timestamp is between [2010-01-01,2030-01-01[: %s\n",
	       now >= 1264975200000.0 && now < 1896127200000.0 ? "yes" : "no");
}
