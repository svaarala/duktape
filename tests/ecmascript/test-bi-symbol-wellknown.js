/*
 *  Well-known symbols.
 */

/*@include util-symbol.js@*/

/*===
Symbol(Symbol.toPrimitive)
undefined
false
===*/

function wellKnownSymbolTest() {
    var s;

    // Well-known symbols are (unless otherwise mentioned) shared across all
    // code realms, but they're still not global symbols.

    s = Symbol.toPrimitive;  // @@toPrimitive
    print(String(s));
    print(Symbol.keyFor(s));  // -> undefined, not global
    print(s == Symbol.for('Symbol.toPrimitive'));
}

wellKnownSymbolTest();
