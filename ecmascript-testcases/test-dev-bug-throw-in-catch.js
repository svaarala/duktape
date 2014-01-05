/*
 *  Bug test case for assertion failures related to unwinding a try-catch block
 *  when the catch block throws an error before finishing.
 *
 *  Assertions must be enabled for the errors to manifest.
 */

/*===
works
works
===*/

/* This causes:
[D] duk_hthread_stacks.c:339 (duk_hthread_catchstack_unwind): unwinding catchstack idx 2: lexical environment active
PANIC 54: (duk_js_var.c:506): assertion failed: act->var_env == NULL (duk_js_var.c:506)
*/

function test1() {
    try {
        try {
            throw new Error('error 1');
        } catch (e) {
            throw new Error('error 2');
        }
    } catch (e2) {
        print('outer');
    }
    print('works');
}

/* This causes:
[D] duk_heap_markandsweep.c:887 (duk_heap_mark_and_sweep): garbage collect (mark-and-sweep) starting, requested flags: 0x00000000, effective flags: 0x00000000
PANIC 54: (duk_heap_markandsweep.c:619): assertion failed: DUK_HEAPHDR_GET_REFCOUNT(curr) == 0 (duk_heap_markandsweep.c:619)
*/

function test2() {
    try {
        throw new Error('error 1');
    } catch (e) {
        throw new Error('error 2');
    }
    print('works');
}

try {
    test2();
} catch (e) {
    print(e);
}
