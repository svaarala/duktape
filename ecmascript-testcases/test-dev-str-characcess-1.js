/*
 *  Misc tests for String.fromCharCode(), String.prototype.charAt(), and
 *  String.prototype.charCodeAt().
 */

var text;

/*---
{
    "custom": true
}
---*/

/*===
65
65601
0
0
1
0
0
0
0
===*/

/* ToUint16 coercion means wrapping.  The following coerce to zero:
 * undefined, null, false, NaN, +Inf, -Inf.  A true value coerces to
 * 1.
 */

print(String.fromCharCode(65).charCodeAt(0));

// This is now coerced with ToUint32() by default to better support non-BMP
// strings.  The expected output is non-compliant so testcase is marked custom.
print(String.fromCharCode(65536 + 65).charCodeAt(0));

print(String.fromCharCode(undefined).charCodeAt(0));
print(String.fromCharCode(null).charCodeAt(0));
print(String.fromCharCode(true).charCodeAt(0));
print(String.fromCharCode(false).charCodeAt(0));
print(String.fromCharCode(NaN).charCodeAt(0));
print(String.fromCharCode(Number.POSITIVE_INFINITY).charCodeAt(0));
print(String.fromCharCode(Number.NEGATIVE_INFINITY).charCodeAt(0));

/*===
You can fool all the people some of the time, and some of the people all the time, but you cannot fool all the people all the time.
===*/

/* Building a larger string at once */

var test = "You can fool all the people some of the time, and some of the people all the time, but you cannot fool all the people all the time.";

var codepoints = [];

for (var i in test) {
    codepoints[codepoints.length] = test.charCodeAt(i);
}

print(String.fromCharCode.apply(null, codepoints));

/*===
f
o
o


f
f
f
o
f


string
string
===*/

/* Reading from out of bounds yields an empty string from charAt().
 * Index coercion is ToInteger(), which coerces NaN to zero but
 * leaves infinities as is.  Undefined/null/false coerce to zero,
 * true coerces to 1.
 *
 * Note that this coercion differs from fromCharCode() which uses
 * ToUint16().
 */
text = 'foo';

print(text.charAt(0));
print(text.charAt(1));
print(text.charAt(2));
print(text.charAt(3));
print(text.charAt(-1));
print(text.charAt(undefined));
print(text.charAt(null));
print(text.charAt(false));
print(text.charAt(true));
print(text.charAt(NaN));
print(text.charAt(Number.POSITIVE_INFINITY));  // out of bounds
print(text.charAt(Number.NEGATIVE_INFINITY));  // out of bounds

print(typeof text.charAt(-1));
print(typeof text.charAt(3));

/* FIXME: object coercion */

/*===
98
97
114
NaN
NaN
98
98
98
97
98
NaN
NaN
===*/

/* Similar tests as above, but for charCodeAt().  Coercion is
 * the same.
 */

text = 'bar';

print(text.charCodeAt(0));
print(text.charCodeAt(1));
print(text.charCodeAt(2));
print(text.charCodeAt(3));
print(text.charCodeAt(-1));
print(text.charCodeAt(undefined));
print(text.charCodeAt(null));
print(text.charCodeAt(false));
print(text.charCodeAt(true));
print(text.charCodeAt(NaN));
print(text.charCodeAt(Number.POSITIVE_INFINITY));  // out of bounds
print(text.charCodeAt(Number.NEGATIVE_INFINITY));  // out of bounds

/* FIXME: object coercion */

/*===
0
===*/

/* Test back-and-forth conversion for all 16-bit codepoints. */

var fail = 0;

for (var i = 0; i < 65536; i++) {
    if (String.fromCharCode(i).charCodeAt(0) != i) {
        fail++;
    }
}
print(fail);
