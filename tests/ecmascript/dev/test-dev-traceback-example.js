/*---
custom: true
---*/

/*===
URIError
invalid input
string
true
true
===*/

try {
    decodeURIComponent('%e1%a9%01');  // invalid utf-8
} catch (e) {
    print(e.name);
    print(e.message);
    print(typeof e.stack);
    print(e.stack.startsWith('URIError: invalid input'));
    print(e.stack.indexOf('\n') >= 0);
}
