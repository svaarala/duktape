
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

