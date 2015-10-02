/*
 *  Special variant of assign-addto test where the initial 't' is not
 *  initialized so it becomes NaN on first assignment and stays that
 *  way for the whole test.
 *
 *  For x64 there's not much performance difference between 't' being
 *  a NaN, a fastint, or a double.
 *
 *  But for x86 there's a roughly 10x difference between 't' being NaN
 *  and 't' being a double (and 20x difference between NaN and fastint).
 *  A similar difference occurs when duk_tval is packed or unpacked, so
 *  the difference is not caused by packed duk_tval and NaN normalization
 *  involved in that.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;

    // With this initializer: x86 packed ~2,4 sec, x86 unpacked ~1,6sec, x64 unpacked ~1,5 sec
    //var t = 1;

    // With this initializer: x86 packed ~5,8 sec, x86 unpacked ~3,1 sec, x64 unpacked ~1,7 sec
    //var t = 0.1;

    // With this initializer: x86 packed ~33,1 sec (!), x86 unpacked ~30,0 sec (!), x64 unpacked ~1,7 sec
    var t;

    var a = 10;

    for (i = 0; i < 1e7; i++) {
        t += a; t += a; t += a; t += a; t += a;
        t += a; t += a; t += a; t += a; t += a;

        t += a; t += a; t += a; t += a; t += a;
        t += a; t += a; t += a; t += a; t += a;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
