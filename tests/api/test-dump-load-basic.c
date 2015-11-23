/*
 *  Bytecode dump/load
 *
 *  This testcase is a little fragile because it dumps bytecode, pc2line
 *  data etc.
 */

#define MANDELBROT ( \
	"function mandel() {\n" \
	"    var w = 76, h = 28, iter = 100;\n" \
	"    var i, j, k, c;\n" \
	"    var x0, y0, xx, yy, xx2, yy2;\n" \
	"    var line;\n" \
	"    for (i = 0; i < h; i++) {\n" \
	"        y0 = (i / h) * 2.5 - 1.25;\n" \
	"        for (j = 0, line = []; j < w; j++) {\n" \
	"            x0 = (j / w) * 3.0 - 2.0;\n" \
	"            for (k = 0, xx = 0, yy = 0, c = '#'; k < iter; k++) {\n" \
	"                xx2 = xx*xx; yy2 = yy*yy;\n" \
	"\n" \
	"                if (xx2 + yy2 < 4.0) {\n" \
	"                    yy = 2*xx*yy + y0;\n" \
	"                    xx = xx2 - yy2 + x0;\n" \
	"                } else {\n" \
	"                    /* xx^2 + yy^2 >= 4.0 */\n" \
	"                    if (k < 3) { c = '.'; }\n" \
	"                    else if (k < 5) { c = ','; }\n" \
	"                    else if (k < 10) { c = '-'; }\n" \
	"                    else { c = '='; }\n" \
	"                    break;\n" \
	"                }\n" \
	"            }\n" \
	"            line.push(c);\n" \
	"        }\n" \
	"        print(line.join(''));\n" \
	"    }\n" \
	"}\n" \
	)

/*===
*** test_basic (duk_safe_call)
dump result type: 7
ff000000000e0000000300000001000900000000000100000001300004c0000000f300400052000040c200000114000101400001414c800041c38000820301014030000141000000814201804030000040000000006f00000000057072696e74000000000568656c6c6f0140091eb851eb851f000000030000000000000000000300020000000100000001300604400080009a0000806f0000002f000000020000000561646465720000000f66616b6546696c656e616d652e6a730000000d03000000010000000c000000000000000178000000000000000179000000010000000000000001780000000179000000000000000000000006676c6f62616c0000000f66616b6546696c656e616d652e6a730000000e0e000000010000000c00000000000000000000000000
load result type: 6
hello 3 3.14
call result type: 1
call result: undefined
final top: 1
==> rc=0, result='undefined'
===*/

/* Basic dump/load test which exercises all constant types and has an inner
 * function.  Hex dumping the bytecode provides an exact test case dependency
 * to the dump format so that any accidental changes break the test.
 */
static duk_ret_t test_basic(duk_context *ctx) {
	unsigned char *p;
	duk_size_t i, sz;

	/* Integer constants generate LDINT now so also use a fractional
	 * constant to exercise number constants.
	 *
	 * Compiled as a program; force source filename.
	 */
	duk_push_string(ctx, "print('hello', (function adder(x,y) { return x+y; })(1, 2), 3.14);");
	duk_push_string(ctx, "fakeFilename.js");
	duk_compile(ctx, 0);
	duk_dump_function(ctx);
	printf("dump result type: %d\n", (int) duk_get_type(ctx, -1));
	fflush(stdout);
	p = (unsigned char *) duk_get_buffer(ctx, -1, &sz);
	for (i = 0; i < sz; i++) {
		printf("%02x", (int) p[i]);
	}
	printf("\n");
	fflush(stdout);

	/* Load test function from dump and execute it. */
	duk_load_function(ctx);
	printf("load result type: %d\n", (int) duk_get_type(ctx, -1));
	fflush(stdout);
	duk_call(ctx, 0);
	printf("call result type: %d\n", (int) duk_get_type(ctx, -1));
	fflush(stdout);
	printf("call result: %s\n", duk_safe_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_mandel (duk_safe_call)
Mandelbrot source length: 884
ff000000005c000000100000000000130000000000010000001e300604c0801300038007004380190083000008338000156e8000016e800000c30080c3ab0003802d800000ee800013ee0000c3b67ffffeae0080c39d8003839c8083821b0000483380000fee800001ee800001030004023300040340000103ab0003802d800000ee80000dee000103b67ffffeae0001039d8103839c818381db0000883380000a6e8000022e80000143800002438000028300010182010143ab0003802d800000ee8000082e000143b67ffffeae048242dc0502831c0602c39a828383ab0003806d800001ae04c0c39c0503839c0403829a0602c39b0383825a7ffffcae810143ab0003806d800000ae000181828000036e838143ab0003806d800000ae000201828000022e848143ab0003806d800000ae00028182800000ee0002c1828000006e7ffff82e0000887300034380000303c20783839800018400008380307ffff26e00004873004343920003440000038442088404180003c48200840030008380307fffec6e000008730000002f014004000000000000013ff400000000000001400800000000000001400000000000000000000000012301401000000000000000000000012e01401400000000000000000000012c01402400000000000000000000012d00000000013d00000000047075736800000000057072696e7400000000046a6f696e000000000000000000000000066d616e64656c000000096d616e64656c2e6a73000000285c000000020000001400000013000000210000002880201001008004244108910080844a02100420000000017700000000000000016800000001000000046974657200000002000000016900000003000000016a00000004000000016b000000050000000163000000060000000278300000000700000002793000000008000000027878000000090000000279790000000a000000037878320000000b000000037979320000000c000000046c696e650000000d0000000000000000
..........................,,,,,,,,,,,,,,,,,,,,,,,,,.........................
....................,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,...................
................,,,,,,,,,,,,,,,,,,,,,,,,,,,---=----,,,,,,,,,,...............
.............,,,,,,,,,,,,,,,,,,,,,,,,,,,,------====--,,,,,,,,,,,............
..........,,,,,,,,,,,,,,,,,,,,,,,,,,,,,-------====------,,,,,,,,,,,.........
........,,,,,,,,,,,,,,,,,,,,,,,,,,,,,-------=#####==-------,,,,,,,,,,.......
......,,,,,,,,,,,,,,,,,,,,,,,,,,,----------==######=----------,,,,,,,,,.....
.....,,,,,,,,,,,,,,,,,,,,,,,-------=========#########=#=-====--,,,,,,,,,....
....,,,,,,,,,,,,,,,,,,--------------=######################==---,,,,,,,,,...
...,,,,,,,,,,,,,,----------------==########################=-----,,,,,,,,,..
..,,,,,,,,,,,,----==----==------===##########################==--,,,,,,,,,,.
.,,,,,,,,,,---------==#==#========###########################=---,,,,,,,,,,,
.,,,,,,,,---------===##########==############################=---,,,,,,,,,,,
.,,,,,,--------=====########################################=----,,,,,,,,,,,
.#########################################################==-----,,,,,,,,,,,
.,,,,,,--------=====########################################=----,,,,,,,,,,,
.,,,,,,,,---------===##########==############################=---,,,,,,,,,,,
.,,,,,,,,,,---------==#==#========###########################=---,,,,,,,,,,,
..,,,,,,,,,,,,----==----==------===##########################==--,,,,,,,,,,.
...,,,,,,,,,,,,,,----------------==########################=-----,,,,,,,,,..
....,,,,,,,,,,,,,,,,,,--------------=######################==---,,,,,,,,,...
.....,,,,,,,,,,,,,,,,,,,,,,,-------=========#########=#=-====--,,,,,,,,,....
......,,,,,,,,,,,,,,,,,,,,,,,,,,,----------==######=----------,,,,,,,,,.....
........,,,,,,,,,,,,,,,,,,,,,,,,,,,,,-------=#####==-------,,,,,,,,,,.......
..........,,,,,,,,,,,,,,,,,,,,,,,,,,,,,-------====------,,,,,,,,,,,.........
.............,,,,,,,,,,,,,,,,,,,,,,,,,,,,------====--,,,,,,,,,,,............
................,,,,,,,,,,,,,,,,,,,,,,,,,,,---=----,,,,,,,,,,...............
....................,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,...................
call result: undefined
final top: 1
==> rc=0, result='undefined'
===*/

/* Dump/load mandelbrot.  No inner functions but a bit more code. */
static duk_ret_t test_mandel(duk_context *ctx) {
	unsigned char *p;
	duk_size_t i, sz;

	printf("Mandelbrot source length: %ld\n", (long) strlen(MANDELBROT));

	/* Compiled as a function; force source filename. */

	duk_push_string(ctx, MANDELBROT);
	duk_push_string(ctx, "mandel.js");
	duk_compile(ctx, DUK_COMPILE_FUNCTION);
	duk_dump_function(ctx);
	p = (unsigned char *) duk_get_buffer(ctx, -1, &sz);
	for (i = 0; i < sz; i++) {
		printf("%02x", (int) p[i]);
	}
	printf("\n");
	fflush(stdout);

	/* Load test function from dump and execute it. */
	duk_load_function(ctx);
	duk_call(ctx, 0);
	printf("call result: %s\n", duk_safe_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_large_func (duk_safe_call)
9999
call result: undefined
final top: 1
==> rc=0, result='undefined'
===*/

/* Test dumping of a large function to exercise buffer resizes. */
static duk_ret_t test_large_func(duk_context *ctx) {
	duk_eval_string(ctx,
		"(function () {\n"
		"    var res = [];\n"
		"    res.push('(function myFunc() {');\n"
		"    var i;\n"
		"    for (i = 0; i < 5e4; i++) {\n"
		"        res.push('var test' + i + ' = ' + i + ';');\n"
		"    }\n"
		"    res.push('print(test9999);')\n"
		"    res.push('})');\n"
		"    return eval(res.join('\\n'));\n"
		"})()");
	duk_dump_function(ctx);
	duk_load_function(ctx);
	duk_call(ctx, 0);
	printf("call result: %s\n", duk_safe_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_properties (duk_safe_call)
.length: {"value":3,"writable":false,"enumerable":false,"configurable":false}
.name: {"value":"test","writable":false,"enumerable":false,"configurable":false}
.fileName: {"value":"fakeFilename.js","writable":true,"enumerable":false,"configurable":true}
.prototype: {"value":{},"writable":true,"enumerable":false,"configurable":false}
._Formals: {value:["a","b","c"],writable:false,enumerable:false,configurable:false}
._Varmap: {value:{a:0,b:1,c:2,x:3},writable:false,enumerable:false,configurable:false}
._Pc2line: {value:|04000000020000000c00000000|,writable:true,enumerable:false,configurable:true}
typeof .prototype: object
typeof .prototype.constructor: function
.prototype.constructor === func: true
descriptor of .prototype: {"value":{},"writable":true,"enumerable":false,"configurable":false}
descriptor of .prototype.constructor: {"writable":true,"enumerable":false,"configurable":true}
final top: 0
==> rc=0, result='undefined'
===*/

/* Properties and property attributes of a loaded function. */
static duk_ret_t test_properties(duk_context *ctx) {
	/* Compile explicitly to force fileName. */
	duk_push_string(ctx,
		"(function () {\n"
		"    var f = function test(a, b, c) { var x = 123; y = 234; };\n"
		"    return f;\n"
		"})()");
	duk_push_string(ctx, "fakeFilename.js");
	duk_compile(ctx, DUK_COMPILE_EVAL);
	duk_call(ctx, 0);

	duk_dump_function(ctx);
	duk_load_function(ctx);

	duk_eval_string(ctx,
		"(function (v) {\n"
		"    var pfx = Duktape.dec('hex', 'ff');\n"
		"    [ 'length', 'name', 'fileName', 'prototype' ].forEach(function (k) {\n"
		"        print('.' + k + ': ' + JSON.stringify(Object.getOwnPropertyDescriptor(v, k)));\n"
		"    });\n"
		"    // internal properties; print with JX to print buffer\n"
		"    [ 'Formals', 'Varmap', 'Pc2line' ].forEach(function (k) {\n"
		"        print('._' + k + ': ' + Duktape.enc('jx', Object.getOwnPropertyDescriptor(v, pfx + k)));\n"
		"    });\n"
		"    // .prototype\n"
		"    print('typeof .prototype: ' + typeof v.prototype);\n"
		"    print('typeof .prototype.constructor: ' + typeof v.prototype.constructor);\n"
		"    print('.prototype.constructor === func: ' + (v === v.prototype.constructor));\n"
		"    print('descriptor of .prototype: ' + JSON.stringify(Object.getOwnPropertyDescriptor(v, 'prototype')));\n"
		"    print('descriptor of .prototype.constructor: ' + JSON.stringify(Object.getOwnPropertyDescriptor(v.prototype, 'constructor')));\n"
		"})");
	duk_dup(ctx, -2),
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_program_code (duk_safe_call)
{"value":123,"writable":true,"enumerable":true,"configurable":false}
{"value":234,"writable":true,"enumerable":true,"configurable":false}
final top: 0
==> rc=0, result='undefined'
===*/

/* Test serialization of Program (instead of function) code:
 *
 * - Bindings established via Eval code are not configurable
 *   (E5 Section 10.5 step 2).
 */
static duk_ret_t test_program_code(duk_context *ctx) {
	/* Demonstrate behavior without dump/load. */
	duk_compile_string(ctx, 0,
		"var testProgram1 = 123;"
	);
	duk_call(ctx, 0);
	duk_pop(ctx);
	duk_eval_string_noresult(ctx,
		"print(JSON.stringify(Object.getOwnPropertyDescriptor(this, 'testProgram1')));"
	);

	/* Same with dump/load. */
	duk_compile_string(ctx, 0,
		"var testProgram2 = 234;"
	);
	duk_dump_function(ctx);
	duk_load_function(ctx);
	duk_call(ctx, 0);
	duk_pop(ctx);
	duk_eval_string_noresult(ctx,
		"print(JSON.stringify(Object.getOwnPropertyDescriptor(this, 'testProgram2')));"
	);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_eval_code (duk_safe_call)
{"value":123,"writable":true,"enumerable":true,"configurable":true}
{"value":234,"writable":true,"enumerable":true,"configurable":true}
final top: 0
==> rc=0, result='undefined'
===*/

/* Test serialization of Eval (instead of function) code:
 *
 * - Bindings established via Eval code are configurable; bindings
 *   established from program and function code are not.  (E5 Section
 *   10.5 step 2).
 *
 * Here we bytecode dump an eval function that is then loaded and executed
 * in the global scope.
 */
static duk_ret_t test_eval_code(duk_context *ctx) {
	/* Demonstrate behavior without dump/load. */
	duk_compile_string(ctx, DUK_COMPILE_EVAL,
		"var testEval1 = 123;"
	);
	duk_call(ctx, 0);
	duk_pop(ctx);
	duk_eval_string_noresult(ctx,
		"print(JSON.stringify(Object.getOwnPropertyDescriptor(this, 'testEval1')));"
	);

	/* Same with dump/load. */
	duk_compile_string(ctx, DUK_COMPILE_EVAL,
		"var testEval2 = 234;"
	);
	duk_dump_function(ctx);
	duk_load_function(ctx);
	duk_call(ctx, 0);
	duk_pop(ctx);
	duk_eval_string_noresult(ctx,
		"print(JSON.stringify(Object.getOwnPropertyDescriptor(this, 'testEval2')));"
	);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_strict (duk_safe_call)
strict: false
strict: true
final top: 0
==> rc=0, result='undefined'
===*/

/* Strictness status is preserved. */
static duk_ret_t test_strict(duk_context *ctx) {
	duk_compile_string(ctx, DUK_COMPILE_FUNCTION,
		"function () {\n"
		"    var strict = (function () { return !this; })();\n"
		"    print('strict: ' + strict);\n"
		"}");
	duk_dump_function(ctx);
	duk_load_function(ctx);
	duk_call(ctx, 0);
	duk_pop(ctx);

	duk_compile_string(ctx, DUK_COMPILE_FUNCTION,
		"function () {\n"
		"    'use strict';\n"
		"    var strict = (function () { return !this; })();\n"
		"    print('strict: ' + strict);\n"
		"}");
	duk_dump_function(ctx);
	duk_load_function(ctx);
	duk_call(ctx, 0);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_varmap (duk_safe_call)
hello world
123
final top: 0
==> rc=0, result='undefined'
===*/

/* _Varmap is preserved if function needs it. */
static duk_ret_t test_varmap(duk_context *ctx) {
	/* Get access to _Varmap by creating a function that provides
	 * an 'eval service' in a function scope.
	 */
	duk_compile_string(ctx, DUK_COMPILE_FUNCTION,
		"function (code) {\n"
		"    var foo = 123;\n"
		"    eval(code);\n"
		"}");
	duk_dump_function(ctx);
	duk_load_function(ctx);

	duk_dup(ctx, -1);
	duk_push_string(ctx, "print('hello world');");
	duk_call(ctx, 1);
	duk_pop(ctx);

	/* Eval code will use GETVAR to read 'foo', and _Varmap is
	 * needed for that.
	 */

	duk_dup(ctx, -1);
	duk_push_string(ctx, "print(foo);");
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_arguments_object (duk_safe_call)
object [object Arguments]
foo bar
123 bar
object [object Arguments]
foo bar
123 bar
final top: 0
==> rc=0, result='undefined'
===*/

/* Arguments object still works after dump/load, relies on e.g. _Formals. */
static duk_ret_t test_arguments_object(duk_context *ctx) {
	duk_eval_string(ctx,
		"(function () {\n"
		"    var f = function test(x,y) {\n"
		"        print(typeof arguments, Object.prototype.toString.call(arguments));\n"
		"        print(arguments[0], arguments[1]);\n"
		"        arguments[0] = 123;\n"
		"        print(x, y);\n"
		"    };\n"
		"    return f;\n"
		"})()");
	duk_dup_top(ctx);
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	duk_call(ctx, 2);
	duk_pop(ctx);
	duk_dump_function(ctx);
	duk_load_function(ctx);
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	duk_call(ctx, 2);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_pc2line (duk_safe_call)
hello world
Error: aiee 5
hello world
Error: aiee 5
final top: 0
==> rc=0, result='undefined'
===*/

/* _Pc2line is preserved, check by traceback line numbers. */
static duk_ret_t test_pc2line(duk_context *ctx) {
	duk_eval_string(ctx,
		"(function () {\n"
		"    var f = function test() {\n"
		"        print('hello world');\n"
		"        try {\n"
		"            throw new Error('aiee');\n"
		"        } catch (e) {\n"
		"            print(e, e.lineNumber);\n"
		"        }\n"
		"    };\n"
		"    return f;\n"
		"})()");
	duk_dup_top(ctx);
	duk_call(ctx, 0);  /* undumped */
	duk_pop(ctx);
	duk_dump_function(ctx);
	duk_load_function(ctx);
	duk_call(ctx, 0);  /* dump/load */
	duk_pop(ctx);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_name_binding_funcexpr (duk_safe_call)
i am a function
i am a function
5
4
3
2
1
0
7
6
5
4
3
2
1
0
final top: 0
==> rc=0, result='undefined'
===*/

/* Name binding for function expressions is preserved, it is important
 * for recursive functions.
 */
static duk_ret_t test_name_binding_funcexpr(duk_context *ctx) {
	duk_eval_string(ctx,
		"(function () {\n"
		"    var f = function test() { print('i am a ' + typeof test); };\n"
		"    return f;\n"
		"})()");
	duk_dup_top(ctx);
	duk_call(ctx, 0);  /* undumped */
	duk_pop(ctx);
	duk_dump_function(ctx);
	duk_load_function(ctx);
	duk_call(ctx, 0);  /* dump/load */
	duk_pop(ctx);

	duk_eval_string(ctx,
		"(function () {\n"
		"    var f = function test(n) { print(n); if (n > 0) { test(n - 1); } };\n"
		"    return f;\n"
		"})()");
	duk_dup_top(ctx);
	duk_push_int(ctx, 5);
	duk_call(ctx, 1);  /* undumped */
	duk_pop(ctx);
	duk_dump_function(ctx);
	duk_load_function(ctx);
	duk_push_int(ctx, 7);
	duk_call(ctx, 1);  /* dump/load */
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_name_binding_funcdecl (duk_safe_call)
i am a number
final top: 1
==> rc=0, result='undefined'
===*/

/* Self-references for a function established through a function declaration
 * is unfortunately not automatically supported.  The self-reference generates
 * bytecode that performs a slow path GETVAR for the function name; the GETVAR
 * normally gets satisfied by looking it up from the global object.  But if
 * we dump/load the function, only the function object is resurrected while the
 * global binding is not.
 */
static duk_ret_t test_name_binding_funcdecl(duk_context *ctx) {
	duk_compile_string(ctx, 0 /*flags: program*/,
		"function declaredTest() {\n"
		"    print('i am a ' + typeof declaredTest);\n"
		"}");
	duk_call(ctx, 0);
	duk_pop(ctx);

	duk_get_global_string(ctx, "declaredTest");
	duk_dump_function(ctx);
	duk_eval_string_noresult(ctx, "declaredTest = 123;");  /* lose original */
	duk_load_function(ctx);
	duk_call(ctx, 0);

	/* Because the declaredTest binding itself is not resurrected (only
	 * the function itself is), the "typeof declaredTest" will refer to
	 * the fake "declaredTest = 123" value, i.e. a number.
	 */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_bound_rejected (duk_safe_call)
dummythis x-arg y-arg
==> rc=1, result='TypeError: compiledfunction required, found [object Function] (stack index -1)'
===*/

/* Bound functions are rejected with TypeError. */
static duk_ret_t test_bound_rejected(duk_context *ctx) {
	/* XXX: Perhaps rework bound function support so that the final non-bound
	 * function is serialized instead?
	 */
	duk_eval_string(ctx,
		"(function () {\n"
		"    var f = function test(x,y) { print(this, x, y); };\n"
		"    return f.bind('dummythis', 'x-arg', 'y-arg');\n"
		"})()");
	duk_dup_top(ctx);
	duk_call(ctx, 0);
	duk_pop(ctx);
	duk_dump_function(ctx);
	duk_load_function(ctx);
	duk_call(ctx, 0);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_external_prototype_lost (duk_safe_call)
inst prototype is Function.prototype: false
inst prototype is custom prototype: true
myValue
f prototype is Function.prototype: 1
myProperty: undefined
final top: 0
==> rc=0, result='undefined'
===*/

/* Custom external prototype is lost during a dump/load. */
static duk_ret_t test_external_prototype_lost(duk_context *ctx) {
	duk_eval_string(ctx,
		"(function () {\n"
		"    var f = function test() {};\n"
		"    var proto = { myProperty: 'myValue' };\n"
		"    f.prototype = proto;\n"
		"    var inst = new f();\n"
		"    print('inst prototype is Function.prototype: ' + (Object.getPrototypeOf(inst) === Function.prototype));\n"
		"    print('inst prototype is custom prototype: ' + (Object.getPrototypeOf(inst) === proto));\n"
		"    print(inst.myProperty);\n"
		"    return f;\n"
		"})()");
	duk_dump_function(ctx);
	duk_load_function(ctx);

	duk_get_prototype(ctx, -1);
	duk_eval_string(ctx, "Function.prototype");
	printf("f prototype is Function.prototype: %d\n", (int) duk_strict_equals(ctx, -1, -2));
	duk_pop_2(ctx);

	duk_new(ctx, 0);
	duk_get_prop_string(ctx, -1, "myProperty");
	printf("myProperty: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_internal_prototype_lost (duk_safe_call)
myValue
myProperty: undefined
f prototype is Function.prototype: 1
final top: 0
==> rc=0, result='undefined'
===*/

/* Custom internal prototype is lost during a dump/load. */
static duk_ret_t test_internal_prototype_lost(duk_context *ctx) {
	duk_eval_string(ctx,
		"(function () {\n"
		"    var f = function test() {};\n"
		"    Object.setPrototypeOf(f, { myProperty: 'myValue' });\n"
		"    print(f.myProperty);\n"
		"    return f;\n"
		"})()");
	duk_dump_function(ctx);
	duk_load_function(ctx);

	duk_get_prop_string(ctx, -1, "myProperty");
	printf("myProperty: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prototype(ctx, -1);
	duk_eval_string(ctx, "Function.prototype");
	printf("f prototype is Function.prototype: %d\n", (int) duk_strict_equals(ctx, -1, -2));
	duk_pop_2(ctx);

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_mandel);
	TEST_SAFE_CALL(test_large_func);

	TEST_SAFE_CALL(test_properties);
	TEST_SAFE_CALL(test_program_code);
	TEST_SAFE_CALL(test_eval_code);
	TEST_SAFE_CALL(test_strict);
	TEST_SAFE_CALL(test_varmap);
	TEST_SAFE_CALL(test_arguments_object);
	TEST_SAFE_CALL(test_pc2line);
	TEST_SAFE_CALL(test_name_binding_funcexpr);
	TEST_SAFE_CALL(test_name_binding_funcdecl);
	TEST_SAFE_CALL(test_bound_rejected);
	TEST_SAFE_CALL(test_external_prototype_lost);
	TEST_SAFE_CALL(test_internal_prototype_lost);
}
