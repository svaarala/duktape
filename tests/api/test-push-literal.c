/*
 *  duk_push_literal()
 */

/*===
*** test_basic (duk_safe_call)
top: 1, string: ''
len: 0
top: 1, string: 'foo'
len: 3
top: 1, string: 'foobar'
len: 6
top: 1
len: 5
top: 1, string: 'foobar'
return str: 'foobar'
top: 1, string: 'foo'
len: 7
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	const char *str;
	duk_size_t len;

	(void) udata;

	/* Empty. */
	(void) duk_push_literal(ctx, "");
	printf("top: %ld, string: '%s'\n", (long) duk_get_top(ctx), duk_get_string(ctx, -1));
	str = duk_get_lstring(ctx, -1, &len);
	printf("len: %ld\n", (long) len);
	duk_pop(ctx);

	/* Normal string. */
	(void) duk_push_literal(ctx, "foo");
	printf("top: %ld, string: '%s'\n", (long) duk_get_top(ctx), duk_get_string(ctx, -1));
	str = duk_get_lstring(ctx, -1, &len);
	printf("len: %ld\n", (long) len);
	duk_pop(ctx);

	/* String can be concantenated and may involve parentheses. */
	(void) duk_push_literal(ctx, ("foo" "bar"));
	printf("top: %ld, string: '%s'\n", (long) duk_get_top(ctx), duk_get_string(ctx, -1));
	str = duk_get_lstring(ctx, -1, &len);
	printf("len: %ld\n", (long) len);
	duk_pop(ctx);

	/* Literal can also be produces using symbol macros. */
	(void) duk_push_literal(ctx, DUK_HIDDEN_SYMBOL("quux"));
	printf("top: %ld\n", (long) duk_get_top(ctx));
	str = duk_get_lstring(ctx, -1, &len);
	printf("len: %ld\n", (long) len);
	duk_pop(ctx);

	/* Return value is a pointer to the interned string data.  It may
	 * or may not be the same as the input.  Right now (Duktape 2.3)
	 * it is always different from the input because there are no
	 * optimizations to take advatange of the literal (e.g. referring
	 * to it as an external string).
	 */
	str = duk_push_literal(ctx, ("foo" "bar"));
	printf("top: %ld, string: '%s'\n", (long) duk_get_top(ctx), duk_get_string(ctx, -1));
	printf("return str: '%s'\n", str);
	duk_pop(ctx);

	/* Embedded NUL.  Behavior depends on whether sizeof() is used (prefer
	 * speed) or whether duk_push_literal() is compiled as duk_push_string()
	 * (prefer size).  Test for default, prefer speed behavior.
	 *
	 * Application code SHOULD NOT make calls like this.
	 */
	(void) duk_push_literal(ctx, "foo" "\x00" "bar");
	printf("top: %ld, string: '%s'\n", (long) duk_get_top(ctx), duk_get_string(ctx, -1));
	str = duk_get_lstring(ctx, -1, &len);
	printf("len: %ld\n", (long) len);
	duk_pop(ctx);

	/* Push a literal (which pins it temporarily), force mark-and-sweep to
	 * free it, repush, etc.  This exercises freeing of temporarily pinned
	 * literals, and is useful for assertion testing.
	 */
	(void) duk_push_literal(ctx, "canBeCollected");
	duk_pop(ctx);
	duk_gc(ctx, 0);
	duk_gc(ctx, 0);
	(void) duk_push_literal(ctx, "canBeCollected");
	duk_pop(ctx);
	duk_gc(ctx, 0);
	duk_gc(ctx, 0);
	(void) duk_push_literal(ctx, "canBeCollected");
	duk_pop(ctx);
	duk_gc(ctx, 0);
	duk_gc(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
