/*===
string bar
string quux
===*/

var obj, val;

try {
    obj = {};
    val = (obj.foo = 'bar');
    print(typeof val, val);

    /* The fact that the assignment fails should have no effect on the
     * value of the assignment expression.  V8 3.7.12.22 evaluates the
     * assignment to 'undefined' if the property write fails.
     */
    Object.preventExtensions(obj);
    val = (obj.bar = 'quux');
    print(typeof val, val);
} catch (e) {
    print(e);
}
