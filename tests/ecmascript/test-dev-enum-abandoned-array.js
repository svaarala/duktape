/*
 *  Enumeration order for abandoned array part changed in Duktape 2.x.
 *  This testcase illustrates the change.  The change is related to array
 *  instance .length which is non-enumerable, so that the change only affects
 *  Object.getOwnPropertyNames() and duk_enum() calls which request enumeration
 *  of non-enumerable properties.
 */

/*===
with array part
- 0
- 1
- 2
- length
- myProperty
without array part
- length
- 0
- 1
- 2
- myProperty
array index added after abandoning array part
- length
- 0
- 1
- 2
- myProperty
- 3
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

    // When array part is not present, the virtual .length property enumerates
    // first, followed by index properties moved into the entry part, followed
    // by other properties (and any array index writes which happen after the
    // array part is abandoned.
    //
    // In Duktape 1.x the .length property is concrete and would be enumerated
    // after the index properties moved into the entry part.  Array indexes
    // added after array part abandonment would still appear last.

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
