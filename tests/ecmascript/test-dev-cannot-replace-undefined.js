/*
 *  The global 'undefined' value is non-writable, non-enumerable, and
 *  non-configurable, so it cannot be replaced.  But you can declare a
 *  shadowing variable.
 */

/*===
undefined undefined
undefined undefined false false false
undefined undefined
undefined undefined false false false
number 234
===*/

print(typeof undefined, undefined);
var pd = Object.getOwnPropertyDescriptor(this, 'undefined');
print(typeof pd.value, pd.value, pd.writable, pd.enumerable, pd.configurable);

undefined = 123;  // silent failure

print(typeof undefined, undefined);
var pd = Object.getOwnPropertyDescriptor(this, 'undefined');
print(typeof pd.value, pd.value, pd.writable, pd.enumerable, pd.configurable);

function test() {
    var undefined = 234;

    print(typeof undefined, undefined);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
