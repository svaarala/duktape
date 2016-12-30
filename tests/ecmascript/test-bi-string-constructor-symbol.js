/*
 *  String(symbol) has special behavior in ES2015, Section 21.1.1.1:
 *
 *      ... If NewTarget is undefined and Type(value) is Symbol,
 *      return SymbolDescriptiveString(value).
 */

/*===
Symbol()
Symbol()
Symbol()
Symbol(foo)
Symbol(foo)
Symbol(Symbol.toPrimitive)
TypeError
===*/

function test() {
    var s;

    // A symbol is coerced into it's descriptive string.  The forms below
    // result in the same descriptive string, although an undefined symbol
    // description and an empty string symbol description are internally
    // separate cases.
    print(String(Symbol()));
    print(String(Symbol(void 0)));
    print(String(Symbol('')));

    // Global symbols (Symbol.for('foo')) and local symbols (Symbol('foo'))
    // have the same descriptive string.
    print(String(Symbol('foo')));
    print(String(Symbol.for('foo')));

    // Well-known symbols have well-known descriptions.
    print(String(Symbol.toPrimitive));

    // The symbol-to-descriptive-string behavior only applies to a
    // non-constructor call.  Constructor calls will invoke ToString(),
    // which ultimately causes a TypeError for symbols.
    try {
        print(new String(Symbol()));
    } catch (e) {
        print(e.name);
        //print(e.stack || e);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
