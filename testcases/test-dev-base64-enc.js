
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

print(__duk__.enc('base64', ''));
print(__duk__.enc('base64', 'f'));
print(__duk__.enc('base64', 'fo'));
print(__duk__.enc('base64', 'foo'));
print(__duk__.enc('base64', 'foob'));
print(__duk__.enc('base64', 'fooba'));
print(__duk__.enc('base64', 'foobar'));

/*===

f
fo
foo
foob
fooba
foobar
===*/

print(__duk__.dec('base64', ''));
print(__duk__.dec('base64', 'Zg=='));
print(__duk__.dec('base64', 'Zm8='));
print(__duk__.dec('base64', 'Zm9v'));
print(__duk__.dec('base64', 'Zm9vYg=='));
print(__duk__.dec('base64', 'Zm9vYmE='));
print(__duk__.dec('base64', 'Zm9vYmFy'));

/*===
Zm9v4Yi0
102 111 111 4660
===*/

/* A string is UTF-8 encoded and then base-64 encoded. */

// U+1234 -> 0xe1 0x88 0xb4
print(__duk__.enc('base64', 'foo\u1234'));

t = '' + __duk__.dec('base64', 'Zm9v4Yi0');
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

t = __duk__.enc('base64', 'f') + __duk__.enc('base64', 'oo');
print(t);
print(__duk__.dec('base64', t));

t = __duk__.enc('base64', 'fo') + __duk__.enc('base64', 'o');
print(t);
print(__duk__.dec('base64', t));

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
    print(__duk__.dec('base64', 'Zg=='));  // standard
    print(__duk__.dec('base64', 'Zh=='));  // non-zero unused bits
} catch (e) {
    print(e.name);
}

try {
    print(__duk__.dec('base64', 'Zm8='));  // standard
    print(__duk__.dec('base64', 'Zm9='));  // non-zero unused bits
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
    print(__duk__.dec('base64', 'eHk='));
} catch(e) {
    print(e.name);
}

try {
    print(__duk__.dec('base64', 'eHk'));
} catch(e) {
    print(e.name);
}

/*===
Zg==
b28=

foo
===*/

/* The current decoder also allows ASCII whitespace characters */

t = __duk__.enc('base64', 'f') + '\n' + __duk__.enc('base64', 'oo') + '\n';
print(t);
print(__duk__.dec('base64', t));

/*===
TypeError
===*/

/* Non-base64 characters will not be accepted */

try {
    print(__duk__.dec('b28?'));
} catch (e) {
    print(e.name);
}

