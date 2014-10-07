// XXX: util
var WHITESPACE_CODEPOINTS = [
    // from WhiteSpace production
    0x0009,    // <TAB>
    0x000B,    // <VT>
    0x000C,    // <FF>
    0x0020,    // <SP>
    0x00A0,    // <NBSP>
    0xFEFF,    // <BOM>

    // WhiteSpace production also has <USP>, which means any other Unicode
    // space separator (category Zs), which needs to be checked from (up to
    // date) Unicode data.  The WhiteSpace-Z.txt file, created as part of
    // the build, currently contains (duplicates eliminated):

//  0x0020,    // 0020;SPACE;Zs;0;WS;;;;;N;;;;;
//  0x00A0,    // 00A0;NO-BREAK SPACE;Zs;0;CS;<noBreak> 0020;;;;N;NON-BREAKING SPACE;;;;
    0x1680,    // 1680;OGHAM SPACE MARK;Zs;0;WS;;;;;N;;;;;
    0x180E,    // 180E;MONGOLIAN VOWEL SEPARATOR;Zs;0;WS;;;;;N;;;;;
    0x2000,    // 2000;EN QUAD;Zs;0;WS;2002;;;;N;;;;;
    0x2001,    // 2001;EM QUAD;Zs;0;WS;2003;;;;N;;;;;
    0x2002,    // 2002;EN SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x2003,    // 2003;EM SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x2004,    // 2004;THREE-PER-EM SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x2005,    // 2005;FOUR-PER-EM SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x2006,    // 2006;SIX-PER-EM SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x2007,    // 2007;FIGURE SPACE;Zs;0;WS;<noBreak> 0020;;;;N;;;;;
    0x2008,    // 2008;PUNCTUATION SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x2009,    // 2009;THIN SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x200A,    // 200A;HAIR SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
//  0x2028,    // 2028;LINE SEPARATOR;Zl;0;WS;;;;;N;;;;;
//  0x2029,    // 2029;PARAGRAPH SEPARATOR;Zp;0;B;;;;;N;;;;;
    0x202F,    // 202F;NARROW NO-BREAK SPACE;Zs;0;CS;<noBreak> 0020;;;;N;;;;;
    0x205F,    // 205F;MEDIUM MATHEMATICAL SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x3000,    // 3000;IDEOGRAPHIC SPACE;Zs;0;WS;<wide> 0020;;;;N;;;;;

    // from LineTerminator production
    0x000a,    // <LF>
    0x000d,    // <CR>
    0x2028,    // <LS>
    0x2029,    // <PS>
];

/*===
basic
string 3 foo
string 3 foo
string 3 foo
string 3 foo
===*/

print('basic');

function basicTest() {
    var all_ws;
    var i;

    function test(str) {
        var t;

        t = str.trim();
        print(typeof t, t.length, t);
    }

    all_ws = [];
    for (i = 0; i < WHITESPACE_CODEPOINTS; i++) {
        all_ws.push(String.fromCharCode(WHITESPACE_CODEPOINTS[i]));
    }
    all_ws = all_ws.join('');

    test('foo');
    test('  foo');
    test('foo  ');
    test(all_ws + 'foo' + all_ws);
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
coercion
TypeError
TypeError
TypeError
string 4 true
string 5 false
string 3 123
string 3 foo
string 3 1,2
string 15 [object Object]
===*/

print('coercion');

function coercionTest() {
    function test(this_val, arg_count) {
        var t;

        try {
            if (arg_count == 0) {
                t = String.prototype.trim.call();
            } else {
                t = String.prototype.trim.call(this_val);
            }
            print(typeof t, t.length, t);
        } catch (e) {
            print(e.name);
        }
    }

    test(undefined, 0);
    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('foo');
    test([1,2]);
    test({ foo: 1, bar: 2 });
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
