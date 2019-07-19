/*
 *  If a Date instance is frozen, setTime(), setUTCFullYear() etc must still work as
 *  they operate on an internal slot which is not bound by normal property access
 *  controls.
 */

/*===
1970-01-01T00:00:00.123Z
1970-01-01T00:00:00.234Z
1970-01-01T00:00:00.345Z
1970-01-01T00:00:00.123Z
1000-01-01T00:00:00.123Z
2000-01-01T00:00:00.123Z
done
===*/

function test1() {
    var d = new Date(123);
    print(d.toISOString());
    d.setTime(234);
    print(d.toISOString());
    Object.freeze(d);
    d.setTime(345);
    print(d.toISOString());
}

function test2() {
    var d = new Date(123);
    print(d.toISOString());
    d.setUTCFullYear(1000);
    print(d.toISOString());
    Object.freeze(d);
    d.setUTCFullYear(2000);
    print(d.toISOString());
}

try {
    test1();
} catch (e) {
    print(e.stack || e);
}

try {
    test2();
} catch (e) {
    print(e.stack || e);
}

print('done');
