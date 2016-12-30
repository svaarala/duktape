/*
 *  Enumeration order for abandoned array part changed in Duktape 2.x with
 *  the introduction of duk_harray and ES2015 [[OwnPropertyKeys]] enumeration
 *  order which is applied to e.g. for-in (this is not required by ES2015).
 */

/*---
{
    "custom": true
}
---*/

/*===
with array part
- 0
- 1
- 2
- length
- myProperty
without array part
- 0
- 1
- 2
- length
- myProperty
array index added after abandoning array part
- 0
- 1
- 2
- 3
- length
- myProperty
===*/

function test() {
    var arr = [ 'foo', 'bar', 'quux' ];
    arr.myProperty = true;

    // When array part is present the array index properties enumerate first,
    // then the virtual .length property, and finally any other properties.

    print('with array part');
    Object.getOwnPropertyNames(arr).forEach(function (k) { print('-', k); });

    arr[100] = 'dummy';  // abandon array part
    arr.length = 3;

    // In Duktape 1.x the .length property is concrete and would be enumerated
    // after the index properties moved into the entry part.  Array indexes
    // added after array part abandonment would still appear last.
    //
    // In Duktape 2.x the duk_harray "natural" enum order for a sparse array
    // would be: .length (virtual), array indexes moved into entries, other
    // properties, and finally array index properties added after array became
    // sparse.  However, there's an ES2015 sort step for enumeration which fixes
    // the enumeration order to the same as for dense arrays.

    print('without array part');
    Object.getOwnPropertyNames(arr).forEach(function (k) { print('-', k); });

    arr.push('baz');
    print('array index added after abandoning array part');
    Object.getOwnPropertyNames(arr).forEach(function (k) { print('-', k); });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
