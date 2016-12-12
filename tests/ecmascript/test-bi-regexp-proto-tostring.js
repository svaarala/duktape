/*===
/foo/gm
/(?:)/
/undefined/undefined
/troll/face
===*/

function test() {
    var re = /foo/gm;
    print(RegExp.prototype.toString.call(re));

    // Must be generic: for RegExp.prototype or any other object
    // would normally return '/undefined/undefined', based on an
    // actual .source and .flags lookup.
    re = RegExp.prototype;
    print(RegExp.prototype.toString.call(re));
    re = {};
    print(RegExp.prototype.toString.call(re));
    re = { source: 'troll', flags: 'face' };
    print(RegExp.prototype.toString.call(re));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
