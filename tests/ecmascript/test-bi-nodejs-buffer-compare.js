/*
 *  Buffer.compare() and buf.compare()
 */

/*@include util-nodejs-buffer.js@*/

/*===
Node.js Buffer.compare() test
0 bytes: 
0 bytes: 
4 bytes: deadbeef
4 bytes: deadbeef
4 bytes: deadbe99
8 bytes: deadbeefcafebabe
8 bytes: deadbeefcafebabe
8 bytes: deadbeefcafeba99
dummy value
dummy value
0 0 0
0 1 0
0 2 -1
0 3 -1
0 4 -1
0 5 -1
0 6 -1
0 7 -1
0 8 TypeError
0 9 TypeError
1 0 0
1 1 0
1 2 -1
1 3 -1
1 4 -1
1 5 -1
1 6 -1
1 7 -1
1 8 TypeError
1 9 TypeError
2 0 1
2 1 1
2 2 0
2 3 0
2 4 1
2 5 -1
2 6 -1
2 7 -1
2 8 TypeError
2 9 TypeError
3 0 1
3 1 1
3 2 0
3 3 0
3 4 1
3 5 -1
3 6 -1
3 7 -1
3 8 TypeError
3 9 TypeError
4 0 1
4 1 1
4 2 -1
4 3 -1
4 4 0
4 5 -1
4 6 -1
4 7 -1
4 8 TypeError
4 9 TypeError
5 0 1
5 1 1
5 2 1
5 3 1
5 4 1
5 5 0
5 6 0
5 7 1
5 8 TypeError
5 9 TypeError
6 0 1
6 1 1
6 2 1
6 3 1
6 4 1
6 5 0
6 6 0
6 7 1
6 8 TypeError
6 9 TypeError
7 0 1
7 1 1
7 2 1
7 3 1
7 4 1
7 5 -1
7 6 -1
7 7 0
7 8 TypeError
7 9 TypeError
8 0 TypeError
8 1 TypeError
8 2 TypeError
8 3 TypeError
8 4 TypeError
8 5 TypeError
8 6 TypeError
8 7 TypeError
8 8 TypeError
8 9 TypeError
9 0 TypeError
9 1 TypeError
9 2 TypeError
9 3 TypeError
9 4 TypeError
9 5 TypeError
9 6 TypeError
9 7 TypeError
9 8 TypeError
9 9 TypeError
===*/

function compareTest() {
    var b1a = new Buffer(0);
    var b1b = new Buffer(0);
    var b2a = new Buffer(4);
    var b2b = new Buffer(4);
    var b2c = new Buffer(4);
    var b3a = new Buffer(8);
    var b3b = new Buffer(8);
    var b3c = new Buffer(8);

    // b2a and b2b have same bytes, b2c is different

    b2a[0] = 0xde; b2a[1] = 0xad; b2a[2] = 0xbe; b2a[3] = 0xef;
    b2b[0] = 0xde; b2b[1] = 0xad; b2b[2] = 0xbe; b2b[3] = 0xef;
    b2c[0] = 0xde; b2c[1] = 0xad; b2c[2] = 0xbe; b2c[3] = 0x99;

    // b3a and b3b have same bytes, and same 4 byte prefix as b2a/b2b.
    // b3c is different.

    b3a[0] = 0xde; b3a[1] = 0xad; b3a[2] = 0xbe; b3a[3] = 0xef;
    b3a[4] = 0xca; b3a[5] = 0xfe; b3a[6] = 0xba; b3a[7] = 0xbe;

    b3b[0] = 0xde; b3b[1] = 0xad; b3b[2] = 0xbe; b3b[3] = 0xef;
    b3b[4] = 0xca; b3b[5] = 0xfe; b3b[6] = 0xba; b3b[7] = 0xbe;

    b3c[0] = 0xde; b3c[1] = 0xad; b3c[2] = 0xbe; b3c[3] = 0xef;
    b3c[4] = 0xca; b3c[5] = 0xfe; b3c[6] = 0xba; b3c[7] = 0x99;

    var buffers = [ b1a, b1b, b2a, b2b, b2c, b3a, b3b, b3c, 'foo', undefined ];

    buffers.forEach(function (b) {
        if (!Buffer.isBuffer(b)) { print('dummy value'); return; }
        printNodejsBuffer(b);
    });

    buffers.forEach(function (b1, i) {
        buffers.forEach(function (b2, j) {
            try {
                if (b2 === undefined) {
                    print(i, j, Buffer.compare(b1));
                } else {
                    print(i, j, Buffer.compare(b1, b2));
                }
            } catch (e) {
                // TypeError for non-buffer arguments
                print(i, j, e.name);
            }
        });
    });
}

try {
    print('Node.js Buffer.compare() test');
    compareTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Node.js Buffer instance compare() test
0 0 TypeError
0 1 TypeError
0 2 TypeError
0 3 TypeError
0 4 TypeError
0 5 TypeError
0 6 TypeError
0 7 TypeError
0 8 TypeError
0 9 TypeError
0 10 TypeError
0 11 TypeError
0 12 TypeError
0 13 TypeError
0 14 TypeError
0 15 TypeError
0 16 TypeError
0 17 TypeError
1 0 TypeError
1 1 TypeError
1 2 TypeError
1 3 TypeError
1 4 TypeError
1 5 TypeError
1 6 TypeError
1 7 TypeError
1 8 TypeError
1 9 TypeError
1 10 TypeError
1 11 TypeError
1 12 TypeError
1 13 TypeError
1 14 TypeError
1 15 TypeError
1 16 TypeError
1 17 TypeError
2 0 TypeError
2 1 TypeError
2 2 TypeError
2 3 TypeError
2 4 TypeError
2 5 TypeError
2 6 TypeError
2 7 TypeError
2 8 TypeError
2 9 TypeError
2 10 TypeError
2 11 TypeError
2 12 TypeError
2 13 TypeError
2 14 TypeError
2 15 TypeError
2 16 TypeError
2 17 TypeError
3 0 TypeError
3 1 TypeError
3 2 TypeError
3 3 TypeError
3 4 TypeError
3 5 TypeError
3 6 TypeError
3 7 TypeError
3 8 TypeError
3 9 TypeError
3 10 TypeError
3 11 TypeError
3 12 TypeError
3 13 TypeError
3 14 TypeError
3 15 TypeError
3 16 TypeError
3 17 TypeError
4 0 TypeError
4 1 TypeError
4 2 TypeError
4 3 TypeError
4 4 TypeError
4 5 TypeError
4 6 TypeError
4 7 TypeError
4 8 TypeError
4 9 TypeError
4 10 TypeError
4 11 TypeError
4 12 TypeError
4 13 TypeError
4 14 TypeError
4 15 TypeError
4 16 TypeError
4 17 TypeError
5 0 TypeError
5 1 TypeError
5 2 TypeError
5 3 TypeError
5 4 TypeError
5 5 TypeError
5 6 TypeError
5 7 TypeError
5 8 TypeError
5 9 TypeError
5 10 TypeError
5 11 TypeError
5 12 TypeError
5 13 TypeError
5 14 TypeError
5 15 TypeError
5 16 TypeError
5 17 TypeError
6 0 TypeError
6 1 TypeError
6 2 TypeError
6 3 TypeError
6 4 TypeError
6 5 TypeError
6 6 TypeError
6 7 TypeError
6 8 TypeError
6 9 TypeError
6 10 TypeError
6 11 TypeError
6 12 TypeError
6 13 TypeError
6 14 TypeError
6 15 TypeError
6 16 TypeError
6 17 TypeError
7 0 TypeError
7 1 TypeError
7 2 TypeError
7 3 TypeError
7 4 TypeError
7 5 TypeError
7 6 TypeError
7 7 TypeError
7 8 TypeError
7 9 TypeError
7 10 TypeError
7 11 TypeError
7 12 TypeError
7 13 TypeError
7 14 TypeError
7 15 TypeError
7 16 TypeError
7 17 TypeError
8 0 TypeError
8 1 TypeError
8 2 TypeError
8 3 TypeError
8 4 TypeError
8 5 TypeError
8 6 TypeError
8 7 TypeError
8 8 TypeError
8 9 TypeError
8 10 TypeError
8 11 TypeError
8 12 TypeError
8 13 TypeError
8 14 TypeError
8 15 TypeError
8 16 TypeError
8 17 TypeError
9 0 TypeError
9 1 TypeError
9 2 TypeError
9 3 TypeError
9 4 TypeError
9 5 TypeError
9 6 TypeError
9 7 TypeError
9 8 TypeError
9 9 TypeError
9 10 TypeError
9 11 TypeError
9 12 TypeError
9 13 TypeError
9 14 TypeError
9 15 TypeError
9 16 TypeError
9 17 TypeError
10 0 TypeError
10 1 TypeError
10 2 TypeError
10 3 TypeError
10 4 TypeError
10 5 TypeError
10 6 TypeError
10 7 TypeError
10 8 TypeError
10 9 TypeError
10 10 TypeError
10 11 TypeError
10 12 TypeError
10 13 TypeError
10 14 TypeError
10 15 TypeError
10 16 TypeError
10 17 TypeError
11 0 TypeError
11 1 TypeError
11 2 TypeError
11 3 TypeError
11 4 TypeError
11 5 TypeError
11 6 TypeError
11 7 TypeError
11 8 TypeError
11 9 TypeError
11 10 TypeError
11 11 0
11 12 0
11 13 1
11 14 -1
11 15 -1
11 16 -1
11 17 1
12 0 TypeError
12 1 TypeError
12 2 TypeError
12 3 TypeError
12 4 TypeError
12 5 TypeError
12 6 TypeError
12 7 TypeError
12 8 TypeError
12 9 TypeError
12 10 TypeError
12 11 0
12 12 0
12 13 1
12 14 -1
12 15 -1
12 16 -1
12 17 1
13 0 TypeError
13 1 TypeError
13 2 TypeError
13 3 TypeError
13 4 TypeError
13 5 TypeError
13 6 TypeError
13 7 TypeError
13 8 TypeError
13 9 TypeError
13 10 TypeError
13 11 -1
13 12 -1
13 13 0
13 14 -1
13 15 -1
13 16 -1
13 17 1
14 0 TypeError
14 1 TypeError
14 2 TypeError
14 3 TypeError
14 4 TypeError
14 5 TypeError
14 6 TypeError
14 7 TypeError
14 8 TypeError
14 9 TypeError
14 10 TypeError
14 11 1
14 12 1
14 13 1
14 14 0
14 15 -1
14 16 -1
14 17 1
15 0 TypeError
15 1 TypeError
15 2 TypeError
15 3 TypeError
15 4 TypeError
15 5 TypeError
15 6 TypeError
15 7 TypeError
15 8 TypeError
15 9 TypeError
15 10 TypeError
15 11 1
15 12 1
15 13 1
15 14 1
15 15 0
15 16 -1
15 17 1
16 0 TypeError
16 1 TypeError
16 2 TypeError
16 3 TypeError
16 4 TypeError
16 5 TypeError
16 6 TypeError
16 7 TypeError
16 8 TypeError
16 9 TypeError
16 10 TypeError
16 11 1
16 12 1
16 13 1
16 14 1
16 15 1
16 16 0
16 17 1
17 0 TypeError
17 1 TypeError
17 2 TypeError
17 3 TypeError
17 4 TypeError
17 5 TypeError
17 6 TypeError
17 7 TypeError
17 8 TypeError
17 9 TypeError
17 10 TypeError
17 11 -1
17 12 -1
17 13 -1
17 14 -1
17 15 -1
17 16 -1
17 17 0
!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!==><<<>
!!!!!!!!!!!==><<<>
!!!!!!!!!!!<<=<<<>
!!!!!!!!!!!>>>=<<>
!!!!!!!!!!!>>>>=<>
!!!!!!!!!!!>>>>>=>
!!!!!!!!!!!<<<<<<=
===*/

function nodejsBufferCompareTest() {
    var b1, b2, b3, b4;
    var values;
    var tmp = [];

    b1 = new Buffer('foo');
    b2 = new Buffer('foo');
    b3 = new Buffer('fo');
    b4 = new Buffer('foo1');
    b5 = new Buffer('foo2');
    b6 = new Buffer('quux');
    b7 = new Buffer('abc');

    values = [
        undefined,
        null,
        true,
        false,
        123,
        'foo', 'fo', 'foo1', 'foo2', 'quux', 'abc',
        b1, b2, b3, b4, b5, b6, b7
    ];

    // Not sure about correct semantics because Node.js v0.12.1 crashes
    // when 'this' is not a Buffer.
    //
    // But reasonable semantics might be:
    //     - If 'this' is not a Buffer, TypeError.
    //     - If argument is not a Buffer, TypeError.
    //     - Compare buffers.

    values.forEach(function (v1, i1) {
        if (i1 > 0) { tmp.push('\n'); }
        values.forEach(function (v2, i2) {
            try {
                // Avoid Node.js issue to get an except string:
                //
                // $ node
                // > Buffer.prototype.compare.call(undefined, new Buffer(4))
                // Segmentation fault (core dumped)
                //
                // Seems to happen when 'this' is not a buffer but argument is.

                if (typeof process === 'object' && process.version !== undefined && !(v1 instanceof Buffer)) {
                    print(i1, i2, 'TypeError');
                    tmp.push('!');
                } else {
                    var cmp = Buffer.prototype.compare.call(v1, v2);
                    print(i1, i2, cmp);
                    if (cmp === -1) {
                        tmp.push('<');
                    } else if (cmp === 0) {
                        tmp.push('=');
                    } else if (cmp === 1) {
                        tmp.push('>');
                    } else {
                        tmp.push('?');
                    }
                }
            } catch (e) {
                print(i1, i2, e.name);
                tmp.push('!');
            }
        });
    });
    print(tmp.join(''));
}

try {
    print('Node.js Buffer instance compare() test');
    nodejsBufferCompareTest();
} catch (e) {
    print(e.stack || e);
}
