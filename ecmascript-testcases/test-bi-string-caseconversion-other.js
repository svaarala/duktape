/*
 *  Case conversion tests for cases not covered by
 *
 *      test-bi-string-caseconversion-single.js
 *
 *  Specifically, context and locale specific conversion rules.
 */

/* XXX: expand when locale/context rules are improved. */

/*===
input: 0046 006f 006f
upper: 0046 004f 004f
lower: 0066 006f 006f
input: 0078 03a3 002e
upper: 0058 03a3 002e
lower: 0078 03c2 002e
input: 0078 03a3
upper: 0058 03a3
lower: 0078 03c2
input: 002e 03a3 002e
upper: 002e 03a3 002e
lower: 002e 03c3 002e
input: 002e 03a3
upper: 002e 03a3
lower: 002e 03c3
===*/

function dump(x) {
    var i;
    var a = [];
    var t;

    for (i = 0; i < x.length; i++) {
        t = x.charCodeAt(i).toString(16);
        while (t.length < 4) { t = '0' + t; }
        a.push(t);
    }

    return a.join(' ');
}

function conv(x) {
    print('input:', dump(x));
    print('upper:', dump(x.toUpperCase()));
    print('lower:', dump(x.toLowerCase()));
}

function test() {
    // Sanity
    conv('Foo');

    // Greek final sigma lowercasing rule
    conv('x\u03a3.');   // prev: letter, curr=greek capital letter sigma, next=non-letter/non-existentZ
    conv('x\u03a3');
    conv('.\u03a3.');   // prev is not a letter, rule not active
    conv('.\u03a3');    // same
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
