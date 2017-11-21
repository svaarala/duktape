/*
 *  duk_random()
 */

/*===
average is sane: 1
done
===*/

void test(duk_context *ctx) {
	long i;
	double d;
	double sum = 0.0;
	double count = 0.0;
	double avg;

	for (i = 0; i < 1000000L; i++) {
		d = (double) duk_random(ctx);
		if (!(d >= 0.0 && d < 1.0)) {
			printf("failed: %lf\n", d);
		}
		sum += d;
		count += 1.0;
	}
	avg = sum / count;

	/* The range here is much larger than actually expected, and here
	 * just to catch cases where duk_random() would consistently return
	 * e.g. 0.0 or some other fixed value.
	 */
	printf("average is sane: %d\n", (avg >= 0.45 && avg <= 0.55) ? 1 : 0);

	printf("done\n");
}
