// Fast path for input byte lengths <= 256.

/*===
"\ud800\udc00"
"\udc00\ud800"
"\udc00\ud800\udc00"
"\udc00\ud800\udc00\ud800"
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\ud800\udc00"
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\udc00\ud800"
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\udc00\ud800\udc00"
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\udc00\ud800\udc00\ud800"
"\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\ud800\udc00"
"\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\udc00\ud800"
"\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\udc00\ud800\udc00"
"\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\u1234\udc00\ud800\udc00\ud800"
===*/

function dump(v) {
    print(JSON.stringify(v).replace(/[\u0080-\uffff]/g, function (c) {
        return '\\u' + ('0000' + c.charCodeAt(0).toString(16)).substr(-4);
    }));
}

function test() {
    var x = '\ud800';
    var y = '\udc00';
    var z = ('x').repeat(256);
    var w = ('\u1234').repeat(86);  // 86*3 = 258

    dump(x + y);
    dump(y + x);
    dump(y + x + y);
    dump(y + x + y + x);

    dump(z + x + y);
    dump(z + y + x);
    dump(z + y + x + y);
    dump(z + y + x + y + x);

    dump(w + x + y);
    dump(w + y + x);
    dump(w + y + x + y);
    dump(w + y + x + y + x);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
