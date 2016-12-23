/*@include util-buffer.js@*/

/*===
enumeration test
for-in plain
0
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
myEnumerable
Object.keys plain
0
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
Object.getOwnPropertyNames plain
0
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
length
===*/

function enumerationTest() {
    var pb = createPlainBuffer('0123456789abcdef');
    var k;

    // Add enumerable inherited property.

    Uint8Array.prototype.myEnumerable = 1;
    print('for-in plain');
    for (k in pb) {
        print(k);
    }
    delete Uint8Array.prototype.myEnumerable;

    // Inherited properties not included.

    Uint8Array.prototype.myEnumerable = 1;
    print('Object.keys plain');
    Object.keys(pb).forEach(function (k) {
        print(k);
    });
    delete Uint8Array.prototype.myEnumerable;

    // Object.getOwnPropertyNames() will include all own properties,
    // including non-enumerable ones.  Shows .buffer property due to
    // object coercion.

    print('Object.getOwnPropertyNames plain');
    Object.getOwnPropertyNames(pb).forEach(function (k) {
        print(k);
    });
}

try {
    print('enumeration test');
    enumerationTest();
} catch (e) {
    print(e.stack || e);
}
