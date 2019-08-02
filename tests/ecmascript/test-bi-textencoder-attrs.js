/*===
true false true
===*/

function test() {
    var pd;
    var G = new Function('return this;')();

    pd = Object.getOwnPropertyDescriptor(G, 'TextEncoder');
    print(pd.writable, pd.enumerable, pd.configurable);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
