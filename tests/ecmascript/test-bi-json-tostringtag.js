/*===
JSON
false false true
===*/

try {
    var pd;

    print(JSON[Symbol.toStringTag]);
    pd = Object.getOwnPropertyDescriptor(JSON, Symbol.toStringTag);
    print(pd.writable, pd.enumerable, pd.configurable);
} catch (e) {
    print(e.stack || e);
}
