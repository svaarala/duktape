/*
 *  Ecmascript specification requires that U+2028 and U+2029 are not escaped
 *  by JSON.stringify().  Because these characters are considered line breaks
 *  by Ecmascript, the resulting JSON won't work when pasted e.g. into a web
 *  page.
 *
 *  Duktape escapes these codepoints to work better with e.g. web templating.
 *  This behavior is non-compliant, the compliant behavior can be forced with
 *  an option.
 *
 *  Reported by 'malord', see GH-68.
 */

/*---
{
    "nonstandard": true
}
---*/

/*===
0 34 "
1 102 f
2 111 o
3 111 o
4 92 \
5 117 u
6 50 2
7 48 0
8 50 2
9 56 8
10 92 \
11 117 u
12 50 2
13 48 0
14 50 2
15 57 9
16 98 b
17 97 a
18 114 r
19 34 "
7
7
===*/

function u2028u2029test() {
    var res = JSON.stringify("foo\u2028\u2029bar");
    var i, n, c;

    for (i = 0, n = res.length; i < n; i++) {
        c = res.charCodeAt(i);
        print(i, c, c >= 0x20 && c <= 0x7e ? String.fromCharCode(c) : 'non-ascii');
    }

    /* The standard behavior is interesting in that if you serialize a
     * string containing U+2028 or U+2029, you won't be able to eval it
     * back in because the characters are considered LineTerminators and
     * a string literal is considered to be unterminated.
     */

    try {
        print(eval(JSON.stringify('foo\u2028bar')).length);
    } catch (e) {
        print(e.name);
    }

    try {
        print(eval(JSON.stringify('foo\u2028bar')).length);
    } catch (e) {
        print(e.name);
    }
}

try {
    u2028u2029test();
} catch (e) {
    print(e.stack || e);
}
