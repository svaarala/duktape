/*===
[object Object]
true
===*/

/* Object.getOwnPropertyDescriptor() should return an object, whose prototype
 * is Object.prototype.  Object.prototype is non-configurable and non-writable,
 * so there is no distinction between current and initial value.
 *
 * Once getOwnPropertyDescriptor() returned an object with null prototype.
 */

try {
    var obj = { foo: 1 };
    var pd = Object.getOwnPropertyDescriptor(obj, 'foo');

    print(Object.getPrototypeOf(pd));
    print(Object.getPrototypeOf(pd) === Object.prototype);
} catch (e) {
    print(e);
}
