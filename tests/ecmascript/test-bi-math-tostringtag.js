/*===
Math
false false true
===*/

try {
    var pd;

    print(Math[Symbol.toStringTag]);
    pd = Object.getOwnPropertyDescriptor(Math, Symbol.toStringTag);
    print(pd.writable, pd.enumerable, pd.configurable);
} catch (e) {
    print(e.stack || e);
}
