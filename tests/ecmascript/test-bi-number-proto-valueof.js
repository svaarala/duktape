/*===
number -Infinity nonzero
number -1000000000 nonzero
number -123 nonzero
number -1 nonzero
number 0 -Infinity
number 0 Infinity
number 1 nonzero
number 123 nonzero
number 1000000000 nonzero
number Infinity nonzero
number NaN nonzero
true
false
===*/

function basicTest() {
    var obj;

    function test(x) {
        var obj;
        var t;

        try {
            obj = new Number(x);
            t = obj.valueOf();
            print(typeof t, t, (t === 0 ? (1 / t) : 'nonzero'));
        } catch (e) {
            print(e.name);
        }
    }

    test(Number.NEGATIVE_INFINITY);
    test(-1e9);
    test(-123);
    test(-1);
    test(-0);
    test(+0);
    test(1);
    test(123);
    test(1e9);
    test(Number.POSITIVE_INFINITY);
    test(Number.NaN);

    // valueOf() returns a plain number

    obj = new Number(123.456789012345678901234567890);
    print(obj.valueOf() == obj);  // true
    print(obj.valueOf() === obj);  // false, number vs. object
}

try {
    basicTest();
} catch (e) {
    print(e);
}
