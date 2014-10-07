/*===
foobar
===*/

try {
    print('fOoBaR'.toLocaleLowerCase());
} catch (e) {
    print(e);
}

/*===
TypeError
TypeError
string 4 true
string 5 false
string 3 123
string 6 foobar
string 9 1,foo,bar
string 15 [object object]
===*/

function coercionTest() {
    function test(str) {
        var t;

        try {
            t = String.prototype.toLocaleLowerCase.call(str);
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
