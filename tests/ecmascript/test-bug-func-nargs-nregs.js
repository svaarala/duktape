/*===
2
===*/

/* This had a bug, causing an assertion error in the debug build:
 *
 * PANIC 54: (build/401/duk_js_call.c:1753): assertion failed: nregs >= nargs (build/401/duk_js_call.c:1753)
 *
 * nregs was 1, nargs was 2.
 *
 * The release build would return undefined.
 */

function f(n,v) {
    return v;
}

try {
    print(f(1,2));
} catch (e) {
    print(e.name);
}
