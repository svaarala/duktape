/*
 *  Miscellaneous Symbol tests.
 */

/*@include util-symbol.js@*/

/*===
symbol misc
symbol
[object Symbol]
Symbol(123)
symbol
[object Symbol]
Symbol()
symbol
[object Symbol]
Symbol(foo)
undefined
undefined undefined undefined undefined
[object Symbol]
[object Symbol]
24
83 121 109 98 111 108 40 102 111 111 0 98 97 114 255 113 117 117 120 65535 98 97 122 41
symbol
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
42
Symbol(Symbol.toStringTag),Symbol(meaningOfLife)
Symbol(Symbol.toStringTag),Symbol(meaningOfLife)
Math
Error
TypeError
TypeError
[object Symbol]
[object Symbol]
FOO
TypeError
Symbol(foo)
TypeError
===*/

function symbolMiscTest() {
    var s;

    s = Symbol('123');
    print(typeof s);
    print(Object.prototype.toString.call(s));
    print(s.toString());

    s = Symbol();
    print(typeof s);
    print(Object.prototype.toString.call(s));
    print(s.toString());

    s = Symbol.for('foo');
    print(typeof s);
    print(Object.prototype.toString.call(s));
    print(s.toString());

    // Symbol doesn't have a virtual .length or index properties.  This is easy
    // to get wrong in the current implementation where symbols are internally
    // strings.
    s1 = Symbol.for('123');  // internal representation: 80 '1' '2' '3'
    print(s1.length);  // Note: cannot use '0' in s1; requires Object argument
    print(typeof s1[0], typeof s1[1], typeof s1[2], typeof s1[3]);

    // Object.prototype.toString() for plain and object symbol.
    s1 = Symbol('foo');
    print(Object.prototype.toString.call(s1));
    print(Object.prototype.toString.call(Object(s1)));

    // NUL and 0xFF (codepoint) within a symbol description; tests for some
    // trivial implementation errors.  Duktape uses the byte 0xFF internally
    // to separate the symbol description and an internal unique suffix; the
    // 0xFF byte (not codepoint!) cannot occur in CESU-8 or extended UTF-8.
    s1 = Symbol('foo\u0000bar\u00ffquux\uffffbaz');
    //print(String(s1));
    print(String(s1).length);
    print(Array.prototype.map.call(String(s1), function (c) { return c.charCodeAt(0); }).join(' '));

    // eval() in general returns any non-string arguments as is, and this
    // applies to symbols too.
    print(typeof eval(Symbol('foo')));

    // Node.js Buffer fill() TypeErrors on symbol.
    var buf = new Buffer(10);
    try {
        buf.fill(Symbol.for('foo'));
        print('never here:', Duktape.enc('jx', buf));
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }

    // Node.js Buffer write() TypeErrors on symbol.
    var buf = new Buffer(10);
    try {
        buf.fill(0);
        buf.write(Symbol('foo'));
        print('never here:', Duktape.enc('jx', buf));
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }

    // String.prototype.toString() rejects symbols and wrapped Symbols.
    try {
        print(typeof String.prototype.toString.call(Symbol('foo')));
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }
    try {
        print(typeof String.prototype.toString.call(Object(Symbol('foo'))));
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }

    // new Function(symbol) is rejected because argument is ToString() coerced.
    try {
        print(typeof new Function(Symbol.for('foo')));
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }

    // String.replace() uses ToString() and rejects symbols.
    try {
        print('foo'.replace(Symbol()));  // search
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }
    try {
        print('foo'.replace('f', Symbol()));  // replace
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }

    // Uint8Array.allocPlain() accepts strings but not symbols.
    try {
        Uint8Array.allocPlain(Symbol.for('foo'));
    } catch (e) {
        //print(e.stack);
        print(e.name);
    }

    // Misc testing.
    Math[Symbol.for('meaningOfLife')] = 42;
    print(Math[Symbol.for('meaningOfLife')]);
    print(Object.getOwnPropertySymbols(Math).map(String));
    print(Object.getOwnPropertySymbols(Math).map(function (v) { return String(v); }));

    // In the initial symbol implementation merge @@toStringTag is not yet
    // used for anything, but Math[@@toStringTag] is defined so that well-known
    // symbol support is exercised through tooling etc.
    print(Math[Symbol.toStringTag]);

    // Error.prototype.toString() called for a symbol.
    print(Error.prototype.toString.call(Object(Symbol())));

    // While String(sym) creates a symbol description, String .toString()
    // rejects symbols with a TypeError.
    try {
        print(String.prototype.toString.call(Symbol('foo')));
    } catch (e) {
        print(e.name);
    }
    try {
        print(String.prototype.toString.call(Object(Symbol('foo'))));
    } catch (e) {
        print(e.name);
    }

    // Object .toString() works.
    print(Object.prototype.toString.call(Symbol('foo')));
    print(Object.prototype.toString.call(Object(Symbol('foo'))));

    // String.prototype methods don't generally work on symbols.
    print(String.prototype.toUpperCase.call('foo'));
    try {
        print(String.prototype.toUpperCase.call(Symbol.for('foo')));
    } catch (e) {
        print(e.name);
    }

    // Symbol.prototype methods reject non-symbols.
    print(Symbol.prototype.toString.call(Symbol.for('foo')));
    try {
        print(Symbol.prototype.toString.call('foo'));
    } catch (e) {
        print(e.name);
    }
}

try {
    print('symbol misc');
    symbolMiscTest();
} catch (e) {
    print(e.stack || e);
}
