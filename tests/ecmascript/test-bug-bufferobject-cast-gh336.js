/*
 *  Demonstrate double-to-integer casting issues present on some (broken)
 *  exotic ARM platforms.  This is not a Duktape bug as such, because the
 *  underlying cause is broken double casts.  For example:
 *
 *    double d = (double) 0xdeadbeefUL;
 *    unsigned int i = (unsigned int) d;
 *
 *  Instead of the above 'i' being 0xdeadbeefUL, 'i' becomes 0x7fffffffUL
 *  on this particular platform.  This also shows up in the test below.
 *
 *  This testcase demonstrates the issue if present, so that it's possible
 *  to see if it has been fixed.
 *
 *  https://github.com/svaarala/duktape/issues/336
 */

/*---
{
    "custom": true
}
---*/

/*===
|414243deadbeef48|
|4142437fedcba948|
===*/

function test() {
    var b;

    // Here 0xdeadbeef incorrectly casts to 0x7fffffff on the exotic target.
    b = new Buffer('ABCDEFGH');
    b.writeUInt32BE(0xdeadbeef, 3);
    print(Duktape.enc('jx', Duktape.Buffer(b)));

    // But 0x7fedcba9 works correctly.
    b = new Buffer('ABCDEFGH');
    b.writeUInt32BE(0x7fedcba9, 3);
    print(Duktape.enc('jx', Duktape.Buffer(b)));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
