/*===
{"bar":2,"foo":1,"baz":4,"quux":3}
{"bar":2,"1":5,"foo":1}
===*/

/* E5.1 Section 15.12.13, main algorithm, step 4.b.ii.5 requires that an
 * implementation detect duplicates in the property list and refuse to
 * serialize the same property name twice.
 *
 * Note: both V8 and Rhino fail this test (but in different ways).
 */

function dupTest1() {
    var obj = { foo: 1, bar: 2, quux: 3, baz: 4 };

    print(JSON.stringify(obj, [ 'bar', 'foo', 'baz', 'baz', 'baz', 'quux' ]));
}

function dupTest2() {
    var obj = { foo: 1, bar: 2, quux: 3, baz: 4, "1": 5 };

    // dup check happens after coercion
    print(JSON.stringify(obj, [ 'bar', 1, new Number(1), '1', 'foo' ]));
}

try {
    dupTest1();
    dupTest2();
} catch (e) {
    print(e);
}
