/*
 *  Reading a variable.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var t;
    var r0 = 123.0;
    var r1 = 123.1;
    var r2 = 123.2;
    var r3 = 123.3;
    var r4 = 123.4;
    var r5 = 123.5;
    var r6 = 123.6;
    var r7 = 123.7;
    var r8 = 123.8;
    var r9 = 123.9;

    for (i = 0; i < 1e7; i++) {
        t = r0;
        t = r1;
        t = r2;
        t = r3;
        t = r4;
        t = r5;
        t = r6;
        t = r7;
        t = r8;
        t = r9;

        t = r0;
        t = r1;
        t = r2;
        t = r3;
        t = r4;
        t = r5;
        t = r6;
        t = r7;
        t = r8;
        t = r9;

        t = r0;
        t = r1;
        t = r2;
        t = r3;
        t = r4;
        t = r5;
        t = r6;
        t = r7;
        t = r8;
        t = r9;

        t = r0;
        t = r1;
        t = r2;
        t = r3;
        t = r4;
        t = r5;
        t = r6;
        t = r7;
        t = r8;
        t = r9;

        t = r0;
        t = r1;
        t = r2;
        t = r3;
        t = r4;
        t = r5;
        t = r6;
        t = r7;
        t = r8;
        t = r9;

        t = r0;
        t = r1;
        t = r2;
        t = r3;
        t = r4;
        t = r5;
        t = r6;
        t = r7;
        t = r8;
        t = r9;

        t = r0;
        t = r1;
        t = r2;
        t = r3;
        t = r4;
        t = r5;
        t = r6;
        t = r7;
        t = r8;
        t = r9;

        t = r0;
        t = r1;
        t = r2;
        t = r3;
        t = r4;
        t = r5;
        t = r6;
        t = r7;
        t = r8;
        t = r9;

        t = r0;
        t = r1;
        t = r2;
        t = r3;
        t = r4;
        t = r5;
        t = r6;
        t = r7;
        t = r8;
        t = r9;

        t = r0;
        t = r1;
        t = r2;
        t = r3;
        t = r4;
        t = r5;
        t = r6;
        t = r7;
        t = r8;
        t = r9;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
