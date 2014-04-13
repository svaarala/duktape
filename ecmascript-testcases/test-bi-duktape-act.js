/*===
-1 0 act
-2 10 test
-3 21 global
===*/

function test() {
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
    test();
} catch (e) {
    print(e);
}
