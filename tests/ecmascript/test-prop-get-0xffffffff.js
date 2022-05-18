/*===
foo
undefined
foo
bar
undefined
bar
quux
undefined
quux
===*/

try {
    var obj = { [2**32-2]: 'foo', [2**32-1]: 'bar', [2**32]: 'quux' };
    print(obj[0xfffffffe]);
    print(obj['0xfffffffe']);
    print(obj['4294967294']);
    print(obj[0xffffffff]);
    print(obj['0xffffffff']);
    print(obj['4294967295']);
    print(obj[0x100000000]);
    print(obj['0x100000000']);
    print(obj['4294967296']);
} catch (e) {
    print(e.stack || e);
}
