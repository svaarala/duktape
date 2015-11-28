/*
 *  Some base-64 tests.
 *
 *  Note: Duktape.dec() results are coerced from buffer to string
 *  with "'' + Duktape.dec(...)" because print() won't add an
 *  automatic newline if print() has only one argument which is
 *  a buffer.
 */

/*---
{
    "custom": true
}
---*/

var t;

/*===

Zg==
Zm8=
Zm9v
Zm9vYg==
Zm9vYmE=
Zm9vYmFy
===*/

print(Duktape.enc('base64', ''));
print(Duktape.enc('base64', 'f'));
print(Duktape.enc('base64', 'fo'));
print(Duktape.enc('base64', 'foo'));
print(Duktape.enc('base64', 'foob'));
print(Duktape.enc('base64', 'fooba'));
print(Duktape.enc('base64', 'foobar'));

/*===

f
fo
foo
foob
fooba
foobar
===*/

print('' + Duktape.dec('base64', ''));
print('' + Duktape.dec('base64', 'Zg=='));
print('' + Duktape.dec('base64', 'Zm8='));
print('' + Duktape.dec('base64', 'Zm9v'));
print('' + Duktape.dec('base64', 'Zm9vYg=='));
print('' + Duktape.dec('base64', 'Zm9vYmE='));
print('' + Duktape.dec('base64', 'Zm9vYmFy'));

/*===
Zm9v4Yi0
102 111 111 4660
===*/

/* A string is UTF-8 encoded and then base-64 encoded. */

// U+1234 -> 0xe1 0x88 0xb4
print(Duktape.enc('base64', 'foo\u1234'));

t = '' + Duktape.dec('base64', 'Zm9v4Yi0');
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
print('' + Duktape.dec('base64', t));

t = Duktape.enc('base64', 'fo') + Duktape.enc('base64', 'o');
print(t);
print('' + Duktape.dec('base64', t));

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
print('' + Duktape.dec('base64', t));

print('' + Duktape.dec('base64', 'Zm9vYmFycXV1eA==\n'));
print('' + Duktape.dec('base64', 'Z m\t9\rv\nY  \r\n\t m  \n\n FycX \r\r\nV1eA =\t\t=\n'));

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
    print('' + Duktape.dec('base64', 'Zg=='));  // standard
    print('' + Duktape.dec('base64', 'Zh=='));  // non-zero unused bits
} catch (e) {
    print(e.name);
}

try {
    print('' + Duktape.dec('base64', 'Zm8='));  // standard
    print('' + Duktape.dec('base64', 'Zm9='));  // non-zero unused bits
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
    print('' + Duktape.dec('base64', 'eHk='));
} catch(e) {
    print(e.name);
}

try {
    print('' + Duktape.dec('base64', 'eHk'));
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
print('' + Duktape.dec('base64', t));

/*===
TypeError
===*/

/* Non-base64 characters will not be accepted */

try {
    print('' + Duktape.dec('base64', 'b28?'));
} catch (e) {
    print(e.name);
}
