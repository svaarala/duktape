/*
 *  Symbol coercion
 */

/*@include util-symbol.js@*/

/*===
symbol coercion
true false
[object Symbol]
[object Symbol]
true false
true true
true true
TypeError
TypeError
Symbol(Symbol(foo))
Symbol(123)
TypeError
Symbol(noSideEffects)
true
true
true
TypeError
val
undefined
Symbol(foo)
Symbol(foo)
Symbol(foo)
Symbol(foo)
true false
false false
false false
TypeError
replacement @@toPrimitive called object [object Symbol]
TypeError
===*/

function symbolCoercionTest() {
    var s1, s2, s3, o1, o2, o3;
    var t;
    var obj;

    // Object coercion creates a wrapped Symbol which non-strict equals
    // the plain symbol.  Double Object coercion is idempotent (of course).
    s1 = Symbol('123');
    o1 = Object(s1);
    print(s1 == o1, s1 === o1);
    print(Object.prototype.toString.call(o1));  // [object Symbol]
    o2 = Object(o1);
    print(Object.prototype.toString.call(o2));
    print(s1 == o2, s1 === o2);
    print(o1 == o1, o1 === o1);
    print(o1 == o2, o1 === o2);

    // ToString coercion is a TypeError.  We need an internal operation that
    // invokes the conceptual ToString() operation directly.  Note that in ES2015
    // String(x) doesn't do that: it has specific support for (plain) Symbols.
    // Use parseFloat() here, it ToString() coerces its argument before parsing.
    try {
        s1 = Symbol('123');
        t = parseFloat(s1);
        print(t);
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }

    // Similarly, here Symbol() does a ToString() on its argument (a symbol here).
    try {
        s1 = Symbol(Symbol('foo'));
        print(String(s1));
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }

    // But explicit .toString() returns e.g. "Symbol(foo)" which works as an
    // argument for creating an odd symbol.
    try {
        s1 = Symbol(Symbol('foo').toString());
        print(String(s1));
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }

    // In ES2015 String() is not a direct call to the internal ToString()
    // algorithm.  Instead, it has special support for plain symbols while
    // other values get a straight ToString() coercion.
    try {
        s1 = Symbol('123');
        print(String(s1));
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }

    // Interestingly, String(Object(Symbol(...))) doesn't invoke the special
    // symbol behavior because the E6 Section 21.1.1.1 check in step 2.a is
    // only for plain symbols.  So, a Symbol object goes through ToString().
    // ToString() for an object invokes ToPrimitive() which usually returns
    // the plain symbol.  That is then ToString() coerced which results in a
    // TypeError.  At least Firefox and recent V8 behave this way also.
    s1 = Symbol('foo');
    o1 = Object(s1);
    try {
        print(String(o1));
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }

    // Note that the special symbol support in String() formats the
    // symbol as "Symbol(<description>)" without invoking Symbol.prototype.toString()!
    var old = setupLoggingSymbolCoercionMethods();
    try {
        s1 = Symbol('noSideEffects');
        print(String(s1));
    } catch (e) {
        print(e.name, e);
    }
    restoreSymbolCoercionMethods(old);

    // ToBoolean(): symbol coerces to true, even empty symbol description.
    s1 = Symbol('123');
    s2 = Symbol();
    s3 = Symbol.for('');
    print(Boolean(s1));
    print(Boolean(s2));
    print(Boolean(s3));

    // ToNumber coercion is a TypeError.  Same for all ToInteger() etc variants.
    try {
        s1 = Symbol('123');
        t = Number(s1);
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }

    // ToObject() coercion creates a Symbol object.  That Symbol object references
    // the argument symbol as its internal value, and non-strict compares true
    // with the symbol.  The object can also be used as a property key and will
    // reference the same property slot as the plain symbol.  (However, the object
    // is rejected by String().)
    s1 = Symbol('foo');
    o1 = Object(s1);
    s2 = Symbol('foo');
    o2 = Object(s2);

    obj = {};
    obj[s1] = 'val';
    print(obj[o1]);
    print(obj[o2]);

    print(String(s1));
    print(o1.toString());  // String(Object(Symbol(...))) is a TypeError
    print(String(s2));
    print(o2.toString());
    print(s1 == o1, s1 === o1);
    print(s1 == s2, s1 === o2);
    print(o1 == s2, o1 === o2);

    // ToPrimitive() for a plain symbol returns the symbol itself with no
    // side effects.
    //
    // ToPrimitive() for an object symbol coerces using a more complicated
    // algorithm.  If the value has @@toPrimitive it gets called.  Otherwise
    // the .valueOf() and/or .toString() methods are called depending on the
    // coercion hint.
    //
    // new Date(value) first ToPrimitive() coerces its argument, and for a
    // symbol it will then ToNumber() coerce it leading to a TypeError.
    // It will shake out side effects though which we test for here.

    // XXX: For now Duktape special cases symbols in ToPrimitive() as if
    // they had the default @@toPrimitive in E6 Section 19.4.3.4.  So this
    // test currently doesn't (correctly) trigger the @@toPrimitive side effect.

    var old = setupLoggingSymbolCoercionMethods();
    try {
        t = new Date(Symbol('foo'));
        print(t);
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }
    try {
        t = new Date(Object(Symbol('foo')));
        print(t);
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }
    restoreSymbolCoercionMethods(old);
}

try {
    print('symbol coercion');
    symbolCoercionTest();
} catch (e) {
    print(e.stack || e);
}
