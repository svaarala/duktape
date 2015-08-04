/*
 *  Duktape 0.10.0 reused a va_list argument when pushing a formatted string
 *  and the initial buffer size was not enough.  This bug testcase tries to
 *  trigger that issue.  Unfortunately it is not triggered with every compiler.
 *
 *  Reported by Andreas Oman.
 */

/*===
about to push, strlen(fmt)=256
push done
result: 1234523456123452345612345234561234523456xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
===*/

#define  CHARS40  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

void test(duk_context *ctx) {
	const char *fmt =
	        "%d%d%d%d%d%d%d%d"
	        CHARS40
	        CHARS40
	        CHARS40
	        CHARS40
	        CHARS40
	        CHARS40;

	/* The bug only manifests when the initial vsnprintf() attempt
	 * runs out of buffer space.  The initial buffer guess is
	 * min(256, strlen(fmt) + 16).
	 *
	 * Here the format string is 16 + 240 bytes, so the guess will
	 * be 256 bytes.  The formatted string will be 8x5 + 240 bytes
	 * = 280 bytes, so a retry happens.
	 */
	printf("about to push, strlen(fmt)=%d\n", (int) strlen(fmt));
	fflush(stdout);

	duk_push_sprintf(ctx,
	                 fmt,
	                 12345, 23456, 12345, 23456,
	                 12345, 23456, 12345, 23456);

	printf("push done\n");
	fflush(stdout);

	printf("result: %s\n", duk_get_string(ctx, -1));
	fflush(stdout);
}
