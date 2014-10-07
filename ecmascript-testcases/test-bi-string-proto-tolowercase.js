/*
 *  Basic toLowerCase() tests.  Brute-force tests cover these conversions
 *  in much more detail.
 */

function dumpString(x) {
    var tmp = [];
    for (var i = 0; i < x.length; i++) {
        tmp.push(x.charCodeAt(i));
    }
    print(typeof x, x.length, tmp.join(' '));
}

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
string 3 57 963 33
string 2 963 33
string 3 97 962 33
string 2 97 962
string 3 97 963 97
===*/

/* Context/language specific rules.  Currently locale/language specific
 * rules don't apply (specific locales are not implemented), but general
 * context sensitive rules do apply.
 */

function greekSigmaTest() {
    var str, t;

    /* Context specific Greek sigma rule.  This is a rather difficult rule to
     * implement (for example, it requires neighboring codepoints to be tested
     * against 'Unicode letter' which is not needed elsewhere).
     */

    // Prev is not a letter, curr is U+03A3 (Greek capital letter sigma), next is not letter.
    // Here the sigma occurs at the end but is the single letter in the "word", so the special
    // rule does not apply.
    dumpString(('9\u03a3!').toLowerCase());

    // Prev does not exist, next is not a letter -> same case.
    dumpString(('\u03a3!').toLowerCase());

    // Prev is a letter, next is not a letter -> rule applies (end of word)
    dumpString(('A\u03a3!').toLowerCase());

    // Prev is a letter, next does not exist -> same case.
    dumpString(('A\u03a3').toLowerCase());

    // Prev is a letter, next is a letter -> in the middle of the word rule should no longer apply
    dumpString(('A\u03a3A').toLowerCase());
}

function localeTest() {
    var str, t;

    // XXX: add locale specific test and ensure locale specific rules do not apply
}

try {
    greekSigmaTest();
} catch (e) {
    print(e);
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
