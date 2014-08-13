/*
 *  If array length is 0xffffffff, Array.prototype.push() appends all the
 *  requested items and then sets the final 'length'.
 *
 *  For Ecmascript arrays (objects with array special behavior), the item
 *  writes will succeed but the modified [[DefineOwnProperty]] algorithm
 *  in Section 15.4.5.1 will throw a RangeError.
 *
 *  When the argument is not an Ecmascript array, the length is updated to
 *  0x100000000.
 */

/*===
array
RangeError
4294967295 foo
RangeError
4294967295 foo bar quux
4294967295 foo bar
non-array
4294967296 foo
4294967296 foo bar quux
4294967295 foo bar
===*/

function testArray() {
    var a;

    // 0xffffffff is maximum 'normal' length for an Array, e.g. assigning
    // to offset 0xffffffff gets no special behavior to avoid bumping length
    // to 0x100000000.  However, Array.prototype.push() has no such limitation.
    // Check for -noncompliant- but real world compatible behavior to refuse a
    // push which would causes a non-32-bit length.

    a = [];
    a.length = 0xffffffff;
    try {
        a.push('foo');
    } catch (e) {
        print(e.name);
    }
    print(a.length, a[0xffffffff]);

    // Same happens for lower 'length' values when there are enough push()
    // arguments.

    a = [];
    a.length = 0xfffffffd;
    try {
        a.push('foo', 'bar', 'quux');
    } catch (e) {
        print(e.name);
    }
    print(a.length, a[0xfffffffd], a[0xfffffffe], a[0xffffffff]);

    // Boundary cases should work.

    a = [];
    a.length = 0xfffffffd;
    a.push('foo', 'bar');
    print(a.length, a[0xfffffffd], a[0xfffffffe]);
}

function testNonArray() {
    var a;

    a = { length: 0xffffffff };
    Array.prototype.push.call(a, 'foo');
    print(a.length, a[0xffffffff]);

    a = { length: 0xfffffffd };
    Array.prototype.push.call(a, 'foo', 'bar', 'quux');
    print(a.length, a[0xfffffffd], a[0xfffffffe], a[0xffffffff]);

    a = { length: 0xfffffffd };
    Array.prototype.push.call(a, 'foo', 'bar');
    print(a.length, a[0xfffffffd], a[0xfffffffe]);
}

print('array');
try {
    testArray();
} catch (e) {
    print(e);
}

print('non-array');
try {
    testNonArray();
} catch (e) {
    print(e);
}
