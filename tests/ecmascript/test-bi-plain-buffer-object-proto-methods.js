/*@include util-buffer.js@*/

/*===
Object.prototype methods
- toString
[object Uint8Array]
- toLocaleString
[object Uint8Array]
- valueOf
[object Uint8Array] false
- hasOwnProperty
0 true
1 true
2 true
3 true
4 true
5 true
6 true
7 true
8 true
9 true
10 true
11 true
12 true
13 true
14 true
15 true
16 false
length true
byteLength false
byteOffset false
BYTES_PER_ELEMENT false
buffer false
noSuch false
- isPrototypeOf
false
false
- propertyIsEnumerable
false
true
true
false
===*/

function objectPrototypeMethodTest() {
    var pb, t;

    function resetValues() {
        pb = createPlainBuffer('abcdefghijklmnop');
    }

    resetValues();
    print('- toString');
    print(pb.toString());

    resetValues();
    print('- toLocaleString');
    print(pb.toLocaleString());

    resetValues();
    print('- valueOf');
    print(pb.valueOf(), pb.valueOf() === pb);  // .valueOf() is Object() coerced now

    resetValues();
    print('- hasOwnProperty');
    [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
      'length', 'byteLength', 'byteOffset', 'BYTES_PER_ELEMENT', 'buffer', 'noSuch' ].forEach(function (k) {
        print(k, pb.hasOwnProperty(k));
    });

    resetValues();
    print('- isPrototypeOf');
    print(pb.isPrototypeOf(pb));  // false, no object is its own prototype (unless there's a prototype loop)
    t = Object.create(pb);
    print(pb.isPrototypeOf(t));   // false; Object.create() object coerces plain buffer when using it as prototype

    resetValues();
    print('- propertyIsEnumerable');
    print(pb.propertyIsEnumerable('byteLength'));
    print(pb.propertyIsEnumerable(3));
    print(pb.propertyIsEnumerable('3'));
    print(pb.propertyIsEnumerable('noSuch'));
}

try {
    print('Object.prototype methods');
    objectPrototypeMethodTest();
} catch (e) {
    print(e.stack || e);
}
