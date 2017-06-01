/*
 *  Gaps in .apply() args array.
 */

/*===
undefined undefined
undefined undefined
undefined undefined
string foo
undefined undefined
===*/

function test() {
    function f(a,b,c,d,e) {
        print(typeof a, a);
        print(typeof b, b);
        print(typeof c, c);
        print(typeof d, d);
        print(typeof e, e);
    }

    var args = [];
    args.length = 10;
    args[3] = 'foo';

    f.apply(null, args);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
