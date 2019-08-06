/*===
true false true
true false true
true false true
===*/

function test() {
    var pd;
    var G = new Function('return this;')();

    pd = Object.getOwnPropertyDescriptor(G, 'CBOR');
    print(pd.writable, pd.enumerable, pd.configurable);

    pd = Object.getOwnPropertyDescriptor(CBOR, 'encode');
    print(pd.writable, pd.enumerable, pd.configurable);

    pd = Object.getOwnPropertyDescriptor(CBOR, 'decode');
    print(pd.writable, pd.enumerable, pd.configurable);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
