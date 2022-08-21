/*===
- seal plain buffer(0)
[object Uint8Array]
- freeze plain buffer(0)
[object Uint8Array]
- seal plain buffer(1)
[object Uint8Array]
- freeze plain buffer(1)
TypeError
- done
===*/

var u8;

print('- seal plain buffer(0)');
try {
    u8 = Uint8Array.allocPlain(0);
    print(Object.prototype.toString.call(Object.seal(u8)));
} catch (e) {
    print(e.name);
}

print('- freeze plain buffer(0)');
try {
    u8 = Uint8Array.allocPlain(0);
    print(Object.prototype.toString.call(Object.freeze(u8)));
} catch (e) {
    print(e.name);
}

print('- seal plain buffer(1)');
try {
    u8 = Uint8Array.allocPlain(1);
    print(Object.prototype.toString.call(Object.seal(u8)));
} catch (e) {
    print(e.name);
}

print('- freeze plain buffer(1)');
try {
    u8 = Uint8Array.allocPlain(1);
    print(Object.prototype.toString.call(Object.freeze(u8)));
} catch (e) {
    print(e.name);
}

print('- done');
