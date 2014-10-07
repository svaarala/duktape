/*===
FOOBAR
===*/

try {
    print('fOoBaR'.toLocaleUpperCase());
} catch (e) {
    print(e);
}

/*===
TypeError
TypeError
string 4 TRUE
string 5 FALSE
string 3 123
string 6 FOOBAR
string 9 1,FOO,BAR
string 15 [OBJECT OBJECT]
===*/

function coercionTest() {
    function test(str) {
        var t;

        try {
            t = String.prototype.toLocaleUpperCase.call(str);
            print(typeof t, t.length, t);
        } catch (e) {
            print(e.name);
        }
    }

    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123.0);
    test('FoObAr');
    test([1,'fOO','Bar']);
    test({ foo: 1, bar: 2 });
}

try {
    coercionTest();
} catch (e) {
    print(e);
}

/* XXX: basic tests like coercion etc. */

/* XXX: test that locale specific conversions actually work.
 * How to force locale for a test?
 */
