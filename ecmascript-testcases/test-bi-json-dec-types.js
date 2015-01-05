/*===
typeof, class, etc
typeof: object
class: Null
typeof: boolean
class: Boolean
typeof: boolean
class: Boolean
typeof: number
class: Number
typeof: object
class: Array
typeof: object
class: Object
array prototype expected: true
object prototype expected: true
===*/

/* Test typeof, class etc of parsed values. */

print('typeof, class, etc');

function getClass(v) {
    var txt;

    txt = Object.prototype.toString.call(v);  /* -> "[object <class>]" */
    m = /^\[object\u0020(.*)\]$/.exec(txt);
    if (m) {
        return m[1];
    } else {
        return;
    }
}

function testTypeofEtc(x) {
    var v = JSON.parse(x);
    print('typeof: ' + typeof v);
    print('class: ' + getClass(v));
}

function testObjectAndArrayPrototypes() {
    var v;

    /* Both Object.prototype and Array.prototype are non-writable,
     * non-configurable properties, so we can't test (and don't need
     * to test) what happens if Object.prototype or Array.prototype
     * is replaced with a custom value.
     */

    v = JSON.parse("[1,2]");
    print('array prototype expected: ' + (Object.getPrototypeOf(v) === Array.prototype));

    v = JSON.parse('{"foo":1,"bar":2}');
    print('object prototype expected: ' + (Object.getPrototypeOf(v) === Object.prototype));
}

try {
    testTypeofEtc("null");
    testTypeofEtc("true");
    testTypeofEtc("false");
    testTypeofEtc("1.23");
    testTypeofEtc("[1,2]");
    testTypeofEtc('{"foo":1,"bar":2}');

    // test Object and Array prototypes
    testObjectAndArrayPrototypes();
} catch (e) {
    print(e.name);
}

/*===
nan/inf
SyntaxError
SyntaxError
SyntaxError
===*/

/* JSONNumber does not accept NaN, Infinity, -Infinity.  Ecmascript also
 * does not but they parse as identifier references (and unary minus for
 * -Infinity) and usually evaluate to something useful.
 *
 * These are tested explicitly because they might be a risk with shared
 * parser handling and because these forms might be used in an extended
 * JSON encoding format.
 */

print('nan/inf');

try {
    print(JSON.parse('NaN'));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse('Infinity'));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse('-Infinity'));
} catch (e) {
    print(e.name);
}

/*===
hex/octal literal
SyntaxError
SyntaxError
SyntaxError
===*/

/* JSONNumber does not allow hex or octal literals. */

print('hex/octal literal');

try {
    print(JSON.parse('0x41'));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse('077'));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse('099'));
} catch (e) {
    print(e.name);
}

/*===
leading plus
1
SyntaxError
===*/

/* JSONNumber does not allow a plus sign.  Actually Ecmascript doesn't either,
 * a leading plus sign is an unary plus operator.
 */

print('leading plus');

try {
    print(eval('+1'));
    print(JSON.parse('+1'));
} catch (e) {
    print(e.name);
}

/*===
empty fractions
1
SyntaxError
===*/

/* JSONNumber does not allow empty fractions part.
 *
 * (Rhino allows it.)
 */

print('empty fractions');

try {
    print(eval('1.'));
    print(JSON.parse('1.'));
} catch (e) {
    print(e.name);
}

/*===
missing integer part
0.123
SyntaxError
===*/

/* JSONNumber does not allow fractions without an integer part. */

print('missing integer part');

try {
    print(eval('.123'));
    print(JSON.parse('.123'));
} catch (e) {
    print(e.name);
}

/*===
leading zeroes
0
SyntaxError
SyntaxError
SyntaxError
===*/

/* JSONNumber does not allow additional leading zeroes.  This is probably a
 * good thing even if Ecmascript parsing allows octal literals.
 */

print('leading zeroes');

try {
    print(JSON.parse('0'));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse('00'));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse('012'));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse('019'));
} catch (e) {
    print(e.name);
}

/*===
single quotes
foo
SyntaxError
===*/

/* JSONString does not allow single quotes, Ecmascript does. */

print('single quotes');

try {
    print(eval("'foo'"));
    print(JSON.parse("'foo'"));
} catch (e) {
    print(e.name);
}

/*===
string codepoints
cp 0
eval 1 0
json SyntaxError
cp 1
eval 1 1
json SyntaxError
cp 2
eval 1 2
json SyntaxError
cp 3
eval 1 3
json SyntaxError
cp 4
eval 1 4
json SyntaxError
cp 5
eval 1 5
json SyntaxError
cp 6
eval 1 6
json SyntaxError
cp 7
eval 1 7
json SyntaxError
cp 8
eval 1 8
json SyntaxError
cp 9
eval 1 9
json SyntaxError
cp 10
eval SyntaxError
json SyntaxError
cp 11
eval 1 11
json SyntaxError
cp 12
eval 1 12
json SyntaxError
cp 13
eval SyntaxError
json SyntaxError
cp 14
eval 1 14
json SyntaxError
cp 15
eval 1 15
json SyntaxError
cp 16
eval 1 16
json SyntaxError
cp 17
eval 1 17
json SyntaxError
cp 18
eval 1 18
json SyntaxError
cp 19
eval 1 19
json SyntaxError
cp 20
eval 1 20
json SyntaxError
cp 21
eval 1 21
json SyntaxError
cp 22
eval 1 22
json SyntaxError
cp 23
eval 1 23
json SyntaxError
cp 24
eval 1 24
json SyntaxError
cp 25
eval 1 25
json SyntaxError
cp 26
eval 1 26
json SyntaxError
cp 27
eval 1 27
json SyntaxError
cp 28
eval 1 28
json SyntaxError
cp 29
eval 1 29
json SyntaxError
cp 30
eval 1 30
json SyntaxError
cp 31
eval 1 31
json SyntaxError
===*/

/* JSONString does not accept codepoints U+0000 to U+001F (Ecmascript does). */

print('string codepoints');

function testStringCodepoint(x) {
    var lit = '"' + String.fromCharCode(x) + '"';
    var t;

    print('cp', x);

    // Codepoints 0x0a and 0x0d will cause eval to fail: newline inside
    // a string
    try {
        t = eval(lit);
        print('eval', t.length, t.charCodeAt(0));
    } catch (e) {
        print('eval', e.name);
    }

    try {
        t = JSON.parse(lit);
        print('json', t.length, t.charCodeAt(0));
    } catch (e) {
        print('json', e.name);
    }
}

function testStringCodepoints() {
    var i;

    for (i = 0; i < 32; i++) {
        testStringCodepoint(i);
    }
}

try {
    testStringCodepoints();
} catch (e) {
    print(e.name, e);
}

/*===
unicode escape
1 4660
2 43981 61185
2 43981 61185
===*/

print('unicode escape');

function testUnicodeEscape() {
    var t;

    t = JSON.parse('"\\u1234"');
    print(t.length, t.charCodeAt(0));

    t = JSON.parse('"\\uabcd\\uef01"');
    print(t.length, t.charCodeAt(0), t.charCodeAt(1));

    // both lowercase and uppercase hex digits are required
    t = JSON.parse('"\\uABCD\\uEF01"');
    print(t.length, t.charCodeAt(0), t.charCodeAt(1));
}

try {
    testUnicodeEscape();
} catch (e) {
    print(e);
}

/*===
character escapes
cp 97 a
eval 1 97
json SyntaxError
cp 98 b
eval 1 8
json 1 8
cp 99 c
eval 1 99
json SyntaxError
cp 100 d
eval 1 100
json SyntaxError
cp 101 e
eval 1 101
json SyntaxError
cp 102 f
eval 1 12
json 1 12
cp 103 g
eval 1 103
json SyntaxError
cp 104 h
eval 1 104
json SyntaxError
cp 105 i
eval 1 105
json SyntaxError
cp 106 j
eval 1 106
json SyntaxError
cp 107 k
eval 1 107
json SyntaxError
cp 108 l
eval 1 108
json SyntaxError
cp 109 m
eval 1 109
json SyntaxError
cp 110 n
eval 1 10
json 1 10
cp 111 o
eval 1 111
json SyntaxError
cp 112 p
eval 1 112
json SyntaxError
cp 113 q
eval 1 113
json SyntaxError
cp 114 r
eval 1 13
json 1 13
cp 115 s
eval 1 115
json SyntaxError
cp 116 t
eval 1 9
json 1 9
cp 117 u
eval 1 117
json SyntaxError
cp 118 v
eval 1 11
json SyntaxError
cp 119 w
eval 1 119
json SyntaxError
cp 120 x
eval 1 120
json SyntaxError
cp 121 y
eval 1 121
json SyntaxError
cp 122 z
eval 1 122
json SyntaxError
===*/

/* JSONString does not allow "unknown" backslash escapes.  These are handled
 * by the Ecmascript grammar as "no-ops", e.g. "\d" === "d" but must be rejected
 * by JSON parsing.
 */

print('character escapes');

function testCharacterEscape(x) {
    var lit = '"' + '\\' + String.fromCharCode(x) + '"';
    var t;

    print('cp', x, String.fromCharCode(x));

    // Escape chars 'b', 'f', 'n', 't', 'v' will decode to special characters.
    // Unterminated hex/unicode escapes ('x' and 'u') are decoded as unknown
    // escapes and *don't* cause a SyntaxError, e.g. "\\u" evals to "u".
    try {
        t = eval(lit);
        print('eval', t.length, t.charCodeAt(0));
    } catch (e) {
        print('eval', e.name);
    }

    try {
        t = JSON.parse(lit);
        print('json', t.length, t.charCodeAt(0));
    } catch (e) {
        print('json', e.name);
    }
}

function testCharacterEscapes() {
    var i;
    var cp;

    for (i = 0; i < 26; i++) {
        cp = 97 + i;   /* a to z */
        testCharacterEscape(cp);
    }
}

try {
    testCharacterEscapes();
} catch (e) {
    print(e.name);
}

/*===
zero escape
0
SyntaxError
===*/

/* Ecmascript strings accept a zero escape, JSONString does not. */

print('zero escape');

try {
    print(eval('"\\0"').charCodeAt(0));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse('"\\0"').charCodeAt(0));
} catch (e) {
    print(e.name);
}

/*===
forward slash escape
/
===*/

/* JSONString accepts forward slash escape (although JSON.stringify() never
 * produces it).
 */

print('forward slash escape');

try {
    print(JSON.parse('"\\/"'));
} catch (e) {
    print(e.name);
}

/*===
hex escape
A
SyntaxError
===*/

/* JSONString does not accept hex escapes, Ecmascript strings do. */

print('hex escape');

try {
    print(eval('"\\x41"'));
    print(JSON.parse('"\\x41"'));
} catch (e) {
    print(e.name);
}

/*===
null literal
null
===*/

print('null literal');

try {
    print(JSON.parse('null'));
} catch (e) {
    print(e.name);
}

/*===
boolean literal
true
false
===*/

print('boolean literal');

try {
    print(JSON.parse('true'));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse('false'));
} catch (e) {
    print(e.name);
}

/*===
tostring coercion of true/false/null
true
false
null
===*/

/* parse() argument is ToString() coerced, so these will parse back;
 * e.g. true -> "true" -> true.
 */

print('tostring coercion of true/false/null');

try {
    print(JSON.parse(true));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse(false));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse(null));
} catch (e) {
    print(e.name);
}
