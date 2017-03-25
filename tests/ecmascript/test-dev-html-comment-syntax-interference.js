/*
 *  Example where valid ES5 code can be parsed as a ES6 Annex B HTML comment.
 *
 *  This is unlikely to happen in practice, and at least recent V8 (Node.js)
 *  also parses the contrived example as a HTML comment.
 */

/*===
false
0
===*/

function test() {
    var x = 10;

    // Evaluate as: 0 < ! --x ==> 0 < ! 9 ==> 0 < false ==> 0 < 0 ==> false
    var y = 0 <! --x;
    print(y);

    // Without spaces the meaning should be the same, but is recognized as a
    // HTML comment instead.  Strict ES5 engines would print 'false' here,
    // Duktape 2.1.x (and Node.js) print 0 instead, interpreting the line as
    // 'var y = 0'.
    var y = 0 <!--x;
    print(y);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
