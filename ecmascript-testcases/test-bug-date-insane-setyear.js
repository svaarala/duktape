/*
 *  Instead of a clean error, causes an assert failure in Duktape 0.11.0.
 *
 *  Without asserts the code works, no valgrind issues.  The assert is:
 *
 *  PANIC 54: assertion failed: t2 + DUK__WEEKDAY_MOD_ADDER >= 0 (duk_bi_date.c:1043) (segfaulting on purpose)
 */

/*===
about to set year
year set ok
Invalid Date
done
===*/

function test() {
    var d = new Date();
    print('about to set year');
    d.setFullYear(123912921921321);
    print('year set ok');
    print(d);
}

try {
    test();
} catch (e) {
    print(e);
}
print('done');
