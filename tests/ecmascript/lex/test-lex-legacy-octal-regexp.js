/*
 *  Legacy octal literals like \377 in regexps.
 */

var re;

/*===
true false
===*/

// \0 is interpreted as a literal NUL (U+0000), even without legacy support.
try {
    re = /[\0]/;
    print(re.test('\u0000'), re.test('0'));
} catch (e) {
    print(e.name);
}

/*===
true false
true false
true false
true false
true false
true false
true false
===*/

// \1 to \7 are octal U+0001 to U+0007.
try {
    re = /[\1]/;
    print(re.test('\u0001'), re.test('1'));
    re = /[\2]/;
    print(re.test('\u0002'), re.test('2'));
    re = /[\3]/;
    print(re.test('\u0003'), re.test('3'));
    re = /[\4]/;
    print(re.test('\u0004'), re.test('4'));
    re = /[\5]/;
    print(re.test('\u0005'), re.test('5'));
    re = /[\6]/;
    print(re.test('\u0006'), re.test('6'));
    re = /[\7]/;
    print(re.test('\u0007'), re.test('7'));
} catch (e) {
    print(e.name);
}

/*===
false true
false true
===*/

// \8 and \9 are literal '8' (U+0038) and '9' (U+0039) characters,
// *NOT* U+0008 and U+0009.
try {
    re = /[\8]/;
    print(re.test('\u0008'), re.test('8'));
    re = /[\9]/;
    print(re.test('\u0009'), re.test('9'));
} catch (e) {
    print(e.name);
}

/*===
true false
===*/

// \00 is interpreted as U+0000, '0' is still not allowed.
try {
    re = /[\00]/;
    print(re.test('\u0000'), re.test('0'));
} catch (e) {
    print(e.name);
}

/*===
true false false false
===*/

// \07 is interpreted like \7, i.e. U+0007.
try {
    re = /[\07]/;
    print(re.test('\u0007'), re.test('7'), re.test('\u0000'), re.test('0'));
} catch (e) {
    print(e.name);
}

/*===
false true true false
false true true false
===*/

// \08 is interpreted as U+0000 and '8'.  Similarly for \09.
try {
    re = /[\08]/;
    print(re.test('\u0008'), re.test('8'), re.test('\u0000'), re.test('0'));
    re = /[\09]/;
    print(re.test('\u0009'), re.test('9'), re.test('\u0000'), re.test('0'));
} catch (e) {
    print(e.name);
}

/*===
true false
===*/

// \000 is interpreted as U+0000, '0' not allowed.
try {
    re = /[\000]/;
    print(re.test('\u0000'), re.test('0'));
} catch (e) {
    print(e.name);
}

/*===
true false false false
===*/

// \007 is interpreted as \7.
try {
    re = /[\007]/;
    print(re.test('\u0007'), re.test('7'), re.test('\u0000'), re.test('0'));
} catch (e) {
    print(e.name);
}

/*===
false true true false
false true true false
===*/

// \008 is interpreted as U+0000 and '8', similarly for \009.
try {
    re = /[\008]/;
    print(re.test('\u0008'), re.test('8'), re.test('\u0000'), re.test('0'));
    re = /[\009]/;
    print(re.test('\u0009'), re.test('9'), re.test('\u0000'), re.test('0'));
} catch (e) {
    print(e.name);
}

/*===
true
===*/

// \377 is maximum octal allowed, interpreted as U+00FF.
try {
    re = /[\377]/;
    print(re.test('\u00ff'));
} catch (e) {
    print(e.name);
}

/*===
true true false false false false
===*/

// \400 is interpreted as \40 octal (U+0020) followed by literal 0.
try {
    re = /[\400]/;
    print(re.test(' '), re.test('0'), re.test('\u0000'), re.test('\u0100'), re.test('\u0004'), re.test('4'));
} catch (e) {
    print(e.name);
}

/*===
true true false
===*/

// \777 is interpreted as \77 octal (U+003F) followed by literal 7.
try {
    re = /[\777]/;
    print(re.test('\u003f'), re.test('7'), re.test('\u0007'));
} catch (e) {
    print(e.name);
}

/*===
true true
===*/

// \0000 is interpreted as U+0000 (\000) followed by literal 0, U+0030.
try {
    re = /[\0000]/;
    print(re.test('\u0000'), re.test('0'));
} catch (e) {
    print(e.name);
}

/*===
false true true false
===*/

// \0007 is interpreted as U+0000 (\000) followed by literal 7, U+0037.
try {
    re = /[\0007]/;
    print(re.test('\u0007'), re.test('7'), re.test('\u0000'), re.test('0'));
} catch (e) {
    print(e.name);
}

/*===
false true true false
false true true false
===*/

// \0008 is interpreted as U+0000 (\000) followed by literal 8, U+0038.
// Similarly for \0009.
try {
    re = /[\0008]/;
    print(re.test('\u0008'), re.test('8'), re.test('\u0000'), re.test('0'));
    re = /[\0009]/;
    print(re.test('\u0009'), re.test('9'), re.test('\u0000'), re.test('0'));
} catch (e) {
    print(e.name);
}

// Some tests from https://github.com/svaarala/duktape/issues/1275 (some overlap).

/*===
true
true
true
false
true
===*/

try {
    re = /(.)(.)(.)[^\3]/;
    print(re.test('xyzx'));
    print(re.test('xyzz'));  // not treated like backref; \3 = 'z'
    print(re.test('xyz\u0002'));
    print(re.test('xyz\u0003'));
    print(re.test('xyz\u0004'));
} catch (e) {
    print(e.name);
}

/*===
true
true
true
false
===*/

try {
    re = /[^\9]/;
    print(re.test('\u0008'));
    print(re.test('\u0009'));
    print(re.test('8'));
    print(re.test('9')); // matches here
} catch (e) {
    print(e.name);
}

/*===
false
true
true
true
===*/

try {
    re = /[^\7]/;
    print(re.test('\u0007'));  // matches here
    print(re.test('\u0008'));
    print(re.test('7'));  // not here
    print(re.test('8'));
} catch (e) {
    print(e.name);
}

/*===
false
true
===*/

try {
    re = /[^\77]/;
    print(re.test('\u003f'));
    print(re.test('\u003e'));
} catch (e) {
    print(e.name);
}

/*===
true
false
===*/

try {
    re = /[^\777]/;
    print(re.test('\u01ff'));
    print(re.test('777'));  // matches here
} catch (e) {
    print(e.name);
}

/*===
true
true
true
true
false
true
false
===*/

try {
    re = /[^\400]/;    // above \0377
    print(re.test('\u0100'));   // doesn't match U+0100 == octal 400
    print(re.test('\u0101'));
    print(re.test('4'));  // not literal '4'
    print(re.test('\u0004'));  // nor \4
    print(re.test('\u0020'));  // matches here: \40
    print(re.test('1'));
    print(re.test('0'));  // 0 also matches so after \40 the remaining zero is a literal
} catch (e) {
    print(e.name);
}

/*===
false
true
===*/

// Accepted also in strict mode.
try {
    eval('(function test() { "use strict"; var re = /[\\7]/; print(re.test("7")); print(re.test("\\u0007")); })();');
} catch (e) {
    print(e.name);
}
