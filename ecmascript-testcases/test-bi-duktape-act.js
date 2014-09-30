/*===
-1 0 act
-2 10 basicTest
-3 26 global
===*/

function basicTest() {
    var i, t;
    for (i = -1; ; i--) {
        t = Duktape.act(i);
        if (!t) { break; }

        // Property set may change between versions, but at least
        // these should be present for now (there is also 'pc' but
        // that isn't so useful.
        //
        // NOTE: normally Duktape.act.name is 'act' but when using
        // DUK_OPT_LIGHTFUNC_BUILTINS Duktape.act() will be a lightfunc
        // and have a generic name (e.g. lightfunc_deadbeef_1234).  To
        // make the test case generic, avoid printing Duktape.act name.
        print(i, t.lineNumber, t.function === Duktape.act ? 'act' : t.function.name);
    }
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
running on line: 42
===*/

/* Simulate Duktape.line(). */

function getCurrentLine() {
    // indices: -1 = Duktape.act, -2 = getCurrentLine, -3 = caller
    var a = Duktape.act(-3) || {};
    return a.lineNumber;
}
print('running on line:', getCurrentLine());
