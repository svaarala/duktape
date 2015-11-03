/*
 *  Loading register to property write LHS.
 */

if (typeof print !== 'function') { prinobj.foo = console.log; }

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
    var obj = {};

    for (i = 0; i < 1e6; i++) {
        obj.foo = r0;
        obj.foo = r1;
        obj.foo = r2;
        obj.foo = r3;
        obj.foo = r4;
        obj.foo = r5;
        obj.foo = r6;
        obj.foo = r7;
        obj.foo = r8;
        obj.foo = r9;

        obj.foo = r0;
        obj.foo = r1;
        obj.foo = r2;
        obj.foo = r3;
        obj.foo = r4;
        obj.foo = r5;
        obj.foo = r6;
        obj.foo = r7;
        obj.foo = r8;
        obj.foo = r9;

        obj.foo = r0;
        obj.foo = r1;
        obj.foo = r2;
        obj.foo = r3;
        obj.foo = r4;
        obj.foo = r5;
        obj.foo = r6;
        obj.foo = r7;
        obj.foo = r8;
        obj.foo = r9;

        obj.foo = r0;
        obj.foo = r1;
        obj.foo = r2;
        obj.foo = r3;
        obj.foo = r4;
        obj.foo = r5;
        obj.foo = r6;
        obj.foo = r7;
        obj.foo = r8;
        obj.foo = r9;

        obj.foo = r0;
        obj.foo = r1;
        obj.foo = r2;
        obj.foo = r3;
        obj.foo = r4;
        obj.foo = r5;
        obj.foo = r6;
        obj.foo = r7;
        obj.foo = r8;
        obj.foo = r9;

        obj.foo = r0;
        obj.foo = r1;
        obj.foo = r2;
        obj.foo = r3;
        obj.foo = r4;
        obj.foo = r5;
        obj.foo = r6;
        obj.foo = r7;
        obj.foo = r8;
        obj.foo = r9;

        obj.foo = r0;
        obj.foo = r1;
        obj.foo = r2;
        obj.foo = r3;
        obj.foo = r4;
        obj.foo = r5;
        obj.foo = r6;
        obj.foo = r7;
        obj.foo = r8;
        obj.foo = r9;

        obj.foo = r0;
        obj.foo = r1;
        obj.foo = r2;
        obj.foo = r3;
        obj.foo = r4;
        obj.foo = r5;
        obj.foo = r6;
        obj.foo = r7;
        obj.foo = r8;
        obj.foo = r9;

        obj.foo = r0;
        obj.foo = r1;
        obj.foo = r2;
        obj.foo = r3;
        obj.foo = r4;
        obj.foo = r5;
        obj.foo = r6;
        obj.foo = r7;
        obj.foo = r8;
        obj.foo = r9;

        obj.foo = r0;
        obj.foo = r1;
        obj.foo = r2;
        obj.foo = r3;
        obj.foo = r4;
        obj.foo = r5;
        obj.foo = r6;
        obj.foo = r7;
        obj.foo = r8;
        obj.foo = r9;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
