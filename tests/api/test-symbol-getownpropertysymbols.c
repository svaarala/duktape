/*===
*** test_basic (duk_safe_call)
0 Symbol(global) 102
1 Symbol(local) 103
2 Symbol() 104
3 Symbol() 105
4 Symbol(wellknown) 106
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx,
		"(function (o) {\n"
		"    Object.getOwnPropertySymbols(o).forEach(function (sym, idx) {\n"
		"        print(idx, String(sym), o[sym]);\n"
		"    });\n"
		"})\n");
	duk_push_object(ctx);

	duk_push_string(ctx, "\xFF" "applicationHidden");
	duk_push_uint(ctx, 101);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "\x80" "global");
	duk_push_uint(ctx, 102);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "\x81" "local" "\xFF" "unique");
	duk_push_uint(ctx, 103);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "\x81" "\xFF" "unique");  /* local, empty description */
	duk_push_uint(ctx, 104);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "\x81" "\xFF" "unique" "\xFF");  /* local, undefined description */
	duk_push_uint(ctx, 105);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "\x81" "wellknown" "\xFF");
	duk_push_uint(ctx, 106);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "\x82" "duktapeHidden");
	duk_push_uint(ctx, 107);
	duk_put_prop(ctx, -3);

	/* 0x83 to 0xBF are reserved but currently not interpreted as symbols.
	 * Sample some values.
	 */
	duk_push_string(ctx, "\x83" "notSymbol");
	duk_push_uint(ctx, 201);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\x84" "notSymbol");
	duk_push_uint(ctx, 202);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\x85" "notSymbol");
	duk_push_uint(ctx, 203);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\x86" "notSymbol");
	duk_push_uint(ctx, 204);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\x87" "notSymbol");
	duk_push_uint(ctx, 205);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\x88" "notSymbol");
	duk_push_uint(ctx, 206);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\x89" "notSymbol");
	duk_push_uint(ctx, 207);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\x8A" "notSymbol");
	duk_push_uint(ctx, 208);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\x8B" "notSymbol");
	duk_push_uint(ctx, 209);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\x8C" "notSymbol");
	duk_push_uint(ctx, 210);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\x8D" "notSymbol");
	duk_push_uint(ctx, 211);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\x8E" "notSymbol");
	duk_push_uint(ctx, 212);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\x8F" "notSymbol");
	duk_push_uint(ctx, 213);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\x9F" "notSymbol");
	duk_push_uint(ctx, 214);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\xAF" "notSymbol");
	duk_push_uint(ctx, 215);
	duk_put_prop(ctx, -3);
	duk_push_string(ctx, "\xBF" "notSymbol");
	duk_push_uint(ctx, 216);
	duk_put_prop(ctx, -3);

	/* [ func obj ] */

	duk_call(ctx, 1);  /* -> [ res ] */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
