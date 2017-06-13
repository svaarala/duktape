/*
 *  Math.clz32()
 */

/*===
function true false true
1
32
0
32
32
25
32
32
32
32
valueOf 1
25
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
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
32
===*/

function test() {
    var pd = Object.getOwnPropertyDescriptor(Math, 'clz32');
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);
    print(Math.clz32.length);

    [ -1/0, -123.1, -0, +0, 123.1, 1/0,
      0/0, '', 'x',
      { valueOf: function () { print('valueOf 1'); return 123 },
        toString: function () { print('toString 1'); return -321; } },

      // single bits
      0x80000000,
      0x40000000,
      0x20000000,
      0x10000000,
      0x08000000,
      0x04000000,
      0x02000000,
      0x01000000,
      0x00800000,
      0x00400000,
      0x00200000,
      0x00100000,
      0x00080000,
      0x00040000,
      0x00020000,
      0x00010000,
      0x00008000,
      0x00004000,
      0x00002000,
      0x00001000,
      0x00000800,
      0x00000400,
      0x00000200,
      0x00000100,
      0x00000080,
      0x00000040,
      0x00000020,
      0x00000010,
      0x00000008,
      0x00000004,
      0x00000002,
      0x00000001,
      0
    ].forEach(function (v, i) {
        x = Math.clz32(v);
        print(x);
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
