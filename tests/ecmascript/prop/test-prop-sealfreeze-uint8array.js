/*===
- seal Uint8Array(0)
[object Uint8Array]
- freeze Uint8Array(0)
[object Uint8Array]
- seal Uint8Array(1)
TypeError
- freeze Uint8Array(1)
TypeError
- done
===*/

var u8;

print('- seal Uint8Array(0)');
try {
    u8 = new Uint8Array(0);
    print(Object.prototype.toString.call(Object.seal(u8)));
} catch (e) {
    print(e.name);
}

print('- freeze Uint8Array(0)');
try {
    u8 = new Uint8Array(0);
    print(Object.prototype.toString.call(Object.freeze(u8)));
} catch (e) {
    print(e.name);
}

print('- seal Uint8Array(1)');
try {
    u8 = new Uint8Array(1);
    print(Object.prototype.toString.call(Object.seal(u8)));
} catch (e) {
    print(e.name);
}

print('- freeze Uint8Array(1)');
try {
    u8 = new Uint8Array(1);
    print(Object.prototype.toString.call(Object.freeze(u8)));
} catch (e) {
    print(e.name);
}

print('- done');
