/*
 *  Basic toLowerCase() tests.  Brute-force tests cover these conversions
 *  in much more detail.
 */

/*===
abcd
228
===*/

function basicTest() {
    print('AbCd'.toLowerCase());
    print('\u00c4'.toLowerCase().charCodeAt(0));  // U+00C4 (A with dots) -> U+00E4 (a with dots) = 228
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
string 3 97 962 33
string 3 97 963 97
===*/

/* Locale/language specific rules should NOT apply, but context specific
 * rules should. Test just a few basic rules.
 */

function localeTest() {
    var str, t;

    // context specific Greek sigma rule

    str = 'A\u03a3!';  // prev is letter, curr is U+03A3 (Greek capital letter sigma), next is not letter
    t = str.toLowerCase();
    print(typeof t, t.length, t.charCodeAt(0), t.charCodeAt(1), t.charCodeAt(2));

    str = 'A\u03a3A';  // should no longer apply
    t = str.toLowerCase();
    print(typeof t, t.length, t.charCodeAt(0), t.charCodeAt(1), t.charCodeAt(2));

    // FIXME: add locale specific test and ensure locale specific rules do not apply
}

try {
    localeTest();
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
            t = String.prototype.toLowerCase.call(str);
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

