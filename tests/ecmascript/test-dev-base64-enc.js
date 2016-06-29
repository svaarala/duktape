/*
 *  Some base-64 tests.
 *
 *  Note: Duktape.dec() results are coerced from buffer to string
 *  with "'' + Duktape.dec(...)" because print() won't add an
 *  automatic newline if print() has only one argument which is
 *  a buffer.
 */

/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

var t;

function encPrint(x) {
    print(Duktape.enc('base64', x));
}

function decPrint(x) {
    print(bufferToString(Duktape.dec('base64', x)));
}

/*===

Zg==
Zm8=
Zm9v
Zm9vYg==
Zm9vYmE=
Zm9vYmFy
===*/

encPrint('');
encPrint('f');
encPrint('fo');
encPrint('foo');
encPrint('foob');
encPrint('fooba');
encPrint('foobar');

/*===

f
fo
foo
foob
fooba
foobar
===*/

decPrint('');
decPrint('Zg==');
decPrint('Zm8=');
decPrint('Zm9v');
decPrint('Zm9vYg==');
decPrint('Zm9vYmE=');
decPrint('Zm9vYmFy');

/*===
Zm9v4Yi0
102 111 111 4660
===*/

/* A string is UTF-8 encoded and then base-64 encoded. */

// U+1234 -> 0xe1 0x88 0xb4
encPrint('foo\u1234');

t = bufferToString(Duktape.dec('base64', 'Zm9v4Yi0'));
print(t.charCodeAt(0), t.charCodeAt(1), t.charCodeAt(2), t.charCodeAt(3));

/*===
Zg==b28=
foo
Zm8=bw==
foo
===*/

/* The current decoder is lenient in that it allows concatenated base64
 * documents to be decoded (even when there is intervening padding).
 */

t = Duktape.enc('base64', 'f') + Duktape.enc('base64', 'oo');
print(t);
decPrint(t);

t = Duktape.enc('base64', 'fo') + Duktape.enc('base64', 'o');
print(t);
decPrint(t);

/*===
Zm9vYmFycXV1eA==
foobarquux
foobarquux
foobarquux
===*/

/* The current decoder allows ASCII whitespace (CR, LF, TAB, SPACE) at
 * any position.
 */

t = Duktape.enc('base64', 'foobarquux');
print(t);
decPrint(t);

decPrint('Zm9vYmFycXV1eA==\n');
decPrint('Z m\t9\rv\nY  \r\n\t m  \n\n FycX \r\r\nV1eA =\t\t=\n');

/*===
f
f
fo
fo
===*/

/* The current decoder will not check that unused bits of a partial
 * group are actually zero.  This matches e.g. Python behavior.
 */

try {
    decPrint('Zg==');  // standard
    decPrint('Zh==');  // non-zero unused bits
} catch (e) {
    print(e.name);
}

try {
    decPrint('Zm8=');  // standard
    decPrint('Zm9=');  // non-zero unused bits
} catch (e) {
    print(e.name);
}


/*===
xy
TypeError
===*/

/* Base64 strings without end padding are not accepted now, although
 * they could be decoded unambiguously.  For instance 'xy' encodes
 * into 'eHk=' which is unambiguously decodable from 'eHk'.
 */

try {
    decPrint('eHk=');
} catch(e) {
    print(e.name);
}

try {
    decPrint('eHk');
} catch(e) {
    print(e.name);
}

/*===
Zg==
b28=

foo
===*/

/* The current decoder also allows ASCII whitespace characters */

t = Duktape.enc('base64', 'f') + '\n' + Duktape.enc('base64', 'oo') + '\n';
print(t);
decPrint(t);

/*===
TypeError
===*/

/* Non-base64 characters will not be accepted */

try {
    decPrint('b28?');
} catch (e) {
    print(e.name);
}
