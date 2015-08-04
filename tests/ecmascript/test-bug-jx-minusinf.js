/*===
-Infinity
SyntaxError
===*/

function test() {
    print(Duktape.dec('jx', '-Infinity'));

    // Duktape 1.2.2 (and previous): incorrectly parses as -Infinity
    try {
        print(Duktape.dec('jx', '1Infinity'));
    } catch (e) {
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
