/*
 *  This is a pretty rare case, but if the built-in eval() function is
 *  called from C without any activations on the callstack, the eval()
 *  should and be handled as an indirect eval.  Note that this is different
 *  than calling the Duktape API eval call.  We can contrive this situation
 *  by using a bound eval().
 */

/*===
hello from eval
rc=0
final top: 0
===*/

void test(duk_context *ctx) {
	duk_ret_t rc;

        duk_eval_string(ctx, "this.testFunc = eval.bind(this, 'print(\\'hello from eval\\')');");
	duk_pop(ctx);

	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "testFunc");
	rc = duk_pcall(ctx, 0);
	printf("rc=%d\n", (int) rc);
	if (rc != 0) {
		/* unexpected error */
		printf("error=%s\n", duk_safe_to_string(ctx, -1));
	}
	duk_pop_2(ctx);

        printf("final top: %ld\n", (long) duk_get_top(ctx));
}
