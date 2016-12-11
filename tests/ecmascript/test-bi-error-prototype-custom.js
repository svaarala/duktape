/*===
function function false true
function function false true
function function false true
===*/

function test() {
    var pd;

    // Property attributes of a few interesting accessors.

    pd = Object.getOwnPropertyDescriptor(Error.prototype, 'fileName');
    print(typeof pd.get, typeof pd.set, pd.enumerable, pd.configurable);
    pd = Object.getOwnPropertyDescriptor(Error.prototype, 'lineNumber');
    print(typeof pd.get, typeof pd.set, pd.enumerable, pd.configurable);
    pd = Object.getOwnPropertyDescriptor(Error.prototype, 'stack');
    print(typeof pd.get, typeof pd.set, pd.enumerable, pd.configurable);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
