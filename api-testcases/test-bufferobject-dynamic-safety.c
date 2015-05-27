/*
 *  Test that duk_hbufferobject is memory safe when the underlying dynamic
 *  buffer is a dynamic buffer that is resized after creation.  In particular,
 *  the testcase attempts to cover memory safety issues related to an
 *  underlying buffer not covering the logical duk_hbufferobject view.
 *
 *  The exact results of "uncovered" operations are not very important and
 *  not always intuitive: an uncovered buffer is not considered normal buffer
 *  usage but its memory safety matters.
 */

/*---
{
    "endianess": "little"
}
---*/

/*
 *  Helpers
 */

static void setup_duktape_buffer(duk_context *ctx, duk_idx_t idx) {
	idx = duk_require_normalize_index(ctx, idx);
	duk_eval_string(ctx,
		"(function (plain_buffer) {\n"
		"    return new Duktape.Buffer(plain_buffer);\n"
		"})\n");
	duk_dup(ctx, idx);
	duk_call(ctx, 1);
}

static void setup_nodejs_buffer(duk_context *ctx, duk_idx_t idx) {
	idx = duk_require_normalize_index(ctx, idx);
	duk_eval_string(ctx,
		"(function (plain_buffer) {\n"
		"    return new Buffer(plain_buffer);\n"
		"})\n");
	duk_dup(ctx, idx);
	duk_call(ctx, 1);
}

static void setup_nodejs_buffer_slice(duk_context *ctx, duk_idx_t idx, duk_int_t start, duk_int_t end) {
	idx = duk_require_normalize_index(ctx, idx);
	duk_eval_string(ctx,
		"(function (plain_buffer, start, end) {\n"
		"    return Buffer(plain_buffer).slice(start, end);\n"
		"})\n");
	duk_dup(ctx, idx);
	duk_push_int(ctx, start);
	duk_push_int(ctx, end);
	duk_call(ctx, 3);
}

static void setup_arraybuffer(duk_context *ctx, duk_idx_t idx) {
	idx = duk_require_normalize_index(ctx, idx);
	duk_eval_string(ctx,
		"(function (plain_buffer) {\n"
		"    return new ArrayBuffer(plain_buffer);\n"
		"})\n");
	duk_dup(ctx, idx);
	duk_call(ctx, 1);
}

static void setup_typedarray(duk_context *ctx, duk_idx_t idx, const char *name) {
	idx = duk_require_normalize_index(ctx, idx);
	duk_push_sprintf(ctx,
		"(function (plain_buffer) {\n"
		"    return new %s(new ArrayBuffer(plain_buffer));\n"
		"})\n", name);
	duk_eval(ctx);
	duk_dup(ctx, idx);
	duk_call(ctx, 1);
}

static void setup_typedarray_slice(duk_context *ctx, duk_idx_t idx, const char *name, duk_int_t start, duk_int_t end) {
	idx = duk_require_normalize_index(ctx, idx);
	duk_push_sprintf(ctx,
		"(function (plain_buffer, start, length) {\n"
		"    return new %s(new ArrayBuffer(plain_buffer), start, length);\n"
		"})\n", name);
	duk_eval(ctx);
	duk_dup(ctx, idx);
	duk_push_int(ctx, start);
	duk_push_int(ctx, end);
	duk_call(ctx, 3);
}

/*
 *  Indexed writes
 */

/*===
*** test_duktape_buffer_indexed_1a (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
*** test_nodejs_buffer_indexed_1a (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
*** test_nodejs_buffer_indexed_1b (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
*** test_arraybuffer_indexed_1a (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
*** test_uint8array_indexed_1a (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
*** test_uint8array_indexed_1b (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
*** test_uint16array_indexed_1a (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
*** test_uint16array_indexed_1b (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
*** test_uint32array_indexed_1a (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
*** test_uint32array_indexed_1b (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
*** test_float32array_indexed_1a (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
*** test_float32array_indexed_1b (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
*** test_float64array_indexed_1a (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
*** test_float64array_indexed_1b (duk_safe_call)
length 100
1
2
3
0 1
99 2
100 3
length 100
10
11
12
13
14
0 10
94 11
95 0
99 0
100 14
final top: 2
==> rc=0, result='undefined'
===*/

static void shared_read_write_index(duk_context *ctx, duk_size_t resize_to) {
	/* [ plain_buffer buffer ] */

	duk_eval_string(ctx,
		"(function (buf) {\n"
		"    print('length', buf.length);\n"
		"    print(buf[0] = 1);\n"
		"    print(buf[99] = 2);\n"
		"    print(buf[100] = 3);\n"   /* outside 'limit', not virtual -> becomes concrete property */
		"    print(0, buf[0]);\n"      /* in buffer */
		"    print(99, buf[99]);\n"    /* in buffer */
		"    print(100, buf[100]);\n"  /* outside duk_hbufferobject 'limit', not virtual */
		"})\n");
	duk_dup(ctx, 1);
	duk_call(ctx, 1);
	duk_pop(ctx);

	/* Resize to fewer number of bytes. */

	duk_resize_buffer(ctx, 0, resize_to);

	/* Reads inside the duk_hbufferobject limit but outside the underlying
	 * buffer return zero; writes are ignored.
	 */

	duk_eval_string(ctx,
		"(function (buf) {\n"
		"    print('length', buf.length);\n"
		"    print(buf[0] = 10);\n"
		"    print(buf[94] = 11);\n"
		"    print(buf[95] = 12);\n"
		"    print(buf[99] = 13);\n"
		"    print(buf[100] = 14);\n"
		"    print(0, buf[0]);\n"     /* in buffer */
		"    print(94, buf[94]);\n"   /* in buffer */
		"    print(95, buf[95]);\n"   /* inside duk_hbufferobject 'limit', but outside buffer */
		"    print(99, buf[99]);\n"   /* inside duk_hbufferobject 'limit', but outside buffer */
		"    print(100, buf[100]);\n"  /* outside duk_hbufferobject 'limit', not virtual */
		"})\n");
	duk_dup(ctx, 1);
	duk_call(ctx, 1);
	duk_pop(ctx);
}

/* Duktape.Buffer */
static duk_ret_t test_duktape_buffer_indexed_1a(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 100);
	setup_duktape_buffer(ctx, -1);
	shared_read_write_index(ctx, 95);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Node.js Buffer, "full slice" */
static duk_ret_t test_nodejs_buffer_indexed_1a(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 100);
	setup_nodejs_buffer(ctx, -1);
	shared_read_write_index(ctx, 95);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Node.js Buffer, "partial slice" */
static duk_ret_t test_nodejs_buffer_indexed_1b(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 150);
	setup_nodejs_buffer_slice(ctx, -1, 30, 130);
	shared_read_write_index(ctx, 30 + 95);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* ArrayBuffer, "full slice" */
static duk_ret_t test_arraybuffer_indexed_1a(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 100);
	setup_arraybuffer(ctx, -1);
	shared_read_write_index(ctx, 95);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Can't create slices for ArrayBuffer directly. */

/* Uint8Array, "full slice" */
static duk_ret_t test_uint8array_indexed_1a(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 100);
	setup_typedarray(ctx, -1, "Uint8Array");
	shared_read_write_index(ctx, 95);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Uint8Array, "partial slice" */
static duk_ret_t test_uint8array_indexed_1b(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 150);
	setup_typedarray_slice(ctx, -1, "Uint8Array", 30, 100);
	shared_read_write_index(ctx, 30 + 95);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Uint16Array, "full slice" */
static duk_ret_t test_uint16array_indexed_1a(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 200);
	setup_typedarray(ctx, -1, "Uint16Array");
	shared_read_write_index(ctx, 95 * 2);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Uint16Array, "partial slice" */
static duk_ret_t test_uint16array_indexed_1b(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 300);
	setup_typedarray_slice(ctx, -1, "Uint16Array", 60, 100);
	shared_read_write_index(ctx, 60 + 95 * 2);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Uint32Array, "full slice" */
static duk_ret_t test_uint32array_indexed_1a(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 400);
	setup_typedarray(ctx, -1, "Uint32Array");
	shared_read_write_index(ctx, 95 * 4);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Uint32Array, "partial slice" */
static duk_ret_t test_uint32array_indexed_1b(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 600);
	setup_typedarray_slice(ctx, -1, "Uint32Array", 120, 100);
	shared_read_write_index(ctx, 120 + 95 * 4);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Float32Array, "full slice" */
static duk_ret_t test_float32array_indexed_1a(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 400);
	setup_typedarray(ctx, -1, "Float32Array");
	shared_read_write_index(ctx, 95 * 4);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Float32Array, "partial slice" */
static duk_ret_t test_float32array_indexed_1b(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 600);
	setup_typedarray_slice(ctx, -1, "Float32Array", 120, 100);
	shared_read_write_index(ctx, 120 + 95 * 4);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Float64Array, "full slice" */
static duk_ret_t test_float64array_indexed_1a(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 800);
	setup_typedarray(ctx, -1, "Float64Array");
	shared_read_write_index(ctx, 95 * 8);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Float64Array, "partial slice" */
static duk_ret_t test_float64array_indexed_1b(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 1200);
	setup_typedarray_slice(ctx, -1, "Float64Array", 240, 100);
	shared_read_write_index(ctx, 240 + 95 * 8);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_json_serialize_1 (duk_safe_call)
resize to 20
[null,{"type":"Buffer","data":[64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83]},{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[|404142434445464748494a4b4c4d4e4f50515253|,{type:"Buffer",data:[64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83]},{type:"Buffer",data:[67,68]},|404142434445464748494a4b4c4d4e4f50515253|,|404142434445464748494a4b4c4d4e4f50515253|,|424344454647|,|404142434445464748494a4b4c4d4e4f50515253|,|44454647|]
resize to 19
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,|424344454647|,null,|44454647|]
resize to 18
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,|424344454647|,null,|44454647|]
resize to 17
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,|424344454647|,null,|44454647|]
resize to 16
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,|424344454647|,null,|44454647|]
resize to 15
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,|424344454647|,null,|44454647|]
resize to 14
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,|424344454647|,null,|44454647|]
resize to 13
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,|424344454647|,null,|44454647|]
resize to 12
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,|424344454647|,null,|44454647|]
resize to 11
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,|424344454647|,null,|44454647|]
resize to 10
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,|424344454647|,null,|44454647|]
resize to 9
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,|424344454647|,null,|44454647|]
resize to 8
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,|424344454647|,null,|44454647|]
resize to 7
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,null,null,null]
resize to 6
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,null,null,null]
resize to 5
[null,null,{"type":"Buffer","data":[67,68]},null,null,null,null,null]
[null,null,{type:"Buffer",data:[67,68]},null,null,null,null,null]
resize to 4
[null,null,null,null,null,null,null,null]
[null,null,null,null,null,null,null,null]
resize to 3
[null,null,null,null,null,null,null,null]
[null,null,null,null,null,null,null,null]
resize to 2
[null,null,null,null,null,null,null,null]
[null,null,null,null,null,null,null,null]
resize to 1
[null,null,null,null,null,null,null,null]
[null,null,null,null,null,null,null,null]
resize to 0
[null,null,null,null,null,null,null,null]
[null,null,null,null,null,null,null,null]
final top: 2
==> rc=0, result='undefined'
===*/

static duk_ret_t test_json_serialize_1(duk_context *ctx) {
	unsigned char *data;
	int i;
	duk_uarridx_t arridx = 0;

	data = (unsigned char *) duk_push_dynamic_buffer(ctx, 20);
	for (i = 0; i < 20; i++) {
		data[i] = 0x40 + i;
	}

	duk_push_array(ctx);  /* index 1 */
	setup_duktape_buffer(ctx, 0);
	duk_put_prop_index(ctx, 1, arridx++);
	setup_nodejs_buffer(ctx, 0);
	duk_put_prop_index(ctx, 1, arridx++);
	setup_nodejs_buffer_slice(ctx, 0, 3, 5);
	duk_put_prop_index(ctx, 1, arridx++);
	setup_arraybuffer(ctx, 0);
	duk_put_prop_index(ctx, 1, arridx++);
	setup_typedarray(ctx, 0, "Uint8Array");
	duk_put_prop_index(ctx, 1, arridx++);
	setup_typedarray_slice(ctx, 0, "Uint8Array", 2, 6);
	duk_put_prop_index(ctx, 1, arridx++);
	setup_typedarray(ctx, 0, "Uint32Array");
	duk_put_prop_index(ctx, 1, arridx++);
	setup_typedarray_slice(ctx, 0, "Uint32Array", 4, 1);
	duk_put_prop_index(ctx, 1, arridx++);

	/* Serialize with a full backing buffer first. */

	for (i = 20; i >= 0; i--) {
		printf("resize to %d\n", i);
		duk_resize_buffer(ctx, 0, i);

		duk_dup(ctx, 1);
		duk_json_encode(ctx, -1);
		printf("%s\n", duk_to_string(ctx, -1));
		duk_pop(ctx);
		duk_eval_string(ctx, "(function (v) { print(Duktape.enc('jx', v)); })");
		duk_dup(ctx, 1);
		duk_call(ctx, 1);
		duk_pop(ctx);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_typedarray_constructor_copy_1 (duk_safe_call)
|40414243444546474849|
|4000410042004300440045004600470048004900|
|40414243444546474800|
|4000410042004300440045004600470048000000|
|40414243444546470000|
|4000410042004300440045004600470000000000|
|40414243444546000000|
|4000410042004300440045004600000000000000|
|40414243444500000000|
|4000410042004300440045000000000000000000|
|40414243440000000000|
|4000410042004300440000000000000000000000|
|40414243000000000000|
|4000410042004300000000000000000000000000|
|40414200000000000000|
|4000410042000000000000000000000000000000|
|40410000000000000000|
|4000410000000000000000000000000000000000|
|40000000000000000000|
|4000000000000000000000000000000000000000|
|00000000000000000000|
|0000000000000000000000000000000000000000|
final top: 2
==> rc=0, result='undefined'
===*/

static duk_ret_t test_typedarray_constructor_copy_1(duk_context *ctx) {
	int i;
	unsigned char *data;

	data = (unsigned char *) duk_push_dynamic_buffer(ctx, 10);  /* index 0 */
	for (i = 0; i < 10; i++) {
		data[i] = 0x40 + i;
	}

	setup_typedarray(ctx, 0, "Uint8Array");  /* index 1 */

	for (i = 10; i >= 0; i--) {
		duk_resize_buffer(ctx, 0, i);

		/* Compatible copy, uses byte copy when buffer is covered. */
		duk_eval_string(ctx,
			"(function (src) {\n"
			"    var buf = new Uint8Array(src);\n"
			"    print(Duktape.enc('jx', buf));\n"
			"})\n");
		duk_dup(ctx, 1);
		duk_call(ctx, 1);
		duk_pop(ctx);

		/* Incompatible copy, uses "fast copy" when buffer is covered.
		 * Endian specific output.
		 */
		duk_eval_string(ctx,
			"(function (src) {\n"
			"    var buf = new Uint16Array(src);\n"
			"    print(Duktape.enc('jx', buf));\n"
			"})\n");
		duk_dup(ctx, 1);
		duk_call(ctx, 1);
		duk_pop(ctx);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_typedarray_set_1 (duk_safe_call)
10 1 1
10 1 2
10 1 3
10 1 4
10 1 5
10 2 1
RangeError
10 2 2
10 2 3
10 2 4
10 2 5
10 3 1
RangeError
10 3 2
10 3 3
10 3 4
10 3 5
10 4 1
RangeError
10 4 2
RangeError
10 4 3
RangeError
10 4 4
10 4 5
RangeError
9 1 1
9 1 2
9 1 3
9 1 4
9 1 5
9 2 1
RangeError
9 2 2
9 2 3
9 2 4
9 2 5
9 3 1
RangeError
9 3 2
9 3 3
9 3 4
9 3 5
9 4 1
RangeError
9 4 2
RangeError
9 4 3
RangeError
9 4 4
9 4 5
RangeError
8 1 1
8 1 2
8 1 3
8 1 4
8 1 5
8 2 1
RangeError
8 2 2
8 2 3
8 2 4
8 2 5
8 3 1
RangeError
8 3 2
8 3 3
8 3 4
8 3 5
8 4 1
RangeError
8 4 2
RangeError
8 4 3
RangeError
8 4 4
8 4 5
RangeError
7 1 1
7 1 2
7 1 3
7 1 4
7 1 5
7 2 1
RangeError
7 2 2
7 2 3
7 2 4
7 2 5
7 3 1
RangeError
7 3 2
7 3 3
7 3 4
7 3 5
7 4 1
RangeError
7 4 2
RangeError
7 4 3
RangeError
7 4 4
7 4 5
RangeError
6 1 1
6 1 2
6 1 3
6 1 4
6 1 5
6 2 1
RangeError
6 2 2
6 2 3
6 2 4
6 2 5
6 3 1
RangeError
6 3 2
6 3 3
6 3 4
6 3 5
6 4 1
RangeError
6 4 2
RangeError
6 4 3
RangeError
6 4 4
6 4 5
RangeError
5 1 1
5 1 2
5 1 3
5 1 4
5 1 5
5 2 1
RangeError
5 2 2
5 2 3
5 2 4
5 2 5
5 3 1
RangeError
5 3 2
5 3 3
5 3 4
5 3 5
5 4 1
RangeError
5 4 2
RangeError
5 4 3
RangeError
5 4 4
5 4 5
RangeError
4 1 1
4 1 2
4 1 3
4 1 4
4 1 5
4 2 1
RangeError
4 2 2
4 2 3
4 2 4
4 2 5
4 3 1
RangeError
4 3 2
4 3 3
4 3 4
4 3 5
4 4 1
RangeError
4 4 2
RangeError
4 4 3
RangeError
4 4 4
4 4 5
RangeError
3 1 1
3 1 2
3 1 3
3 1 4
3 1 5
3 2 1
RangeError
3 2 2
3 2 3
3 2 4
3 2 5
3 3 1
RangeError
3 3 2
3 3 3
3 3 4
3 3 5
3 4 1
RangeError
3 4 2
RangeError
3 4 3
RangeError
3 4 4
3 4 5
RangeError
2 1 1
2 1 2
2 1 3
2 1 4
2 1 5
2 2 1
RangeError
2 2 2
2 2 3
2 2 4
2 2 5
2 3 1
RangeError
2 3 2
2 3 3
2 3 4
2 3 5
2 4 1
RangeError
2 4 2
RangeError
2 4 3
RangeError
2 4 4
2 4 5
RangeError
1 1 1
1 1 2
1 1 3
1 1 4
1 1 5
1 2 1
RangeError
1 2 2
1 2 3
1 2 4
1 2 5
1 3 1
RangeError
1 3 2
1 3 3
1 3 4
1 3 5
1 4 1
RangeError
1 4 2
RangeError
1 4 3
RangeError
1 4 4
1 4 5
RangeError
0 1 1
0 1 2
0 1 3
0 1 4
0 1 5
0 2 1
RangeError
0 2 2
0 2 3
0 2 4
0 2 5
0 3 1
RangeError
0 3 2
0 3 3
0 3 4
0 3 5
0 4 1
RangeError
0 4 2
RangeError
0 4 3
RangeError
0 4 4
0 4 5
RangeError
final top: 6
==> rc=0, result='undefined'
===*/

static duk_ret_t test_typedarray_set_1(duk_context *ctx) {
	int i, dst, src;
	unsigned char *data;

	data = (unsigned char *) duk_push_dynamic_buffer(ctx, 10);  /* index 0 */
	for (i = 0; i < 10; i++) {
		data[i] = 0x40 + i;
	}

	/* Some combinations are not set() compatible and cause a RangeError
	 * (source longer than target).  But this set should exercise byte copy,
	 * fast copy, and slow copy code paths.
	 */

	setup_typedarray(ctx, 0, "Uint8Array");               /* index 1, length 10 */
	setup_typedarray_slice(ctx, 0, "Uint8Array", 2, 5);   /* index 2, length 5 */
	setup_typedarray(ctx, 0, "Uint16Array");              /* index 3, length 5 */
	setup_typedarray_slice(ctx, 0, "Uint16Array", 2, 3);  /* index 4, length 3 */
	duk_eval_string(ctx, "[ 1, 2, 3, 4, 5 ]");            /* index 5, length 5 */

	for (i = 10; i >= 0; i--) {
		duk_resize_buffer(ctx, 0, i);

		/* Various copy combinations.  Resulting buffer is omitted;
		 * when both dst and src are typedarrays they share the same
		 * underlying buffer. so the results are a bit confusing (and
		 * not important to memory safety).
		 */

		for (dst = 1; dst <= 4; dst++) {
			for (src = 1; src <= 5; src++) {
				printf("%d %d %d\n", i, dst, src);
				duk_eval_string(ctx,
					"(function (dst, src) {\n"
					"    for (var i = 0; i < dst.length; i++) { dst[i] = 0x11; }\n"
					"    try { dst.set(src); } catch (e) { print(e.name); }\n"
					"})\n");
				duk_dup(ctx, dst);
				duk_dup(ctx, src);
				duk_call(ctx, 2);
				duk_pop(ctx);
			}
		}
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_nodejs_buffer_compare_1 (duk_safe_call)
10 1 1
0 0
10 1 2
-1 -1
10 1 3
-1 -1
10 1 4
-1 -1
10 2 1
1 1
10 2 2
0 0
10 2 3
-1 -1
10 2 4
-1 -1
10 3 1
1 1
10 3 2
1 1
10 3 3
0 0
10 3 4
-1 -1
10 4 1
1 1
10 4 2
1 1
10 4 3
1 1
10 4 4
0 0
9 1 1
-1 -1
9 1 2
-1 -1
9 1 3
-1 -1
9 1 4
-1 -1
9 2 1
-1 -1
9 2 2
0 0
9 2 3
-1 -1
9 2 4
-1 -1
9 3 1
-1 -1
9 3 2
1 1
9 3 3
0 0
9 3 4
-1 -1
9 4 1
-1 -1
9 4 2
1 1
9 4 3
1 1
9 4 4
0 0
8 1 1
-1 -1
8 1 2
-1 -1
8 1 3
-1 -1
8 1 4
-1 -1
8 2 1
-1 -1
8 2 2
0 0
8 2 3
-1 -1
8 2 4
-1 -1
8 3 1
-1 -1
8 3 2
1 1
8 3 3
0 0
8 3 4
-1 -1
8 4 1
-1 -1
8 4 2
-1 -1
8 4 3
-1 -1
8 4 4
-1 -1
7 1 1
-1 -1
7 1 2
-1 -1
7 1 3
-1 -1
7 1 4
-1 -1
7 2 1
-1 -1
7 2 2
0 0
7 2 3
-1 -1
7 2 4
-1 -1
7 3 1
-1 -1
7 3 2
1 1
7 3 3
0 0
7 3 4
-1 -1
7 4 1
-1 -1
7 4 2
-1 -1
7 4 3
-1 -1
7 4 4
-1 -1
6 1 1
-1 -1
6 1 2
-1 -1
6 1 3
-1 -1
6 1 4
-1 -1
6 2 1
-1 -1
6 2 2
0 0
6 2 3
-1 -1
6 2 4
-1 -1
6 3 1
-1 -1
6 3 2
-1 -1
6 3 3
-1 -1
6 3 4
-1 -1
6 4 1
-1 -1
6 4 2
-1 -1
6 4 3
-1 -1
6 4 4
-1 -1
5 1 1
-1 -1
5 1 2
-1 -1
5 1 3
-1 -1
5 1 4
-1 -1
5 2 1
-1 -1
5 2 2
0 0
5 2 3
-1 -1
5 2 4
-1 -1
5 3 1
-1 -1
5 3 2
-1 -1
5 3 3
-1 -1
5 3 4
-1 -1
5 4 1
-1 -1
5 4 2
-1 -1
5 4 3
-1 -1
5 4 4
-1 -1
4 1 1
-1 -1
4 1 2
-1 -1
4 1 3
-1 -1
4 1 4
-1 -1
4 2 1
-1 -1
4 2 2
0 0
4 2 3
-1 -1
4 2 4
-1 -1
4 3 1
-1 -1
4 3 2
-1 -1
4 3 3
-1 -1
4 3 4
-1 -1
4 4 1
-1 -1
4 4 2
-1 -1
4 4 3
-1 -1
4 4 4
-1 -1
3 1 1
-1 -1
3 1 2
-1 -1
3 1 3
-1 -1
3 1 4
-1 -1
3 2 1
-1 -1
3 2 2
-1 -1
3 2 3
-1 -1
3 2 4
-1 -1
3 3 1
-1 -1
3 3 2
-1 -1
3 3 3
-1 -1
3 3 4
-1 -1
3 4 1
-1 -1
3 4 2
-1 -1
3 4 3
-1 -1
3 4 4
-1 -1
2 1 1
-1 -1
2 1 2
-1 -1
2 1 3
-1 -1
2 1 4
-1 -1
2 2 1
-1 -1
2 2 2
-1 -1
2 2 3
-1 -1
2 2 4
-1 -1
2 3 1
-1 -1
2 3 2
-1 -1
2 3 3
-1 -1
2 3 4
-1 -1
2 4 1
-1 -1
2 4 2
-1 -1
2 4 3
-1 -1
2 4 4
-1 -1
1 1 1
-1 -1
1 1 2
-1 -1
1 1 3
-1 -1
1 1 4
-1 -1
1 2 1
-1 -1
1 2 2
-1 -1
1 2 3
-1 -1
1 2 4
-1 -1
1 3 1
-1 -1
1 3 2
-1 -1
1 3 3
-1 -1
1 3 4
-1 -1
1 4 1
-1 -1
1 4 2
-1 -1
1 4 3
-1 -1
1 4 4
-1 -1
0 1 1
-1 -1
0 1 2
-1 -1
0 1 3
-1 -1
0 1 4
-1 -1
0 2 1
-1 -1
0 2 2
-1 -1
0 2 3
-1 -1
0 2 4
-1 -1
0 3 1
-1 -1
0 3 2
-1 -1
0 3 3
-1 -1
0 3 4
-1 -1
0 4 1
-1 -1
0 4 2
-1 -1
0 4 3
-1 -1
0 4 4
-1 -1
final top: 5
==> rc=0, result='undefined'
===*/

static int test_nodejs_buffer_compare_1(duk_context *ctx) {
	int i, dst, src;
	unsigned char *data;

	/* There are two relevant methods: Buffer.compare and Buffer.prototype.compare() */

	data = (unsigned char *) duk_push_dynamic_buffer(ctx, 10);  /* index 0 */
	for (i = 0; i < 10; i++) {
		data[i] = 0x40 + i;
	}

	setup_nodejs_buffer(ctx, 0);
	setup_nodejs_buffer_slice(ctx, 0, 1, 4);
	setup_nodejs_buffer_slice(ctx, 0, 3, 7);
	setup_nodejs_buffer_slice(ctx, 0, 6, 9);

	for (i = 10; i >= 0; i--) {
		duk_resize_buffer(ctx, 0, i);

		for (dst = 1; dst <= 4; dst++) {
			for (src = 1; src <= 4; src++) {
				printf("%d %d %d\n", i, dst, src);
				duk_eval_string(ctx,
					"(function (dst, src) {\n"
					"    print(Buffer.compare(dst, src), dst.compare(src));\n"
					"})");
				duk_dup(ctx, dst);
				duk_dup(ctx, src);
				duk_call(ctx, 2);
				duk_pop(ctx);
			}
		}
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_nodejs_buffer_write_1 (duk_safe_call)
10 1 5
10 1 6
10 1 7
10 1 8
10 2 5
10 2 6
10 2 7
10 2 8
10 3 5
10 3 6
10 3 7
10 3 8
10 4 5
10 4 6
10 4 7
10 4 8
9 1 5
9 1 6
9 1 7
9 1 8
9 2 5
9 2 6
9 2 7
9 2 8
9 3 5
9 3 6
9 3 7
9 3 8
9 4 5
9 4 6
9 4 7
9 4 8
8 1 5
8 1 6
8 1 7
8 1 8
8 2 5
8 2 6
8 2 7
8 2 8
8 3 5
8 3 6
8 3 7
8 3 8
8 4 5
8 4 6
8 4 7
8 4 8
7 1 5
7 1 6
7 1 7
7 1 8
7 2 5
7 2 6
7 2 7
7 2 8
7 3 5
7 3 6
7 3 7
7 3 8
7 4 5
7 4 6
7 4 7
7 4 8
6 1 5
6 1 6
6 1 7
6 1 8
6 2 5
6 2 6
6 2 7
6 2 8
6 3 5
6 3 6
6 3 7
6 3 8
6 4 5
6 4 6
6 4 7
6 4 8
5 1 5
5 1 6
5 1 7
5 1 8
5 2 5
5 2 6
5 2 7
5 2 8
5 3 5
5 3 6
5 3 7
5 3 8
5 4 5
5 4 6
5 4 7
5 4 8
4 1 5
4 1 6
4 1 7
4 1 8
4 2 5
4 2 6
4 2 7
4 2 8
4 3 5
4 3 6
4 3 7
4 3 8
4 4 5
4 4 6
4 4 7
4 4 8
3 1 5
3 1 6
3 1 7
3 1 8
3 2 5
3 2 6
3 2 7
3 2 8
3 3 5
3 3 6
3 3 7
3 3 8
3 4 5
3 4 6
3 4 7
3 4 8
2 1 5
2 1 6
2 1 7
2 1 8
2 2 5
2 2 6
2 2 7
2 2 8
2 3 5
2 3 6
2 3 7
2 3 8
2 4 5
2 4 6
2 4 7
2 4 8
1 1 5
1 1 6
1 1 7
1 1 8
1 2 5
1 2 6
1 2 7
1 2 8
1 3 5
1 3 6
1 3 7
1 3 8
1 4 5
1 4 6
1 4 7
1 4 8
0 1 5
0 1 6
0 1 7
0 1 8
0 2 5
0 2 6
0 2 7
0 2 8
0 3 5
0 3 6
0 3 7
0 3 8
0 4 5
0 4 6
0 4 7
0 4 8
final top: 9
==> rc=0, result='undefined'
===*/

static int test_nodejs_buffer_write_1(duk_context *ctx) {
	int i, dst, src;
	unsigned char *data;

	data = (unsigned char *) duk_push_dynamic_buffer(ctx, 10);  /* index 0 */
	for (i = 0; i < 10; i++) {
		data[i] = 0x40 + i;
	}

	setup_nodejs_buffer(ctx, 0);              /* index 1 */
	setup_nodejs_buffer_slice(ctx, 0, 1, 4);  /* index 2 */
	setup_nodejs_buffer_slice(ctx, 0, 3, 7);  /* index 3 */
	setup_nodejs_buffer_slice(ctx, 0, 6, 9);  /* index 4 */
	duk_push_string(ctx, "");                 /* index 5 */
	duk_push_string(ctx, "fo");               /* index 6 */
	duk_push_string(ctx, "bar");              /* index 7 */
	duk_push_string(ctx, "quux");             /* index 8 */

	for (i = 10; i >= 0; i--) {
		duk_resize_buffer(ctx, 0, i);

		/* The resulting buffers are not printed here; they're not very
		 * intuitive because both the source and the destination share
		 * the same underlying buffer.
		 */

		for (dst = 1; dst <= 4; dst++) {
			for (src = 5; src <= 8; src++) {
				printf("%d %d %d\n", i, dst, src);
				duk_eval_string(ctx,
					"(function (dst, src) {\n"
					"    for (var i = 0; i < dst.length; i++) { dst[i] = 0x11; }\n"
					"    dst.write(src);\n"
					"    for (var i = 0; i < dst.length; i++) { dst[i] = 0x11; }\n"
					"    dst.write(src, 1);\n"
					"})");
				duk_dup(ctx, dst);
				duk_dup(ctx, src);
				duk_call(ctx, 2);
				duk_pop(ctx);
			}
		}
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_nodejs_buffer_copy_1 (duk_safe_call)
10 1 1
10 1 2
10 1 3
10 1 4
10 2 1
10 2 2
10 2 3
10 2 4
10 3 1
10 3 2
10 3 3
10 3 4
10 4 1
10 4 2
10 4 3
10 4 4
9 1 1
9 1 2
9 1 3
9 1 4
9 2 1
9 2 2
9 2 3
9 2 4
9 3 1
9 3 2
9 3 3
9 3 4
9 4 1
9 4 2
9 4 3
9 4 4
8 1 1
8 1 2
8 1 3
8 1 4
8 2 1
8 2 2
8 2 3
8 2 4
8 3 1
8 3 2
8 3 3
8 3 4
8 4 1
8 4 2
8 4 3
8 4 4
7 1 1
7 1 2
7 1 3
7 1 4
7 2 1
7 2 2
7 2 3
7 2 4
7 3 1
7 3 2
7 3 3
7 3 4
7 4 1
7 4 2
7 4 3
7 4 4
6 1 1
6 1 2
6 1 3
6 1 4
6 2 1
6 2 2
6 2 3
6 2 4
6 3 1
6 3 2
6 3 3
6 3 4
6 4 1
6 4 2
6 4 3
6 4 4
5 1 1
5 1 2
5 1 3
5 1 4
5 2 1
5 2 2
5 2 3
5 2 4
5 3 1
5 3 2
5 3 3
5 3 4
5 4 1
5 4 2
5 4 3
5 4 4
4 1 1
4 1 2
4 1 3
4 1 4
4 2 1
4 2 2
4 2 3
4 2 4
4 3 1
4 3 2
4 3 3
4 3 4
4 4 1
4 4 2
4 4 3
4 4 4
3 1 1
3 1 2
3 1 3
3 1 4
3 2 1
3 2 2
3 2 3
3 2 4
3 3 1
3 3 2
3 3 3
3 3 4
3 4 1
3 4 2
3 4 3
3 4 4
2 1 1
2 1 2
2 1 3
2 1 4
2 2 1
2 2 2
2 2 3
2 2 4
2 3 1
2 3 2
2 3 3
2 3 4
2 4 1
2 4 2
2 4 3
2 4 4
1 1 1
1 1 2
1 1 3
1 1 4
1 2 1
1 2 2
1 2 3
1 2 4
1 3 1
1 3 2
1 3 3
1 3 4
1 4 1
1 4 2
1 4 3
1 4 4
0 1 1
0 1 2
0 1 3
0 1 4
0 2 1
0 2 2
0 2 3
0 2 4
0 3 1
0 3 2
0 3 3
0 3 4
0 4 1
0 4 2
0 4 3
0 4 4
final top: 5
==> rc=0, result='undefined'
===*/

static int test_nodejs_buffer_copy_1(duk_context *ctx) {
	int i, dst, src;
	unsigned char *data;

	data = (unsigned char *) duk_push_dynamic_buffer(ctx, 10);  /* index 0 */
	for (i = 0; i < 10; i++) {
		data[i] = 0x40 + i;
	}

	setup_nodejs_buffer(ctx, 0);
	setup_nodejs_buffer_slice(ctx, 0, 1, 4);
	setup_nodejs_buffer_slice(ctx, 0, 3, 7);
	setup_nodejs_buffer_slice(ctx, 0, 6, 9);

	for (i = 10; i >= 0; i--) {
		duk_resize_buffer(ctx, 0, i);

		/* The resulting buffers are not printed here; they're not very
		 * intuitive because both the source and the destination share
		 * the same underlying buffer.
		 */

		for (dst = 1; dst <= 4; dst++) {
			for (src = 1; src <= 4; src++) {
				printf("%d %d %d\n", i, dst, src);
				duk_eval_string(ctx,
					"(function (dst, src) {\n"
					"    for (var i = 0; i < dst.length; i++) { dst[i] = 0x11; }\n"
					"    for (var i = 0; i < src.length; i++) { src[i] = 0x22; }\n"
					"    src.copy(dst);\n"
					"    for (var i = 0; i < dst.length; i++) { dst[i] = 0x11; }\n"
					"    for (var i = 0; i < src.length; i++) { src[i] = 0x22; }\n"
					"    src.copy(dst, 4, 1);\n"
					"})");
				duk_dup(ctx, dst);
				duk_dup(ctx, src);
				duk_call(ctx, 2);
				duk_pop(ctx);
			}
		}
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_nodejs_buffer_concat_1 (duk_safe_call)
10
{type:"Buffer",data:[64,65,66,67,68,69,70,71,72,73,65,66,67,68,69,67,68,69,70,70,71,72,65,66,67,68,69,70,71]}
9
{type:"Buffer",data:[0,0,0,0,0,0,0,0,0,0,65,66,67,68,69,67,68,69,70,70,71,72,65,66,67,68,69,70,71]}
8
{type:"Buffer",data:[0,0,0,0,0,0,0,0,0,0,65,66,67,68,69,67,68,69,70,0,0,0,65,66,67,68,69,70,71]}
7
{type:"Buffer",data:[0,0,0,0,0,0,0,0,0,0,65,66,67,68,69,67,68,69,70,0,0,0,0,0,0,0,0,0,0]}
6
{type:"Buffer",data:[0,0,0,0,0,0,0,0,0,0,65,66,67,68,69,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}
5
{type:"Buffer",data:[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}
4
{type:"Buffer",data:[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}
3
{type:"Buffer",data:[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}
2
{type:"Buffer",data:[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}
1
{type:"Buffer",data:[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}
0
{type:"Buffer",data:[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}
final top: 6
==> rc=0, result='undefined'
===*/

static int test_nodejs_buffer_concat_1(duk_context *ctx) {
	int i;
	unsigned char *data;

	data = (unsigned char *) duk_push_dynamic_buffer(ctx, 10);  /* index 0 */
	for (i = 0; i < 10; i++) {
		data[i] = 0x40 + i;
	}

	setup_nodejs_buffer(ctx, 0);
	setup_nodejs_buffer_slice(ctx, 0, 1, 6);
	setup_nodejs_buffer_slice(ctx, 0, 3, 7);
	setup_nodejs_buffer_slice(ctx, 0, 6, 9);
	setup_nodejs_buffer_slice(ctx, 0, 1, 8);

	for (i = 10; i >= 0; i--) {
		duk_resize_buffer(ctx, 0, i);

		printf("%d\n", i);
		duk_eval_string(ctx,
			"(function (arg1, arg2, arg3, arg4, arg5) {\n"
			"    var res = Buffer.concat([ arg1, arg2, arg3, arg4, arg5 ]);\n"
			"    print(Duktape.enc('jx', res));\n"
			"})");
		duk_dup(ctx, 1);
		duk_dup(ctx, 2);
		duk_dup(ctx, 3);
		duk_dup(ctx, 4);
		duk_dup(ctx, 5);
		duk_call(ctx, 5);
		duk_pop(ctx);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	/* Indexed read/write */
	TEST_SAFE_CALL(test_duktape_buffer_indexed_1a);
	TEST_SAFE_CALL(test_nodejs_buffer_indexed_1a);
	TEST_SAFE_CALL(test_nodejs_buffer_indexed_1b);
	TEST_SAFE_CALL(test_arraybuffer_indexed_1a);
	TEST_SAFE_CALL(test_uint8array_indexed_1a);
	TEST_SAFE_CALL(test_uint8array_indexed_1b);
	TEST_SAFE_CALL(test_uint16array_indexed_1a);
	TEST_SAFE_CALL(test_uint16array_indexed_1b);
	TEST_SAFE_CALL(test_uint32array_indexed_1a);
	TEST_SAFE_CALL(test_uint32array_indexed_1b);
	TEST_SAFE_CALL(test_float32array_indexed_1a);
	TEST_SAFE_CALL(test_float32array_indexed_1b);
	TEST_SAFE_CALL(test_float64array_indexed_1a);
	TEST_SAFE_CALL(test_float64array_indexed_1b);

	/* JSON.serialize */
	TEST_SAFE_CALL(test_json_serialize_1);

	/* TypedArray constructor byte copy and fast copy */
	TEST_SAFE_CALL(test_typedarray_constructor_copy_1);

	/* TypedArray.prototype.set() */
	TEST_SAFE_CALL(test_typedarray_set_1);

	/* Node.js Buffer compare() */
	TEST_SAFE_CALL(test_nodejs_buffer_compare_1);

	/* Node.js Buffer write() */
	TEST_SAFE_CALL(test_nodejs_buffer_write_1);

	/* Node.js Buffer copy() */
	TEST_SAFE_CALL(test_nodejs_buffer_copy_1);

	/* Node.js Buffer concat() */
	TEST_SAFE_CALL(test_nodejs_buffer_concat_1);
}
