/*===
00:  und=1 null=0 noru=1 bool=0 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=1 objcoerc=0
01:  und=0 null=1 noru=1 bool=0 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=1 objcoerc=0
02:  und=0 null=0 noru=0 bool=1 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=1 objcoerc=1
03:  und=0 null=0 noru=0 bool=1 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=1 objcoerc=1
04:  und=0 null=0 noru=0 bool=0 num=1 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=1 objcoerc=1
05:  und=0 null=0 noru=0 bool=0 num=1 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=1 objcoerc=1
06:  und=0 null=0 noru=0 bool=0 num=1 nan=1 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=1 objcoerc=1
07:  und=0 null=0 noru=0 bool=0 num=1 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=1 objcoerc=1
08:  und=0 null=0 noru=0 bool=0 num=1 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=1 objcoerc=1
09:  und=0 null=0 noru=0 bool=0 num=0 nan=0 str=1 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=1 objcoerc=1
10:  und=0 null=0 noru=0 bool=0 num=0 nan=0 str=1 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=1 objcoerc=1
11:  und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=0 objcoerc=1
12:  und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=1 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=0 objcoerc=1
13:  und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=0 fun=1 cfun=1 efun=0 bfun=0 call=1 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=0 objcoerc=1
14:  und=0 null=0 noru=0 bool=0 num=0 nan=0 str=1 obj=1 arr=0 fun=1 cfun=0 efun=1 bfun=0 call=1 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=0 objcoerc=1
15:  und=0 null=0 noru=0 bool=0 num=0 nan=0 str=1 obj=1 arr=0 fun=1 cfun=0 efun=0 bfun=1 call=1 thr=0 buf=0 dyn=0 fix=0 ptr=0 prim=0 objcoerc=1
16:  und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=1 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=1 buf=0 dyn=0 fix=0 ptr=0 prim=0 objcoerc=1
17:  und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=1 dyn=0 fix=1 ptr=0 prim=1 objcoerc=0
18:  und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=1 dyn=1 fix=0 ptr=0 prim=1 objcoerc=0
19:  und=0 null=0 noru=0 bool=0 num=0 nan=0 str=0 obj=0 arr=0 fun=0 cfun=0 efun=0 bfun=0 call=0 thr=0 buf=0 dyn=0 fix=0 ptr=1 prim=1 objcoerc=0
===*/

#include <math.h>

int my_c_func(duk_context *ctx) {
	return 0;
}

void test(duk_context *ctx) {
	int i, n;

	/*
	 *  push test values
	 */

	/* 0 */
	duk_push_undefined(ctx);

	/* 1 */
	duk_push_null(ctx);

	/* 2 */
	duk_push_true(ctx);

	/* 3 */
	duk_push_false(ctx);

	/* 4 */
	duk_push_int(ctx, 123);

	/* 5 */
	duk_push_number(ctx, 123.4);

	/* 6 */
	duk_push_nan(ctx);

	/* 7 */
	duk_push_number(ctx, INFINITY);

	/* 8 */
	duk_push_number(ctx, -INFINITY);

	/* 9 */
	duk_push_string(ctx, "");

	/* 10 */
	duk_push_string(ctx, "foo");

	/* 11 */
	duk_push_new_object(ctx);

	/* 12 */
	duk_push_new_array(ctx);

	/* 13 */
	duk_push_new_c_function(ctx, my_c_func, DUK_VARARGS);

	/* 14 */
#if 0
	duk_eval(ctx, "(function() { print('hello'); })");
#else
	duk_push_string(ctx, "FIXME");
#endif

	/* 15 */
#if 0
	duk_eval(ctx, "escape.bind(null, 'foo')");
#else
	duk_push_string(ctx, "FIXME");
#endif

	/* 16 */
	duk_push_new_thread(ctx);

	/* 17 */
	duk_push_new_buffer(ctx, 1024, 0 /*dynamic*/);

	/* 18 */
	duk_push_new_buffer(ctx, 1024, 1 /*dynamic*/);

	/* 19 */
	duk_push_pointer(ctx, (void *) 0xf00);

	/*
	 *  call checkers for each
	 */

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		printf("%02d: ", i);
		printf(" und=%d", duk_is_undefined(ctx, i));
		printf(" null=%d", duk_is_null(ctx, i));
		printf(" noru=%d", duk_is_null_or_undefined(ctx, i));
		printf(" bool=%d", duk_is_boolean(ctx, i));
		printf(" num=%d", duk_is_number(ctx, i));
		printf(" nan=%d", duk_is_nan(ctx, i));
		printf(" str=%d", duk_is_string(ctx, i));
		printf(" obj=%d", duk_is_object(ctx, i));
		printf(" arr=%d", duk_is_array(ctx, i));
		printf(" fun=%d", duk_is_function(ctx, i));
		printf(" cfun=%d", duk_is_c_function(ctx, i));
		printf(" efun=%d", duk_is_ecmascript_function(ctx, i));
		printf(" bfun=%d", duk_is_bound_function(ctx, i));
		printf(" call=%d", duk_is_callable(ctx, i));
		printf(" thr=%d", duk_is_thread(ctx, i));
		printf(" buf=%d", duk_is_buffer(ctx, i));
		printf(" dyn=%d", duk_is_dynamic(ctx, i));
		printf(" fix=%d", duk_is_fixed(ctx, i));
		printf(" ptr=%d", duk_is_pointer(ctx, i));
		printf(" prim=%d", duk_is_primitive(ctx, i));
		printf(" objcoerc=%d", duk_is_object_coercible(ctx, i));
		printf("\n");
	}
}

