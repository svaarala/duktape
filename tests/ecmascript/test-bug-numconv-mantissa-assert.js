/*
 *  Duktape 0.10.0 assert error from test262 ch15/15.1/15.1.2/15.1.2.2/S15.1.2.2_A7.2_T3:
 *
 *  PANIC 54: assertion failed: nc_ctx->digits[0] == 1 (duk_numconv.c:1402) (segfaulting on purpose)
 *
 *  Despite the assertion Duktape works correctly (passes this test).
 */

/*===
7.555786372591432e+22
===*/

try {
    print(75557863725914323419136);
} catch (e) {
    print(e);
}
