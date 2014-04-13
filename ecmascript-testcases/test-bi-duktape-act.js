/*===
-1 0 act
-2 10 basicTest
-3 21 global
===*/

function basicTest() {
    var i, t;
    for (i = -1; ; i--) {
        t = Duktape.act(i);
        if (!t) { break; }

        // Property set may change between versions, but at least
        // these should be present for now (there is also 'pc' but
        // that isn't so useful.
        print(i, t.lineNumber, t.function.name);
    }
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
running on line: 37
===*/

/* Simulate Duktape.line(). */

function getCurrentLine() {
    // indices: -1 = Duktape.act, -2 = getCurrentLine, -3 = caller
    var a = Duktape.act(-3) || {};
    return a.lineNumber;
}
print('running on line:', getCurrentLine());
