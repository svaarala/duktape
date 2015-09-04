/*
 *  ToObject() (E5 Section 9.9).
 *
 *  Calling Object(v) invokes ToObject() and returns the result directly,
 *  except for undefined and null.  Undefined and null should cause a TypeError.
 *  That side effect can be tested e.g. using Object.prototype.valueOf() which
 *  will call ToObject() directly on the 'this' value with no prechecks.
 *  Because Duktape Object.prototype.valueOf() is strict, the 'this' value is
 *  not coerced during call handling.
 */

/*===
0 TypeError
1 TypeError
[object Boolean]
[object Boolean]
[object Number]
[object String]
[object Array]
[object Object]
[object Function]
===*/

function test() {
    [ undefined, null, true, false, 123.0, 'foo',
      [], {}, function test() {} ].forEach(function (v, i) {
        try {
            if (v === undefined || v === null) {
                // just to get the TypeError
                print(Object.prototype.toString.call(Object.prototype.ValueOf(v)));
            } else {
                print(Object.prototype.toString.call(Object(v)));
            }
        } catch (e) {
            print(i, e.name);
        }
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
