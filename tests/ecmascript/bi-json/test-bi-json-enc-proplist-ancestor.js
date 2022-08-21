/*===
{"quux":3,"foo":1}
0 quux
1 foo
2 baz
{"quux":3,"foo":1}
1 foo
0 baz
2 baz
{"baz":4,"foo":1}
===*/

/* Should ancestor properties be enumerated when creating PropertyList?
 * Specification is a bit unclear on this point.  See E5.1 Section 15.12.3,
 * main algorithm, step 4.b.ii.
 *
 * Newer specifications use an explicit index loop (to .length - 1) and make
 * it clear ancestors are enumerated (but not beyond .length - 1).
 */

function propertyListAncestorTest() {
    var obj = { foo: 1, bar: 2, quux: 3, baz: 4 };
    var k;
    var plist;

    // Baseline.
    print(JSON.stringify(obj, [ 'quux', 'foo' ]));

    // Plist will contain "0" and "1"; it will inherit "2" but it's >= .length so it's ignored
    // by JSON.stringify().
    Array.prototype[2] = 'baz';
    plist = [ 'quux', 'foo' ];
    for (var k in plist) {
        print(k, plist[k]);
    }
    print(JSON.stringify(obj, plist));

    // If the gap is at < .length, it gets looked up from an ancestor.
    Array.prototype[0] = 'baz';
    plist = [];
    plist[1] = 'foo';
    for (var k in plist) {
        print(k, plist[k]);
    }
    print(JSON.stringify(obj, plist));
}

propertyListAncestorTest();
