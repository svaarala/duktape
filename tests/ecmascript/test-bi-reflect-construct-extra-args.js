/*
 *  Reflect.construct() arguments beyond newTarget are ignored.
 */

/*===
object
1970-01-01T00:00:00.123Z
object
1970-01-01T00:00:00.123Z
===*/

function test() {
    var t = Reflect.construct(Date, [ 123 ], Date);
    print(typeof t);
    print(t.toISOString());

    // This should work like Reflect.construct(Date, [ 123 ], Date).
    var t = Reflect.construct(Date, [ 123 ], Date, 'extra');
    print(typeof t);
    print(t.toISOString());
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
