/*
 *  duk_def_prop()
 */

/*===
*** test_value_only (duk_safe_call)
top before: 4
top after: 2
"my_key" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
final top: 2
==> rc=0, result='undefined'
*** test_value_redefine (duk_safe_call)
top before: 4
top after: 2
"my_key" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
top before: 4
top after: 2
"my_key" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
top before: 4
==> rc=1, result='TypeError: not configurable'
*** test_value_attr_combinations (duk_safe_call)
top before: 3
top after: 1
top before: 3
top after: 1
top before: 3
top after: 1
top before: 3
top after: 1
top before: 3
top after: 1
top before: 3
top after: 1
top before: 3
top after: 1
top before: 3
top after: 1
"my_key_0" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
"my_key_1" {value:123,writable:true,enumerable:false,configurable:false} no-getter no-setter
"my_key_2" {value:123,writable:false,enumerable:true,configurable:false} no-getter no-setter
"my_key_3" {value:123,writable:true,enumerable:true,configurable:false} no-getter no-setter
"my_key_4" {value:123,writable:false,enumerable:false,configurable:true} no-getter no-setter
"my_key_5" {value:123,writable:true,enumerable:false,configurable:true} no-getter no-setter
"my_key_6" {value:123,writable:false,enumerable:true,configurable:true} no-getter no-setter
"my_key_7" {value:123,writable:true,enumerable:true,configurable:true} no-getter no-setter
final top: 1
==> rc=0, result='undefined'
*** test_value_attr_presence (duk_safe_call)
"my_key_1" {value:123,writable:true,enumerable:true,configurable:true} no-getter no-setter
"my_key_1" {value:123,writable:false,enumerable:true,configurable:true} no-getter no-setter
"my_key_1" {value:123,writable:false,enumerable:false,configurable:true} no-getter no-setter
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
"my_key_2" {value:321,writable:false,enumerable:false,configurable:true} no-getter no-setter
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
"my_key_2" {value:321,writable:true,enumerable:false,configurable:true} no-getter no-setter
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
"my_key_2" {value:321,writable:true,enumerable:true,configurable:true} no-getter no-setter
final top: 1
==> rc=0, result='undefined'
*** test_setget_only (duk_safe_call)
"my_key_1" {get:{_func:true},set:undefined,enumerable:false,configurable:false} getter no-setter
"my_key_1" {get:{_func:true},set:undefined,enumerable:false,configurable:false} getter no-setter
"my_key_2" {get:undefined,set:{_func:true},enumerable:false,configurable:true} no-getter setter
"my_key_1" {get:{_func:true},set:undefined,enumerable:false,configurable:false} getter no-setter
"my_key_2" {get:undefined,set:{_func:true},enumerable:false,configurable:true} no-getter setter
"my_key_3" {get:{_func:true},set:{_func:true},enumerable:false,configurable:false} getter setter
final top: 1
==> rc=0, result='undefined'
*** test_value_and_setget (duk_safe_call)
==> rc=1, result='TypeError: invalid descriptor'
*** test_writable_and_set (duk_safe_call)
==> rc=1, result='TypeError: invalid descriptor'
*** test_setget_undefined (duk_safe_call)
"my_key_1" {get:{_func:true},set:{_func:true},enumerable:false,configurable:true} getter setter
"my_key_1" {get:undefined,set:{_func:true},enumerable:false,configurable:true} no-getter setter
"my_key_1" {get:undefined,set:undefined,enumerable:false,configurable:true} no-getter no-setter
final top: 1
==> rc=0, result='undefined'
*** test_getter_nonobject (duk_safe_call)
"my_key_1" {get:{_func:true},set:{_func:true},enumerable:false,configurable:true} getter setter
==> rc=1, result='TypeError: unexpected type'
*** test_setter_nonobject (duk_safe_call)
"my_key_1" {get:{_func:true},set:{_func:true},enumerable:false,configurable:true} getter setter
==> rc=1, result='TypeError: unexpected type'
*** test_getter_noncallable (duk_safe_call)
==> rc=1, result='TypeError: not callable'
*** test_setter_noncallable (duk_safe_call)
==> rc=1, result='TypeError: not callable'
*** test_setget_lightfunc (duk_safe_call)
"my_key_1" {get:{_func:true},set:{_func:true},enumerable:false,configurable:true} func func
final top: 1
==> rc=0, result='undefined'
*** test_fail_nonextensible (duk_safe_call)
==> rc=1, result='TypeError: not extensible'
*** test_force_nonextensible (duk_safe_call)
"my_key_1" {value:321,writable:false,enumerable:false,configurable:false} no-getter no-setter
final top: 1
==> rc=0, result='undefined'
*** test_fail_set_configurable (duk_safe_call)
"my_key_1" {value:123,writable:false,enumerable:true,configurable:false} no-getter no-setter
==> rc=1, result='TypeError: not configurable'
*** test_force_set_configurable (duk_safe_call)
"my_key_1" {value:123,writable:false,enumerable:true,configurable:false} no-getter no-setter
"my_key_1" {value:123,writable:false,enumerable:true,configurable:true} no-getter no-setter
final top: 1
==> rc=0, result='undefined'
*** test_fail_set_enumerable (duk_safe_call)
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
==> rc=1, result='TypeError: not configurable'
*** test_force_set_enumerable (duk_safe_call)
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
"my_key_1" {value:123,writable:false,enumerable:true,configurable:false} no-getter no-setter
final top: 1
==> rc=0, result='undefined'
*** test_fail_set_writable (duk_safe_call)
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
==> rc=1, result='TypeError: not configurable'
*** test_force_set_writable (duk_safe_call)
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
"my_key_1" {value:123,writable:true,enumerable:false,configurable:false} no-getter no-setter
final top: 1
==> rc=0, result='undefined'
*** test_fail_set_value (duk_safe_call)
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
==> rc=1, result='TypeError: not configurable'
*** test_force_set_value (duk_safe_call)
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
"my_key_1" {value:321,writable:false,enumerable:false,configurable:false} no-getter no-setter
final top: 1
==> rc=0, result='undefined'
*** test_fail_set_getter (duk_safe_call)
"my_key_1" {get:{_func:true},set:{_func:true},enumerable:false,configurable:false} func func
==> rc=1, result='TypeError: not configurable'
*** test_force_set_getter (duk_safe_call)
"my_key_1" {get:{_func:true},set:{_func:true},enumerable:false,configurable:false} func func
"my_key_1" {get:{_func:true},set:{_func:true},enumerable:false,configurable:false} getter func
final top: 1
==> rc=0, result='undefined'
*** test_fail_set_setter (duk_safe_call)
"my_key_1" {get:{_func:true},set:{_func:true},enumerable:false,configurable:false} func func
==> rc=1, result='TypeError: not configurable'
*** test_force_set_setter (duk_safe_call)
"my_key_1" {get:{_func:true},set:{_func:true},enumerable:false,configurable:false} func func
"my_key_1" {get:{_func:true},set:{_func:true},enumerable:false,configurable:false} func setter
final top: 1
==> rc=0, result='undefined'
*** test_fail_data2accessor (duk_safe_call)
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
==> rc=1, result='TypeError: not configurable'
*** test_force_data2accessor (duk_safe_call)
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
"my_key_1" {get:{_func:true},set:{_func:true},enumerable:true,configurable:false} getter setter
final top: 1
==> rc=0, result='undefined'
*** test_fail_accessor2data (duk_safe_call)
"my_key_1" {get:{_func:true},set:{_func:true},enumerable:false,configurable:false} func func
==> rc=1, result='TypeError: not configurable'
*** test_force_accessor2data (duk_safe_call)
"my_key_1" {get:{_func:true},set:{_func:true},enumerable:false,configurable:false} func func
"my_key_1" {value:321,writable:true,enumerable:false,configurable:false} no-getter no-setter
final top: 1
==> rc=0, result='undefined'
*** test_fail_array_smaller (duk_safe_call)
"0" {value:"foo",writable:true,enumerable:true,configurable:true} no-getter no-setter
"1" {value:"bar",writable:true,enumerable:true,configurable:true} no-getter no-setter
"2" {value:"quux",writable:true,enumerable:true,configurable:false} no-getter no-setter
"3" {value:"baz",writable:true,enumerable:true,configurable:true} no-getter no-setter
"length" {value:4,writable:true,enumerable:false,configurable:false} no-getter no-setter
==> rc=1, result='TypeError: array length write failed'
*** test_force_array_smaller (duk_safe_call)
"0" {value:"foo",writable:true,enumerable:true,configurable:true} no-getter no-setter
"1" {value:"bar",writable:true,enumerable:true,configurable:true} no-getter no-setter
"2" {value:"quux",writable:true,enumerable:true,configurable:false} no-getter no-setter
"3" {value:"baz",writable:true,enumerable:true,configurable:true} no-getter no-setter
"length" {value:4,writable:true,enumerable:false,configurable:false} no-getter no-setter
"0" {value:"foo",writable:true,enumerable:true,configurable:true} no-getter no-setter
"length" {value:1,writable:true,enumerable:false,configurable:false} no-getter no-setter
json: ["foo"]
final top: 1
==> rc=0, result='undefined'
*** test_fail_array_smaller_nonwritablelength (duk_safe_call)
"0" {value:"foo",writable:true,enumerable:true,configurable:true} no-getter no-setter
"1" {value:"bar",writable:true,enumerable:true,configurable:true} no-getter no-setter
"2" {value:"quux",writable:true,enumerable:true,configurable:false} no-getter no-setter
"3" {value:"baz",writable:true,enumerable:true,configurable:true} no-getter no-setter
"length" {value:4,writable:false,enumerable:false,configurable:false} no-getter no-setter
==> rc=1, result='TypeError: array length non-writable'
*** test_force_array_smaller_nonwritablelength (duk_safe_call)
"0" {value:"foo",writable:true,enumerable:true,configurable:true} no-getter no-setter
"1" {value:"bar",writable:true,enumerable:true,configurable:true} no-getter no-setter
"2" {value:"quux",writable:true,enumerable:true,configurable:false} no-getter no-setter
"3" {value:"baz",writable:true,enumerable:true,configurable:true} no-getter no-setter
"length" {value:4,writable:false,enumerable:false,configurable:false} no-getter no-setter
"0" {value:"foo",writable:true,enumerable:true,configurable:true} no-getter no-setter
"length" {value:1,writable:false,enumerable:false,configurable:false} no-getter no-setter
json: ["foo"]
final top: 1
==> rc=0, result='undefined'
*** test_fail_nondeletable (duk_safe_call)
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
my_key_1 present: 1
==> rc=1, result='TypeError: not configurable'
*** test_force_nondeletable (duk_safe_call)
"my_key_1" {value:123,writable:false,enumerable:false,configurable:false} no-getter no-setter
my_key_1 present: 1
my_key_1 present: 0
final top: 1
==> rc=0, result='undefined'
===*/

static void dump_object(duk_context *ctx, duk_idx_t idx) {
	idx = duk_require_normalize_index(ctx, idx);

	/* The weird fn() helper is to handle lightfunc name printing (= avoid it). */
	duk_eval_string(ctx,
	    "(function (o) {\n"
	    "    Object.getOwnPropertyNames(o).forEach(function (k) {\n"
	    "        var pd = Object.getOwnPropertyDescriptor(o, k);\n"
	    "        function fn(x) { if (x.name !== 'getter' && x.name !== 'setter') { return 'func' }; return x.name; }\n"
	    "        print(Duktape.enc('jx', k), Duktape.enc('jx', pd), (pd.get ? fn(pd.get) : 'no-getter'), (pd.set ? fn(pd.set) : 'no-setter'));\n"
	    "    });\n"
	    "})");
	duk_dup(ctx, idx);
	duk_call(ctx, 1);
	duk_pop(ctx);
}

static duk_ret_t my_getter(duk_context *ctx) {
	printf("my_getter called\n");
	duk_push_string(ctx, "fakeGetterValue");
	return 1;
}

static duk_ret_t my_setter(duk_context *ctx) {
	printf("my_setter called\n");
	return 0;
}

static void push_getter(duk_context *ctx) {
	duk_push_c_function(ctx, my_getter, 0 /*nargs*/);
	duk_push_string(ctx, "getter");
	duk_put_prop_string(ctx, -2, "name");
}

static void push_getter_lightfunc(duk_context *ctx) {
	duk_push_c_lightfunc(ctx, my_getter, 0 /*nargs*/, 0 /*length*/, 0 /*magic*/);
}

static void push_setter(duk_context *ctx) {
	duk_push_c_function(ctx, my_setter, 1 /*nargs*/);
	duk_push_string(ctx, "setter");
	duk_put_prop_string(ctx, -2, "name");
}

static void push_setter_lightfunc(duk_context *ctx) {
	duk_push_c_lightfunc(ctx, my_setter, 1 /*nargs*/, 1 /*length*/, 0 /*magic*/);
}

/* Define new property, value only.  Other attributes get defaults. */
static duk_ret_t test_value_only(duk_context *ctx) {
	duk_push_object(ctx);
	duk_push_string(ctx, "dummy");
	duk_push_string(ctx, "my_key");
	duk_push_int(ctx, 123);
	printf("top before: %ld\n", (long) duk_get_top(ctx));
	duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_VALUE);
	printf("top after: %ld\n", (long) duk_get_top(ctx));
	dump_object(ctx, -2);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Try to re-define a value for a non-configurable property. */
static duk_ret_t test_value_redefine(duk_context *ctx) {
	/* Define new property, value only.  Other attributes will have
	 * default values (false).
	 */

	duk_push_object(ctx);
	duk_push_string(ctx, "dummy");
	duk_push_string(ctx, "my_key");
	duk_push_int(ctx, 123);
	printf("top before: %ld\n", (long) duk_get_top(ctx));
	duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_VALUE);
	printf("top after: %ld\n", (long) duk_get_top(ctx));
	dump_object(ctx, -2);

	/* Attempt to redefine value with exact SameValue succeeds even
	 * when not configurable.  (This, like most other things in this
	 * test case, is standard Object.defineProperty() behavior.)
	 */

	duk_push_string(ctx, "my_key");
	duk_push_int(ctx, 123);
	printf("top before: %ld\n", (long) duk_get_top(ctx));
	duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_VALUE);
	printf("top after: %ld\n", (long) duk_get_top(ctx));
	dump_object(ctx, -2);

	/* Attempt to redefine value fails because the property is not
	 * configurable.  A TypeError gets thrown.
	 */

	duk_push_string(ctx, "my_key");
	duk_push_int(ctx, 321);
	printf("top before: %ld\n", (long) duk_get_top(ctx));
	duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_VALUE);
	printf("top after: %ld\n", (long) duk_get_top(ctx));
	dump_object(ctx, -2);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Define a property with a value and all attribute combinations. */
static duk_ret_t test_value_attr_combinations(duk_context *ctx) {
	int i;

	duk_push_object(ctx);

	for (i = 0; i < 8; i++) {
		duk_push_sprintf(ctx, "my_key_%d", i);
		duk_push_int(ctx, 123);
		printf("top before: %ld\n", (long) duk_get_top(ctx));
		duk_def_prop(ctx,
		             -3,
		             DUK_DEFPROP_HAVE_VALUE |
		             DUK_DEFPROP_HAVE_WRITABLE |
		             DUK_DEFPROP_HAVE_ENUMERABLE |
		             DUK_DEFPROP_HAVE_CONFIGURABLE |
		             (i & 1 ? DUK_DEFPROP_WRITABLE : 0) |
		             (i & 2 ? DUK_DEFPROP_ENUMERABLE : 0) |
		             (i & 4 ? DUK_DEFPROP_CONFIGURABLE : 0));
		printf("top after: %ld\n", (long) duk_get_top(ctx));
	}

	dump_object(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Test presence of value and attributes; exercises the "tri-state"
 * nature of attributes.
 */
static duk_ret_t test_value_attr_presence(duk_context *ctx) {
	duk_push_object(ctx);

	/* First set the property to have all attributes true. */

	duk_push_string(ctx, "my_key_1");
	duk_push_int(ctx, 123);
	duk_def_prop(ctx,
	             -3,
	             DUK_DEFPROP_HAVE_VALUE |
	             DUK_DEFPROP_HAVE_WRITABLE |
	             DUK_DEFPROP_HAVE_ENUMERABLE |
	             DUK_DEFPROP_HAVE_CONFIGURABLE |
	             DUK_DEFPROP_WRITABLE |
	             DUK_DEFPROP_ENUMERABLE |
	             DUK_DEFPROP_CONFIGURABLE);
	dump_object(ctx, -1);

	/* Then turn the attributes off one-by-one.  Configurable must
	 * be last to avoid TypeErrors.
	 */

	duk_push_string(ctx, "my_key_1");
	duk_def_prop(ctx, -2, DUK_DEFPROP_HAVE_WRITABLE);
	dump_object(ctx, -1);

	duk_push_string(ctx, "my_key_1");
	duk_def_prop(ctx, -2, DUK_DEFPROP_HAVE_ENUMERABLE);
	dump_object(ctx, -1);

	duk_push_string(ctx, "my_key_1");
	duk_def_prop(ctx, -2, DUK_DEFPROP_HAVE_CONFIGURABLE);
	dump_object(ctx, -1);

	/* Same test for another property, but start with attributes
	 * cleared.  However, configurable must be set for this to
	 * work so leave that out from the test.
	 */

	duk_push_string(ctx, "my_key_2");
	duk_push_int(ctx, 321);
	duk_def_prop(ctx,
	             -3,
	             DUK_DEFPROP_HAVE_VALUE |
	             DUK_DEFPROP_HAVE_WRITABLE |
	             DUK_DEFPROP_HAVE_ENUMERABLE |
	             DUK_DEFPROP_HAVE_CONFIGURABLE |
	             DUK_DEFPROP_CONFIGURABLE);
	dump_object(ctx, -1);

	/* Then turn the attributes on one-by-one. */

	duk_push_string(ctx, "my_key_2");
	duk_def_prop(ctx, -2, DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE);
	dump_object(ctx, -1);

	duk_push_string(ctx, "my_key_2");
	duk_def_prop(ctx, -2, DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_ENUMERABLE);
	dump_object(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Test property creation with setter, getter, or both.  Use a few attribute
 * combinations at the same time (including not present = default).
 */
static duk_ret_t test_setget_only(duk_context *ctx) {
	duk_push_object(ctx);

	/* Getter only. */

	duk_push_string(ctx, "my_key_1");
	push_getter(ctx);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);
	dump_object(ctx, -1);

	/* Setter only. */

	duk_push_string(ctx, "my_key_2");
	push_setter(ctx);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_SETTER |
	                      DUK_DEFPROP_HAVE_ENUMERABLE | 0 |
	                      DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE);
	dump_object(ctx, -1);

	/* Getter and setter. */

	duk_push_string(ctx, "my_key_3");
	push_getter(ctx);
	push_setter(ctx);
	duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER |
	                      DUK_DEFPROP_HAVE_SETTER |
	                      DUK_DEFPROP_WRITABLE |  /* Note: ignored, no "have writable" flag */
	                      /* enumerable defaults */
	                      DUK_DEFPROP_HAVE_CONFIGURABLE | 0);
	dump_object(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Test an invalid call where we "have" a value, a getter, and a setter.
 * This is an invalid property descriptor.
 */
static duk_ret_t test_value_and_setget(duk_context *ctx) {
	duk_push_object(ctx);

	duk_push_string(ctx, "my_key_1");
	duk_push_int(ctx, 123);  /* value */
	push_getter(ctx);
	push_setter(ctx);
	duk_def_prop(ctx, -5, DUK_DEFPROP_HAVE_VALUE |
	                      DUK_DEFPROP_HAVE_GETTER |
	                      DUK_DEFPROP_HAVE_SETTER);

	dump_object(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Test an invalid call where we "have" writable (implies plain property)
 * and setter (implies accessor property).
 */
static duk_ret_t test_writable_and_set(duk_context *ctx) {
	duk_push_object(ctx);

	duk_push_string(ctx, "my_key_1");
	push_setter(ctx);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_WRITABLE |
	                      DUK_DEFPROP_HAVE_SETTER);

	dump_object(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Test a valid call where setter and getter are undefined.  This causes
 * them to be removed from the property.
 */
static duk_ret_t test_setget_undefined(duk_context *ctx) {
	duk_push_object(ctx);

	/* First setup a setter and a getter. */

	duk_push_string(ctx, "my_key_1");
	push_getter(ctx);
	push_setter(ctx);
	duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE |
	                      DUK_DEFPROP_HAVE_GETTER |
	                      DUK_DEFPROP_HAVE_SETTER);

	dump_object(ctx, -1);

	/* Remove getter. */

	duk_push_string(ctx, "my_key_1");
	duk_push_undefined(ctx);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);

	dump_object(ctx, -1);

	/* Remove setter. */

	duk_push_string(ctx, "my_key_1");
	duk_push_undefined(ctx);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_SETTER);

	dump_object(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Test an invalid call where getter is a non-object (but not undefined). */
static duk_ret_t test_getter_nonobject(duk_context *ctx) {
	duk_push_object(ctx);

	/* First setup a setter and a getter. */

	duk_push_string(ctx, "my_key_1");
	push_getter(ctx);
	push_setter(ctx);
	duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE |
	                      DUK_DEFPROP_HAVE_GETTER |
	                      DUK_DEFPROP_HAVE_SETTER);

	dump_object(ctx, -1);

	/* Try to set getter to a non-object */

	duk_push_string(ctx, "my_key_1");
	duk_push_int(ctx, 987);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);

	dump_object(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Test an invalid call where setter is a non-object (but not undefined). */
static duk_ret_t test_setter_nonobject(duk_context *ctx) {
	duk_push_object(ctx);

	/* First setup a setter and a getter. */

	duk_push_string(ctx, "my_key_1");
	push_getter(ctx);
	push_setter(ctx);
	duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE |
	                      DUK_DEFPROP_HAVE_GETTER |
	                      DUK_DEFPROP_HAVE_SETTER);

	dump_object(ctx, -1);

	/* Try to set setter to a non-object */

	duk_push_string(ctx, "my_key_1");
	duk_push_int(ctx, 987);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_SETTER);

	dump_object(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Test an invalid call where getter is a non-callable object. */
static duk_ret_t test_getter_noncallable(duk_context *ctx) {
	duk_push_object(ctx);

	duk_push_string(ctx, "my_key_1");
	duk_push_object(ctx);  /* getter */
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE |
	                      DUK_DEFPROP_HAVE_GETTER);

	dump_object(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Test an invalid call where setter is a non-callable object. */
static duk_ret_t test_setter_noncallable(duk_context *ctx) {
	duk_push_object(ctx);

	duk_push_string(ctx, "my_key_1");
	duk_push_object(ctx);  /* setter */
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE |
	                      DUK_DEFPROP_HAVE_SETTER);

	dump_object(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Test that lightfuncs work as setter and getter.  They get coerced to
 * a full function in the process though.
 */
static duk_ret_t test_setget_lightfunc(duk_context *ctx) {
	duk_push_object(ctx);

	duk_push_string(ctx, "my_key_1");
	push_getter_lightfunc(ctx);
	push_setter_lightfunc(ctx);
	duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE |
	                      DUK_DEFPROP_HAVE_GETTER |
	                      DUK_DEFPROP_HAVE_SETTER);

	dump_object(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Force addition of a new property into a non-extensible object. */
static duk_ret_t test_force_nonextensible_raw(duk_context *ctx, duk_bool_t forced) {
	duk_eval_string(ctx,
	                "(function () {\n"
	                "    var obj = {};\n"
	                "    Object.preventExtensions(obj);\n"
	                "    return obj;\n"
	                "})()\n");

	dump_object(ctx, 0);

	duk_push_string(ctx, "my_key_1");
	duk_push_int(ctx, 321);
	duk_def_prop(ctx, 0, DUK_DEFPROP_HAVE_VALUE |
	                     (forced ? DUK_DEFPROP_FORCE : 0));

	dump_object(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_fail_nonextensible(duk_context *ctx) {
	return test_force_nonextensible_raw(ctx, 0);
}
static duk_ret_t test_force_nonextensible(duk_context *ctx) {
	return test_force_nonextensible_raw(ctx, 1);
}

/* Force a non-configurable property configurable. */
static duk_ret_t test_force_set_configurable_raw(duk_context *ctx, duk_bool_t forced) {
	duk_eval_string(ctx,
	                "(function () {\n"
	                "    var obj = {};\n"
	                "    Object.defineProperty(obj, 'my_key_1', { value: 123, writable: false, enumerable: true, configurable: false });\n"
	                "    return obj;\n"
	                "})()\n");

	dump_object(ctx, 0);

	duk_push_string(ctx, "my_key_1");
	duk_def_prop(ctx, 0, DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE |
	                     (forced ? DUK_DEFPROP_FORCE : 0));

	dump_object(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_fail_set_configurable(duk_context *ctx) {
	return test_force_set_configurable_raw(ctx, 0);
}
static duk_ret_t test_force_set_configurable(duk_context *ctx) {
	return test_force_set_configurable_raw(ctx, 1);
}

/* Force a non-configurable property enumerable. */
static duk_ret_t test_force_set_enumerable_raw(duk_context *ctx, duk_bool_t forced) {
	duk_eval_string(ctx,
	                "(function () {\n"
	                "    var obj = {};\n"
	                "    Object.defineProperty(obj, 'my_key_1', { value: 123, writable: false, enumerable: false, configurable: false });\n"
	                "    return obj;\n"
	                "})()\n");

	dump_object(ctx, 0);

	duk_push_string(ctx, "my_key_1");
	duk_def_prop(ctx, 0, DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_ENUMERABLE |
	                     (forced ? DUK_DEFPROP_FORCE : 0));

	dump_object(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_fail_set_enumerable(duk_context *ctx) {
	return test_force_set_enumerable_raw(ctx, 0);
}
static duk_ret_t test_force_set_enumerable(duk_context *ctx) {
	return test_force_set_enumerable_raw(ctx, 1);
}

/* Force a non-configurable property writable. */
static duk_ret_t test_force_set_writable_raw(duk_context *ctx, duk_bool_t forced) {
	duk_eval_string(ctx,
	                "(function () {\n"
	                "    var obj = {};\n"
	                "    Object.defineProperty(obj, 'my_key_1', { value: 123, writable: false, enumerable: false, configurable: false });\n"
	                "    return obj;\n"
	                "})()\n");

	dump_object(ctx, 0);

	duk_push_string(ctx, "my_key_1");
	duk_def_prop(ctx, 0, DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE |
	                     (forced ? DUK_DEFPROP_FORCE : 0));

	dump_object(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_fail_set_writable(duk_context *ctx) {
	return test_force_set_writable_raw(ctx, 0);
}
static duk_ret_t test_force_set_writable(duk_context *ctx) {
	return test_force_set_writable_raw(ctx, 1);
}

/* Force value change for a non-configurable property. */
static duk_ret_t test_force_set_value_raw(duk_context *ctx, duk_bool_t forced) {
	duk_eval_string(ctx,
	                "(function () {\n"
	                "    var obj = {};\n"
	                "    Object.defineProperty(obj, 'my_key_1', { value: 123, writable: false, enumerable: false, configurable: false });\n"
	                "    return obj;\n"
	                "})()\n");

	dump_object(ctx, 0);

	duk_push_string(ctx, "my_key_1");
	duk_push_int(ctx, 321);
	duk_def_prop(ctx, 0, DUK_DEFPROP_HAVE_VALUE |
	                     (forced ? DUK_DEFPROP_FORCE : 0));

	dump_object(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_fail_set_value(duk_context *ctx) {
	return test_force_set_value_raw(ctx, 0);
}
static duk_ret_t test_force_set_value(duk_context *ctx) {
	return test_force_set_value_raw(ctx, 1);
}

/* Force setter/getter change for a non-configurable property. */
static duk_ret_t test_force_set_setget_raw(duk_context *ctx, duk_bool_t setter, duk_bool_t forced) {
	duk_eval_string(ctx,
	                "(function () {\n"
	                "    var obj = {};\n"
	                "    Object.defineProperty(obj, 'my_key_1', { set: function () {}, get: function () {}, enumerable: false, configurable: false });\n"
	                "    return obj;\n"
	                "})()\n");

	dump_object(ctx, 0);

	duk_push_string(ctx, "my_key_1");
	if (setter) {
		push_setter(ctx);
	} else {
		push_getter(ctx);
	}
	duk_def_prop(ctx, 0, (setter ? DUK_DEFPROP_HAVE_SETTER : DUK_DEFPROP_HAVE_GETTER) |
	                     (forced ? DUK_DEFPROP_FORCE : 0));

	dump_object(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_fail_set_getter(duk_context *ctx) {
	return test_force_set_setget_raw(ctx, 0, 0);
}
static duk_ret_t test_force_set_getter(duk_context *ctx) {
	return test_force_set_setget_raw(ctx, 0, 1);
}
static duk_ret_t test_fail_set_setter(duk_context *ctx) {
	return test_force_set_setget_raw(ctx, 1, 0);
}
static duk_ret_t test_force_set_setter(duk_context *ctx) {
	return test_force_set_setget_raw(ctx, 1, 1);
}

/* Force change from data to accessor property for non-configurable property.
 * Also set the new value enumerable (to cover more cases).
 */
static duk_ret_t test_force_data2accessor_raw(duk_context *ctx, duk_bool_t forced) {
	duk_eval_string(ctx,
	                "(function () {\n"
	                "    var obj = {};\n"
	                "    Object.defineProperty(obj, 'my_key_1', { value: 123, writable: false, enumerable: false, configurable: false });\n"
	                "    return obj;\n"
	                "})()\n");

	dump_object(ctx, 0);

	duk_push_string(ctx, "my_key_1");
	push_getter(ctx);
	push_setter(ctx);
	duk_def_prop(ctx, 0, DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_ENUMERABLE |
	                     DUK_DEFPROP_HAVE_GETTER |
	                     DUK_DEFPROP_HAVE_SETTER |
	                     (forced ? DUK_DEFPROP_FORCE : 0));

	dump_object(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_fail_data2accessor(duk_context *ctx) {
	return test_force_data2accessor_raw(ctx, 0);
}
static duk_ret_t test_force_data2accessor(duk_context *ctx) {
	return test_force_data2accessor_raw(ctx, 1);
}

/* Force change from accessor to data property for non-configurable property.
 * Also set the new value writable (to cover more cases).
 */
static duk_ret_t test_force_accessor2data_raw(duk_context *ctx, duk_bool_t forced) {
	duk_eval_string(ctx,
	                "(function () {\n"
	                "    var obj = {};\n"
	                "    Object.defineProperty(obj, 'my_key_1', { set: function () {}, get: function () {}, enumerable: false, configurable: false });\n"
	                "    return obj;\n"
	                "})()\n");

	dump_object(ctx, 0);

	duk_push_string(ctx, "my_key_1");
	duk_push_int(ctx, 321);
	duk_def_prop(ctx, 0, DUK_DEFPROP_HAVE_VALUE |
	                     DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE |
	                     (forced ? DUK_DEFPROP_FORCE : 0));

	dump_object(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_fail_accessor2data(duk_context *ctx) {
	return test_force_accessor2data_raw(ctx, 0);
}
static duk_ret_t test_force_accessor2data(duk_context *ctx) {
	return test_force_accessor2data_raw(ctx, 1);
}

/* Make array smaller, ignoring non-configurable elements. */
static duk_ret_t test_force_array_smaller_raw(duk_context *ctx, duk_bool_t length_writable, duk_bool_t forced) {
	if (length_writable) {
		duk_eval_string(ctx,
		                "(function () {\n"
		                "    var obj = [ 'foo', 'bar', 'quux' /*non-configurable*/, 'baz' ];\n"
		                "    Object.defineProperty(obj, '2', { configurable: false });\n"
		                "    return obj;\n"
		                "})()\n");
	} else {
		duk_eval_string(ctx,
		                "(function () {\n"
		                "    var obj = [ 'foo', 'bar', 'quux' /*non-configurable*/, 'baz' ];\n"
		                "    Object.defineProperty(obj, '2', { configurable: false });\n"
		                "    Object.defineProperty(obj, 'length', { writable: false, configurable: false });\n"
		                "    return obj;\n"
		                "})()\n");
	}

	dump_object(ctx, 0);

	duk_push_string(ctx, "length");
	duk_push_int(ctx, 1);
	duk_def_prop(ctx, 0, DUK_DEFPROP_HAVE_VALUE |
	                     (forced ? DUK_DEFPROP_FORCE : 0));

	dump_object(ctx, 0);

	duk_json_encode(ctx, 0);
	printf("json: %s\n", duk_get_string(ctx, 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_fail_array_smaller(duk_context *ctx) {
	return test_force_array_smaller_raw(ctx, 1, 0);
}
static duk_ret_t test_force_array_smaller(duk_context *ctx) {
	return test_force_array_smaller_raw(ctx, 1, 1);
}
static duk_ret_t test_fail_array_smaller_nonwritablelength(duk_context *ctx) {
	return test_force_array_smaller_raw(ctx, 0, 0);
}
static duk_ret_t test_force_array_smaller_nonwritablelength(duk_context *ctx) {
	return test_force_array_smaller_raw(ctx, 0, 1);
}

/* Delete a non-deletable property in two steps: first use duk_def_prop()
 * to make it configurable by force, and then delete it normally.  Ideally
 * duk_del_prop() would provide a forced variant so this could be done in
 * one step.
 */
static duk_ret_t test_force_nondeletable_raw(duk_context *ctx, duk_bool_t forced) {
	duk_eval_string(ctx,
	                "(function () {\n"
	                "    var obj = {};\n"
	                "    Object.defineProperty(obj, 'my_key_1', { value: 123, writable: false, enumerable: false, configurable: false });\n"
	                "    return obj;\n"
	                "})()\n");

	dump_object(ctx, 0);
	printf("my_key_1 present: %d\n", (int) duk_has_prop_string(ctx, -1, "my_key_1"));

	if (forced) {
		duk_push_string(ctx, "my_key_1");
		duk_def_prop(ctx, 0, DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE |
		                     DUK_DEFPROP_FORCE);
	} else {
		/* Keep non-configurable for delete. */
	}

	duk_del_prop_string(ctx, -1, "my_key_1");

	dump_object(ctx, 0);
	printf("my_key_1 present: %d\n", (int) duk_has_prop_string(ctx, -1, "my_key_1"));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_fail_nondeletable(duk_context *ctx) {
	return test_force_nondeletable_raw(ctx, 0);
}
static duk_ret_t test_force_nondeletable(duk_context *ctx) {
	return test_force_nondeletable_raw(ctx, 1);
}

void test(duk_context *ctx) {
	/* Behaviors matching Object.defineProperty() */
	TEST_SAFE_CALL(test_value_only);
	TEST_SAFE_CALL(test_value_redefine);
	TEST_SAFE_CALL(test_value_attr_combinations);
	TEST_SAFE_CALL(test_value_attr_presence);
	TEST_SAFE_CALL(test_setget_only);
	TEST_SAFE_CALL(test_value_and_setget);
	TEST_SAFE_CALL(test_writable_and_set);
	TEST_SAFE_CALL(test_setget_undefined);
	TEST_SAFE_CALL(test_getter_nonobject);
	TEST_SAFE_CALL(test_setter_nonobject);
	TEST_SAFE_CALL(test_getter_noncallable);
	TEST_SAFE_CALL(test_setter_noncallable);
	TEST_SAFE_CALL(test_setget_lightfunc);

	/* Forced changes: ignore non-extensibility and non-configurability
	 * whenever possible and make changes anyway.  Not all operations can
	 * still be forced, e.g. virtual properties cannot be changed.
	 */
	TEST_SAFE_CALL(test_fail_nonextensible);
	TEST_SAFE_CALL(test_force_nonextensible);
	TEST_SAFE_CALL(test_fail_set_configurable);
	TEST_SAFE_CALL(test_force_set_configurable);
	TEST_SAFE_CALL(test_fail_set_enumerable);
	TEST_SAFE_CALL(test_force_set_enumerable);
	TEST_SAFE_CALL(test_fail_set_writable);
	TEST_SAFE_CALL(test_force_set_writable);
	TEST_SAFE_CALL(test_fail_set_value);
	TEST_SAFE_CALL(test_force_set_value);
	TEST_SAFE_CALL(test_fail_set_getter);
	TEST_SAFE_CALL(test_force_set_getter);
	TEST_SAFE_CALL(test_fail_set_setter);
	TEST_SAFE_CALL(test_force_set_setter);
	TEST_SAFE_CALL(test_fail_data2accessor);
	TEST_SAFE_CALL(test_force_data2accessor);
	TEST_SAFE_CALL(test_fail_accessor2data);
	TEST_SAFE_CALL(test_force_accessor2data);
	TEST_SAFE_CALL(test_fail_array_smaller);
	TEST_SAFE_CALL(test_force_array_smaller);
	TEST_SAFE_CALL(test_fail_array_smaller_nonwritablelength);
	TEST_SAFE_CALL(test_force_array_smaller_nonwritablelength);
	TEST_SAFE_CALL(test_fail_nondeletable);
	TEST_SAFE_CALL(test_force_nondeletable);
}
