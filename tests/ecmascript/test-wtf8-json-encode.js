/*===
"foo<55296><56320>bar<55723><57325>quux<55296>baz<56320>quuux<56320><55296>quuuux"
===*/

function test() {
    var v = 'foo\ud800\udc00bar\ud9ab\udfedquux\ud800baz\udc00quuux\udc00\ud800quuuux';
    var e = JSON.stringify(v).replace(/./g, function (c) { if (c.charCodeAt(0) < 0x80) { return c; } else { return '<' + c.charCodeAt(0) + '>'; } });
    print(e);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
