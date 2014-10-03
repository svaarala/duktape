/*---
{
    "custom": true
}
---*/

var t;

/*===
666f6fe188b4
102 111 111 4660
===*/

print(Duktape.enc('hex', 'foo\u1234'));

t = '' + Duktape.dec('hex', '666f6fe188b4');
print(t.charCodeAt(0), t.charCodeAt(1), t.charCodeAt(2), t.charCodeAt(3));
