/*===
*** test_basic (duk_safe_call)
final top: 0
==> rc=0, result='undefined'
*** test_compaction (duk_safe_call)
true true
true true
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	duk_gc(ctx, 0);
	duk_gc(ctx, 0);
	duk_gc(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_compaction(duk_context *ctx, void *udata) {
	/* Force GC a few times to ensure voluntary GC won't surprise us. */
	duk_gc(ctx, 0);
	duk_gc(ctx, 0);

	/* Create a non-compact object. */
	duk_eval_string(ctx,
		"(function () {\n"
		"    var obj = { foo: 123, bar: 234, quux: 345 };\n"
		"    delete obj.foo;  // now has an empty slot\n"
		"    delete obj.bar;  // and another\n"
		"    return obj;\n"
		"})()\n");

	/* Before compaction, esize and enext will be > 1.  Don't test
	 * for specific values because they'll depend on resizing parameters.
	 */

	duk_eval_string(ctx,
		"(function (obj) {\n"
		"    var info, esize, enext;\n"
		"    info = Duktape.info(obj);\n"
		"    esize = ('esize' in info ? info.esize : info[5]);\n"
		"    enext = ('enext' in info ? info.enext : info[6]);\n"
		"    print(esize > 1, enext > 1);\n"
		"})\n");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_gc(ctx, DUK_GC_COMPACT);

	/* After compaction esize == enext == 1 because the object has only
	 * a single property.
	 */
	duk_eval_string(ctx,
		"(function (obj) {\n"
		"    var info, esize, enext;\n"
		"    info = Duktape.info(obj);\n"
		"    esize = ('esize' in info ? info.esize : info[5]);\n"
		"    enext = ('enext' in info ? info.enext : info[6]);\n"
		"    print(esize == 1, enext == 1);\n"
		"})\n");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_compaction);
}
