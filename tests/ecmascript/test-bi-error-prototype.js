/*===
string true false true
string true false true
function true false true
===*/

function test() {
    var pd;

    pd = Object.getOwnPropertyDescriptor(Error.prototype, 'name');
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);
    pd = Object.getOwnPropertyDescriptor(Error.prototype, 'message');
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);
    pd = Object.getOwnPropertyDescriptor(Error.prototype, 'toString');
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
