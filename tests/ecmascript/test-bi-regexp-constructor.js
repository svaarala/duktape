/*
 *  RegExp constructor
 */

/*===
true /foo/gm
true /foo/gm
false /foo/gm
false /foo/i
/foo/gim
/foo/m
===*/

function test() {
    var x, y;

    // XXX: add basic cases, this testcase now mostly tests for ES2015 differences.

    // Constructor called as a plain function, with a RegExp instance argument
    // and undefined flags -> return as is.
    x = /foo/gm;
    y = RegExp(x);
    print(x === y, y.toString());
    y = RegExp(x, void 0);  // same as missing
    print(x === y, y.toString());
    y = RegExp(x, 'gm');  // present -> create copy
    print(x === y, y.toString());
    y = RegExp(x, 'i');  // present -> create copy
    print(x === y, y.toString());

    // New in ES2015: argument can be a regexp instance and overriding flags given.
    x = /foo/gim;
    print(x.toString());
    y = new RegExp(x, 'm');
    print(y.toString());
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
