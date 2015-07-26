/*===
stack[0] --> type=1 mask=0x00000002 undefined bool=0 num=nan str=(null) buf-is-null=1 ptr=(nil) isobj=0 isarr=0 isfunc=0
stack[1] --> type=2 mask=0x00000004 null bool=0 num=nan str=(null) buf-is-null=1 ptr=(nil) isobj=0 isarr=0 isfunc=0
stack[2] --> type=3 mask=0x00000008 boolean bool=0 num=nan str=(null) buf-is-null=1 ptr=(nil) isobj=0 isarr=0 isfunc=0
stack[3] --> type=3 mask=0x00000008 boolean bool=1 num=nan str=(null) buf-is-null=1 ptr=(nil) isobj=0 isarr=0 isfunc=0
stack[4] --> type=4 mask=0x00000010 number bool=0 num=234.000000 str=(null) buf-is-null=1 ptr=(nil) isobj=0 isarr=0 isfunc=0
stack[5] --> type=5 mask=0x00000020 string bool=0 num=nan str=foo buf-is-null=1 ptr=(nil) isobj=0 isarr=0 isfunc=0
stack[6] --> type=6 mask=0x00000040 object bool=0 num=nan str=(null) buf-is-null=1 ptr=(nil) isobj=1 isarr=0 isfunc=0
stack[7] --> type=6 mask=0x00000040 object bool=0 num=nan str=(null) buf-is-null=1 ptr=(nil) isobj=1 isarr=1 isfunc=0
stack[8] --> type=6 mask=0x00000040 object bool=0 num=nan str=(null) buf-is-null=1 ptr=(nil) isobj=1 isarr=0 isfunc=1
stack[9] --> type=7 mask=0x00000080 buffer bool=0 num=nan str=(null) buf-is-null=0 ptr=(nil) isobj=0 isarr=0 isfunc=0
stack[10] --> type=7 mask=0x00000080 buffer bool=0 num=nan str=(null) buf-is-null=0 ptr=(nil) isobj=0 isarr=0 isfunc=0
stack[11] --> type=8 mask=0x00000100 pointer bool=0 num=nan str=(null) buf-is-null=1 ptr=0xdeadbeef isobj=0 isarr=0 isfunc=0
stack[12] --> type=0 mask=0x00000001 none bool=0 num=nan str=(null) buf-is-null=1 ptr=(nil) isobj=0 isarr=0 isfunc=0
===*/

/* Test basic type handling.  The expected output is dependent on specific
 * values of API constants.  This is intentional, although not very ideal.
 */

static duk_ret_t my_c_func(duk_context *ctx) {
	return 0;
}

void test(duk_context *ctx) {
	duk_idx_t i, n;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_boolean(ctx, 0);
	duk_push_boolean(ctx, 123);
	duk_push_number(ctx, 234);
	duk_push_string(ctx, "foo");
	duk_push_object(ctx);
	duk_push_array(ctx);
	duk_push_c_function(ctx, my_c_func, DUK_VARARGS);
	duk_push_fixed_buffer(ctx, 1024);
	duk_push_dynamic_buffer(ctx, 1024);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);

	n = duk_get_top(ctx);
	for (i = 0; i < n + 1; i++) {  /* end on invalid index on purpose */
		duk_int_t typeval, typemask;

		typeval = duk_get_type(ctx, i);
		typemask = duk_get_type_mask(ctx, i);

		printf("stack[%ld] --> type=%ld mask=0x%08lx ",
		       (long) i, (long) typeval, (long) typemask);

		switch(duk_get_type(ctx, i)) {
		case DUK_TYPE_NONE:       printf("none"); break;
		case DUK_TYPE_UNDEFINED:  printf("undefined"); break;
		case DUK_TYPE_NULL:       printf("null"); break;
		case DUK_TYPE_BOOLEAN:    printf("boolean"); break;
		case DUK_TYPE_NUMBER:     printf("number"); break;
		case DUK_TYPE_STRING:     printf("string"); break;
		case DUK_TYPE_OBJECT:     printf("object"); break;
		case DUK_TYPE_BUFFER:     printf("buffer"); break;
		case DUK_TYPE_POINTER:    printf("pointer"); break;
		default:                  printf("unknown(%d)", (int) duk_get_type(ctx, i)); break;
		}

		printf(" bool=%d num=%lf str=%s buf-is-null=%d ptr=%p",
		       (int) duk_get_boolean(ctx, i),
		       (double) duk_get_number(ctx, i),
		       duk_get_string(ctx, i),
		       (duk_get_buffer(ctx, i, NULL) == NULL ? 1 : 0),
		       duk_get_pointer(ctx, i));

		printf(" isobj=%d isarr=%d isfunc=%d",
		       (int) duk_is_object(ctx, i),
		       (int) duk_is_array(ctx, i),
		       (int) duk_is_function(ctx, i));

		printf("\n");
	}
}
