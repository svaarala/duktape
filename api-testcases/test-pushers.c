/*===
top=1 type=1 bool=0 num=nan clen=0 str='(null)' str-is-NULL=1 ptr-is-NULL=1
top=2 type=2 bool=0 num=nan clen=0 str='(null)' str-is-NULL=1 ptr-is-NULL=1
top=3 type=3 bool=1 num=nan clen=0 str='(null)' str-is-NULL=1 ptr-is-NULL=1
top=4 type=3 bool=0 num=nan clen=0 str='(null)' str-is-NULL=1 ptr-is-NULL=1
top=5 type=3 bool=1 num=nan clen=0 str='(null)' str-is-NULL=1 ptr-is-NULL=1
top=6 type=3 bool=0 num=nan clen=0 str='(null)' str-is-NULL=1 ptr-is-NULL=1
top=7 type=3 bool=1 num=nan clen=0 str='(null)' str-is-NULL=1 ptr-is-NULL=1
top=8 type=4 bool=0 num=123.400000 clen=0 str='(null)' str-is-NULL=1 ptr-is-NULL=1
top=9 type=4 bool=0 num=234.000000 clen=0 str='(null)' str-is-NULL=1 ptr-is-NULL=1
top=10 type=4 bool=0 num=nan clen=0 str='(null)' str-is-NULL=1 ptr-is-NULL=1
-> res is non-NULL
top=11 type=5 bool=0 num=nan clen=3 str='foo' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=12 type=5 bool=0 num=nan clen=3 str='foo' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=13 type=5 bool=0 num=nan clen=0 str='' str-is-NULL=0 ptr-is-NULL=1
-> res is NULL
top=14 type=2 bool=0 num=nan clen=0 str='(null)' str-is-NULL=1 ptr-is-NULL=1
-> res is non-NULL
top=15 type=5 bool=0 num=nan clen=4 str='foob' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=16 type=5 bool=0 num=nan clen=6 str='foob' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=17 type=5 bool=0 num=nan clen=1 str='' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=18 type=5 bool=0 num=nan clen=0 str='' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=19 type=5 bool=0 num=nan clen=0 str='' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=20 type=5 bool=0 num=nan clen=0 str='' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=21 type=5 bool=0 num=nan clen=3 str='foo' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=22 type=5 bool=0 num=nan clen=22 str='foo 123 bar 0x1234cafe' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=23 type=5 bool=0 num=nan clen=0 str='' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=24 type=5 bool=0 num=nan clen=0 str='' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=25 type=5 bool=0 num=nan clen=11 str='test: 2+3=5' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=26 type=5 bool=0 num=nan clen=0 str='' str-is-NULL=0 ptr-is-NULL=1
-> res is non-NULL
top=27 type=5 bool=0 num=nan clen=0 str='' str-is-NULL=0 ptr-is-NULL=1
top=28 type=8 bool=0 num=nan clen=0 str='(null)' str-is-NULL=1 ptr-is-NULL=1
top=29 type=8 bool=0 num=nan clen=0 str='(null)' str-is-NULL=1 ptr-is-NULL=0
===*/

#define  PRINTTOP()     print_top(ctx)
#define  PRINTRESTOP()  do { print_res(res); print_top(ctx); } while(0)

static void print_res(const char *res) {
	printf("-> res is %s\n", (res ? "non-NULL" : "NULL"));
}

static void print_top(duk_context *ctx) {
	duk_size_t clen = 0;
	if (duk_is_string(ctx, -1)) {
		clen = duk_get_length(ctx, -1);
	}
	printf("top=%ld type=%ld bool=%d num=%.6lf clen=%ld str='%s' str-is-NULL=%d ptr-is-NULL=%d\n",
	       (long) duk_get_top(ctx), (long) duk_get_type(ctx, -1), (int) duk_get_boolean(ctx, -1),
	       (double) duk_get_number(ctx, -1), (long) clen, duk_get_string(ctx, -1),
	       (duk_get_string(ctx, -1) ? 0 : 1),
	       (duk_get_pointer(ctx, -1) ? 0 : 1));
}

static const char *test_vsprintf_3x_int(duk_context *ctx, ...) {
	va_list ap;
	const char *res;

	va_start(ap, ctx);
	res = duk_push_vsprintf(ctx, "test: %d+%d=%d", ap);
	va_end(ap);

	return res;
}

static const char *test_vsprintf_empty(duk_context *ctx, ...) {
	va_list ap;
	const char *res;

	va_start(ap, ctx);
	res = duk_push_vsprintf(ctx, "", ap);
	va_end(ap);

	return res;
}

static const char *test_vsprintf_null(duk_context *ctx, ...) {
	va_list ap;
	const char *res;

	va_start(ap, ctx);
	res = duk_push_vsprintf(ctx, NULL, ap);
	va_end(ap);

	return res;
}

void test(duk_context *ctx) {
	const char *res;

	duk_push_undefined(ctx); PRINTTOP();
	duk_push_null(ctx); PRINTTOP();
	duk_push_true(ctx); PRINTTOP();
	duk_push_false(ctx); PRINTTOP();
	duk_push_boolean(ctx, -1); PRINTTOP();
	duk_push_boolean(ctx, 0); PRINTTOP();
	duk_push_boolean(ctx, 1); PRINTTOP();
	duk_push_number(ctx, 123.4); PRINTTOP();
	duk_push_int(ctx, 234); PRINTTOP();
	duk_push_nan(ctx); PRINTTOP();

	res = duk_push_string(ctx, "foo"); PRINTRESTOP();
	res = duk_push_string(ctx, "foo\0bar\0"); PRINTRESTOP();
	res = duk_push_string(ctx, ""); PRINTRESTOP();    /* pushes empty */
	res = duk_push_string(ctx, NULL); PRINTRESTOP();  /* pushes a NULL */

	res = duk_push_lstring(ctx, "foobar", 4); PRINTRESTOP();
	res = duk_push_lstring(ctx, "foob\0\0", 6); PRINTRESTOP();
	res = duk_push_lstring(ctx, "\0", 1); PRINTRESTOP();  /* pushes 1-byte string (0x00) */
	res = duk_push_lstring(ctx, "\0", 0); PRINTRESTOP();  /* pushes empty */
	res = duk_push_lstring(ctx, NULL, 0); PRINTRESTOP();  /* pushes empty */
	res = duk_push_lstring(ctx, NULL, 10); PRINTRESTOP(); /* pushes empty */

	res = duk_push_sprintf(ctx, "foo"); PRINTRESTOP();
	res = duk_push_sprintf(ctx, "foo %d %s 0x%08lx", 123, "bar", (long) 0x1234cafe); PRINTRESTOP();
	res = duk_push_sprintf(ctx, ""); PRINTRESTOP();
	res = duk_push_sprintf(ctx, NULL); PRINTRESTOP();

	res = test_vsprintf_3x_int(ctx, 2, 3, 5); PRINTRESTOP();
	res = test_vsprintf_empty(ctx, 2, 3, 5); PRINTRESTOP();
	res = test_vsprintf_null(ctx, 2, 3, 5); PRINTRESTOP();

	duk_push_pointer(ctx, (void *) 0); PRINTTOP();
	duk_push_pointer(ctx, (void *) 0xdeadbeef); PRINTTOP();
}
