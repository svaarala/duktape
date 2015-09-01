/*
 *  RegExp literals (E5 Sections 7.8.5, 15.10).
 */

/*===
[object RegExp]
[object RegExp]
0 object [object RegExp]
1 object [object RegExp]
2 SyntaxError
3 SyntaxError
0 true
1 false
2 false
3 false
4 false
===*/

// RegExp in global code
print(Object.prototype.toString.call(/foo/));

function test() {
    var i;

    // RegExp in function code
    print(Object.prototype.toString.call(/bar/));

    [
        // regexps
        '/foo/', '/foo/gim',

        // regexp not allowed (confused with division)
        '1 /foo/',

        // newline not allowed inside regexp
        '/foo\nbar/'
    ].forEach(function (evalCode, i) {
        try {
            var t = eval(evalCode);
            print(i, typeof t, Object.prototype.toString.call(t));
        } catch (e) {
            print(i, e.name);
        }
    });

    // Each time a literal is evaluated a new RegExp is created
    var regexps = [];
    for (i = 0; i < 5; i++) {
        regexps[i] = /foobar/gim;
    }
    for (i = 0; i < regexps.length; i++) {
        print(i, regexps[0] === regexps[i]);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
