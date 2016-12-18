/*
 *  Symbol creation.
 */

/*@include util-symbol.js@*/
/*@include util-string.js@*/

/*===
symbol creation
TypeError
TypeError
Symbol()
Symbol()
Symbol()
Symbol()
false false
Symbol(123)
Symbol(123)
Symbol(123)
Symbol(123)
false false
Symbol(123)
Symbol(123)
Symbol(123)
Symbol(123)
false false
false false
true true
Symbol(123)
Symbol(true)
Symbol()
Symbol()
Symbol(null)
Symbol([object ArrayBuffer])
Symbol(undefined)
Symbol(undefined)
Symbol(null)
Symbol([object ArrayBuffer])
false
false
true
true
true
"Symbol(foo<U+0000><U+CAFE>bar)"
===*/

function symbolCreationTest() {
    var s, s1, s2, s3;

    // Constructing as 'new' is a TypeError.
    try {
        s = new Symbol();
    } catch (e) {
        print(e.name);
    }
    try {
        s = new Symbol('123');
    } catch (e) {
        print(e.name);
    }

    // Creating an anonymous symbol.
    s1 = Symbol();
    s2 = Symbol();
    print(String(s1));
    print(s1.toString());
    print(String(s2));
    print(s2.toString());
    print(s1 == s2, s1 === s2);  // never equal

    // Creating a symbol with a description.
    s1 = Symbol('123');
    s2 = Symbol('123');
    print(String(s1));
    print(s1.toString());
    print(String(s2));
    print(s2.toString());
    print(s1 == s2, s1 === s2);  // never equal

    // Creating a global symbol.
    s1 = Symbol('123');
    s2 = Symbol.for('123');
    s3 = Symbol.for('123');
    print(String(s1));
    print(s1.toString());
    print(String(s2));
    print(s2.toString());
    print(s1 == s2, s1 === s2);  // never equal
    print(s1 == s3, s1 === s3);  // never equal
    print(s2 == s3, s2 === s3);  // equal

    // Symbol() argument is string coerced.
    s1 = Symbol(123);
    s2 = Symbol(true);
    print(String(s1));
    print(String(s2));

    // Missing argument and undefined are treated specially and create a
    // a Symbol with internal "description" set to undefined.  This is
    // technically different from a Symbol with an empty string as its
    // internal description, but the difference doesn't seem to be
    // externally visible.  Other argument types are string coerced (even
    // when it makes little sense).
    s1 = Symbol();
    s2 = Symbol(void 0);
    print(String(s1));
    print(String(s2));
    s1 = Symbol(null);
    print(String(s1));
    s1 = Symbol(new ArrayBuffer(9));
    print(String(s1));

    // Symbol.for coerces an undefined argument to 'undefined' rather than
    // empty string.
    s1 = Symbol.for();
    s2 = Symbol.for(void 0);
    print(String(s1));
    print(String(s2));
    s1 = Symbol.for(null);
    print(String(s1));
    s1 = Symbol.for(new ArrayBuffer(9));
    print(String(s1));

    // As a special case, ensure that symbols with an undefined description
    // are distinct.
    print(Symbol(void 0) === Symbol(void 0));
    print(Symbol() === Symbol());

    // But Symbol.for() is the same as Symbol.for('undefined') and always
    // the same symbol.
    print(Symbol.for() === Symbol.for());
    print(Symbol.for() === Symbol.for(void 0));
    print(Symbol.for() === Symbol.for('undefined'));

    // Symbol description may contain arbitrary characters.
    safePrintString(String(Symbol('foo\u0000\ucafebar')));
}

try {
    print('symbol creation');
    symbolCreationTest();
} catch (e) {
    print(e.stack || e);
}
