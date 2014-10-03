/*===
{"key3":3,"key1":1}
{"key4":4,"key3":3,"key2":2}
===*/

/* Try an extremely sparse property list.
 *
 * The enumeration order requirement for processing the property list
 * is much stricter than the standard enumeration requirements.
 * E5.1 Section 15.12.3, main algorithm, step 4.b.ii:
 *
 *   "For each value v of a property of replacer that has an array
 *    index property name. The properties are enumerated in the
 *    ascending array index order of their names."
 *
 * This poses some difficulties for very sparse arrays:
 *
 *   - You can't enumerate the array efficiently by walking through
 *     each array index (consider an array with 2 elements, at indices
 *     0 and 999999999).  Also note that the "length" property plays
 *     no part in the specified algorithm, even though the PropertyList
 *     must be an Array (have internal class "Array").
 *
 *   - You can't use the default enumeration order; if the array part
 *     has been abandoned, there is no guarantee that array indexed
 *     properties are in correct order in the entry part.
 *
 *   - You could enumerate the keys and sort them, but this would be a
 *     relatively complex solution to a corner case problem.
 *
 * The current implementation enumerates array indexed properties using
 * the default enumeration order; this may now produce the wrong order
 * for sparse arrays (= no array part).
 *
 * As a side note, V8 seems to iterate over the index space one
 * by one.  This is VERY slow (but yields the correct order):
 *
 *   > var obj = { foo: 1, bar: 2, quux: 3 };
 *   > var arr = []; arr[1] = 'quux'; arr[100000000] = 'foo'; arr[500] = 'bar';
 *   > JSON.stringify(obj, arr)
 *   '{"quux":3,"bar":2,"foo":1}'
 */

function sparsePropertyListTest1() {
    var obj = { key1: 1, key2: 2, key3: 3, key4: 4, key5: 5 };

    var plist1 = [];
    var plist2 = [];

    // non-sparse array with gap
    plist1[0] = 'key3';
    plist1[2] = 'key1';
    print(JSON.stringify(obj, plist1));

    // correct order: key4, key3, key2
    // (don't make numbers very large, V8 spends a lot of time on this otherwise)
    plist2[1] = 'key4';
    plist2[9999999] = 'key2';   // this forces plist2 sparse
    plist2[1000000] = 'key3';   // this key now enumerates AFTER the previous one
    print(JSON.stringify(obj, plist2));
}

try {
    sparsePropertyListTest1();
} catch (e) {
    print(e.name);
}
