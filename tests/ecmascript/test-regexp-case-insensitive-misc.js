/*===
true
true
===*/

function test() {
    var re;

    // http://www.fileformat.info/info/unicode/char/aba0/index.htm
    // U+ABA0 case normalizes to U+13D0.  Firefox matches both like
    // Duktape, Node.js v4.2.6 only matches U+ABA0.
    re = /\uaba0/i;
    print(re.test('\uaba0'));
    print(re.test('\u13d0'));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
