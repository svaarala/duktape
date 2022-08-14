/*
 *  String.prototype.repeat() for an internal string just concatenates the
 *  internal representations.  With WTF-8 sanitization all byte sequences
 *  ultimately treated as strings are in valid WTF-8 so concatenation cannot
 *  create e.g. symbol values.  However, at the joint between segments
 *  invalid surrogates may pair up to generate a valid surrogate pair which
 *  is combined in the result.
 */

/*@include util-buffer.js@*/
/*@include util-string.js@*/

/*---
custom: true
---*/

/*===
4
"<U+FFFD>ABC"
15
"<U+DC12>foo<U+D834><U+DC12>foo<U+D834><U+DC12>foo<U+D834>"
|edb092666f6ff09d8092666f6ff09d8092666f6feda0b4|
===*/

function test() {
    var str, res;

    // Internal prefix 0xFE is replaced by U+FFFD and is repeated, unaffected.
    str = bufferToStringRaw(new Uint8Array([ 0xfe, 0x41, 0x42, 0x43 ]));
    print(str.length);
    safePrintString(str);

    // Surrogates may pair up at join point.
    str = '\udc12foo\ud834';
    res = str.repeat(3);
    print(res.length);
    safePrintString(res);
    print(Duktape.enc('jx', Uint8Array.allocPlain(res)));
}

test();
