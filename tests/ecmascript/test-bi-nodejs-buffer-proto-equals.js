/*
 *  Node.js Buffer equals()
 */

/*@include util-nodejs-buffer.js@*/

/*===
node.js Buffer equals() test
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
8 0 TypeError
8 1 TypeError
8 2 TypeError
8 3 TypeError
8 4 TypeError
8 5 TypeError
8 6 TypeError
8 7 TypeError
8 8 true
8 9 true
8 10 false
8 11 false
9 0 TypeError
9 1 TypeError
9 2 TypeError
9 3 TypeError
9 4 TypeError
9 5 TypeError
9 6 TypeError
9 7 TypeError
9 8 true
9 9 true
9 10 false
9 11 false
10 0 TypeError
10 1 TypeError
10 2 TypeError
10 3 TypeError
10 4 TypeError
10 5 TypeError
10 6 TypeError
10 7 TypeError
10 8 false
10 9 false
10 10 true
10 11 false
11 0 TypeError
11 1 TypeError
11 2 TypeError
11 3 TypeError
11 4 TypeError
11 5 TypeError
11 6 TypeError
11 7 TypeError
11 8 false
11 9 false
11 10 false
11 11 true
!!!!!!!!!!!!
!!!!!!!!!!!!
!!!!!!!!!!!!
!!!!!!!!!!!!
!!!!!!!!!!!!
!!!!!!!!!!!!
!!!!!!!!!!!!
!!!!!!!!!!!!
!!!!!!!!1100
!!!!!!!!1100
!!!!!!!!0010
!!!!!!!!0001
===*/

function nodejsBufferEqualsTest() {
    var b1, b2, b3, b4;
    var values;
    var tmp = [];

    b1 = new Buffer('foo');
    b2 = new Buffer('foo');
    b3 = new Buffer('foo1');
    b4 = new Buffer('bar');

    values = [
        undefined,
        null,
        true,
        false,
        123,
        'foo', 'foo1', 'bar',
        b1, b2, b3, b4
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
                // > Buffer.prototype.equals.call(undefined, new Buffer(4))
                // Segmentation fault (core dumped)
                //
                // Seems to happen when 'this' is not a buffer but argument is.

                if (typeof process === 'object' && process.version !== undefined && !(v1 instanceof Buffer)) {
                    print(i1, i2, 'TypeError');
                    tmp.push('!');
                } else {
                    var eq = Buffer.prototype.equals.call(v1, v2);
                    print(i1, i2, eq);
                    tmp.push(eq ? '1' : '0');
                }
            } catch (e) {
                print(i1, i2, e.name);
                tmp.push('!');
            }
        });
    });
    print(tmp.join(''));
}

// Note: Buffer.equals() does not exist.

try {
    print('node.js Buffer equals() test');
    nodejsBufferEqualsTest();
} catch (e) {
    print(e.stack || e);
}
