/*===
{"quux":3,"foo":1}
0 quux
1 foo
2 baz
{"quux":3,"foo":1,"baz":4}
===*/

/* Should ancestor properties be enumerated when creating PropertyList?
 * Specification is a bit unclear on this point.  See E5.1 Section 15.12.3,
 * main algorithm, step 4.b.ii.
 *
 * We currently iterate ancestor array indexed properties too.
 *
 * Creating an array instance with a chosen internal prototype is apparently
 * not possible.  However, Array.prototype is itself an array and we can add
 * a value to it.
 */

function propertyListAncestorTest() {
    var obj = { foo: 1, bar: 2, quux: 3, baz: 4 };
    var k;
    var plist;

    // baseline
    print(JSON.stringify(obj, [ 'quux', 'foo' ]));

    // this is not very common
    Array.prototype[2] = 'baz';

    // plist will contain "0" and "1", and will inherit "2"
    plist = [ 'quux', 'foo' ];
    for (var k in plist) {
        print(k, plist[k]);
    }

    // Now E5.1 Section 15.12.3, main algorithm, step 4.b.ii should presumably
    // iterate indices "0" and "1" from the array and "2" from the ancestor.
    // Note that 'length' plays no role in this.
    //
    // (Both Rhino and V8 fail this test.)
    print(JSON.stringify(obj, plist));
}

try {
    propertyListAncestorTest();
} catch (e) {
    print(e.name);
}
