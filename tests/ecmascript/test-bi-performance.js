/*===
object
[object Object]
true
true true true
function
true
true true true
undefined
true
number
number
true
===*/

function test() {
    var pd;
    var t1, t2;
    var i;

    // 'performance' is a global object.  In Firefox and Chrome it is an
    // accessor, whose setter allows overwriting the property with a
    // value (the property gets converted to a data property).
    // In Duktape 'performance' is now a normal data property.
    print(typeof performance);
    print(Object.prototype.toString.call(performance));
    pd = Object.getOwnPropertyDescriptor(this, 'performance');
    print(pd.value !== void 0);
    print(pd.writable, pd.enumerable, pd.configurable);

    // 'performance.now' is a function, property attributes don't follow
    // the convention of normal built-ins and are 'wec'.
    print(typeof performance.now);
    pd = Object.getOwnPropertyDescriptor(performance, 'now');
    print(pd.value !== void 0);
    print(pd.writable, pd.enumerable, pd.configurable);

    // 'performance.timeOrigin' is missing for now.
    print(typeof performance.timeOrigin);
    pd = Object.getOwnPropertyDescriptor(performance, 'timeOrigin');
    print(pd === void 0);

    // Basic test of the guarantee that performance.now() won't go backwards.
    t1 = performance.now();
    print(typeof t1);
    for (i = 0; i < 1e7; i++) {
    }
    t2 = performance.now();
    print(typeof t2);
    print(t2 >= t1);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
