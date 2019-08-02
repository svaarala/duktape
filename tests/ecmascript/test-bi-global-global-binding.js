/*
 *  'globalThis' binding based on https://github.com/tc39/proposal-global
 */

/*===
object
true
object
true
true
false
true
===*/

function test() {
    var pd, g;

    // Standard trick to access global object.
    g = new Function('return this')();

    // proposal-global provides 'globalThis' for cleaner standard access.
    print(typeof globalThis);
    print(g === globalThis);

    // Attributes in current proposal.
    pd = Object.getOwnPropertyDescriptor(g, 'globalThis');
    print(typeof pd);
    if (pd) {
        print(pd.value === g);
        print(pd.writable);
        print(pd.enumerable);
        print(pd.configurable);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
