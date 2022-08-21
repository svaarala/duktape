/*===
true false true
===*/

function test() {
    var pd;
    var G = new Function('return this;')();

    pd = Object.getOwnPropertyDescriptor(G, 'TextDecoder');
    print(pd.writable, pd.enumerable, pd.configurable);
}

test();
