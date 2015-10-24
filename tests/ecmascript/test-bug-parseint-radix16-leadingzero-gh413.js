/*
 *  https://github.com/svaarala/duktape/issues/413
 */

/*===
225
123
83
102
291
4660
0
4660
4660
===*/

function test() {
    // Concrete bug reported in GH-413, prints '0' in Duktape 1.3.0,
    // should parse as 0x00e1.
    print(parseInt('00e1', 16));

    // Some related tests in the same code path.
    print(parseInt('0123'));        // 83 in Rhino, 123 in Node v0.12.1; Duktape now treats as 123 too (GH-414)
    print(parseInt('0123', 8));     // 83 in Rhino and node v0.12.1
    print(parseInt('0123', 9));     // 102 in Rhino and node v0.12.1
    print(parseInt('0123', 16));
    print(parseInt('0x1234'));
    print(parseInt('0x1234', 15));  // this must parse as 0, 0x autodetect not allowed
    print(parseInt('0x1234', 16));
    print(parseInt('0X1234', 16));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
