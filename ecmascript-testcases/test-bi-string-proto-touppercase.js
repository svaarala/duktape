/*
 *  Basic toUpperCase() tests.  Brute-force tests cover these conversions
 *  in much more detail.
 */

/*===
ABCD
196
===*/

function basicTest() {
    print('AbCd'.toUpperCase());
    print('\u00e4'.toUpperCase().charCodeAt(0));  // U+00E4 (a with dots) -> U+00E4 (A with dots) = 196
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
===*/

/* Locale/language specific rules should NOT apply, but context specific
 * rules should. Test just a few basic rules.
 */

function localeTest() {
    var str, t;

    // XXX: add context specific test and ensure context specific rules apply
    // XXX: add locale specific test and ensure locale specific rules do not apply
}

try {
    localeTest();
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
            t = String.prototype.toUpperCase.call(str);
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
