/*===
<2028><66><6f><6f>
<2029><66><6f><6f>
<2028><66><6f><6f>
<2029><66><6f><6f>
===*/

function dump(x) {
    print(x.replace(/[\u0000-\uffff]/g, function (c) { return '<' + c.charCodeAt(0).toString(16) + '>'; }));
}

function test() {
    var inp;

    // In ES2019 U+2028 and U+2029 are allowed in string literals so that
    // all JSON.stringify() output parses with eval().

    inp = '"\u2028foo"';
    dump(eval(inp));

    inp = '"\u2029foo"';
    dump(eval(inp));

    inp = '\'\u2028foo\'';
    dump(eval(inp));

    inp = '\'\u2029foo\'';
    dump(eval(inp));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
