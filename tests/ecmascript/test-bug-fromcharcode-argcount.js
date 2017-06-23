/*===
250
ABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJ
300
ABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJ
===*/

function test(count) {
    var src = [];
    var i;
    src.push('String.fromCharCode(');
    for (i = 0; i < count; i++) {
        if (i > 0) {
            src.push(',');
        }
        src.push(String(65 + (i % 10)));
    }
    src.push(')');
    return eval(src.join(''));
}

try {
    var t;

    try {
        t = test(250);
        print(t.length);
        print(t);
    } catch (e) {
        print(e.name);
    }

    /* This currently fails due to Duktape register count limits; the failure is
     * not specific to String.fromCharCode().  However, String.fromCharCode() is
     * an example of a function which could conceivably be used with a lot of
     * arguments, to build a large Unicode string.
     *
     * Limit raised to ~64k in Duktape 2.2.
     */
    try {
        t = test(300);
        print(t.length);
        print(t);
    } catch (e) {
        print(e.name);
    }
} catch (e) {
    print(e.stack || e);
}
