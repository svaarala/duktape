/*===
[object Object]
[object MyObject]
[object Object]
[object Object]
[object Object]
[object OverrideDate]
[object Symbol]
[object OverrideSymbol]
[object Symbol]
===*/

function basicTest() {
    var o = {};
    print(o.toString());
    o[Symbol.toStringTag] = 'MyObject';
    print(o.toString());

    // If @@toStringTag exists but isn't a string, it is ignored.
    // Check a Symbol value specifically because it's internally very
    // string like; it too must be ignored.
    var o = {};
    o[Symbol.toStringTag] = Symbol('foo');
    print(o.toString());
    var o = {};
    o[Symbol.toStringTag] = 123;
    print(o.toString());
    var o = {};
    o[Symbol.toStringTag] = { test: 1 };
    print(o.toString());

    // Override @toStringTag for a built-in object which has a legacy
    // fallback.
    var o = new Date();
    o[Symbol.toStringTag] = 'OverrideDate';
    print(Object.prototype.toString.call(o));

    // If a Symbol object has a non-string @@toStringTag, the legacy algorithm
    // should be used.  Based on https://www.ecma-international.org/ecma-262/6.0/#sec-object.prototype.tostring
    // (which is unchanged in later versions for this case) the result should
    // be "[object Object]":
    //  - Step 3: the value is already an object, so no-op.
    //  - Steps 6-14: no match.
    //  - Step 15 sets builtinTag to "Object".
    //  - Steps 16-18: Type(tag) is not a string, so set tag to builtinTag, i.e. "Object".
    //  - Step 19: return "[object Object]".
    //
    // However, both V8 and Firefox return "[object Symbol]" so we also do that
    // here for now.  This may change in later versions.
    var o = Object(Symbol('foo'));
    print(Object.prototype.toString.call(o));
    var o = Object(Symbol('foo'));
    Object.defineProperty(o, Symbol.toStringTag, { value: 'OverrideSymbol', configurable: true });
    print(Object.prototype.toString.call(o));
    var o = Object(Symbol('foo'));
    Object.defineProperty(o, Symbol.toStringTag, { value: 123, configurable: true });
    print(Object.prototype.toString.call(o));
}

try {
    basicTest();
} catch (e) {
    print(e.stack || e);
}

/*===
[object OverrideNumber]
[object OverrideNumber]
false
true
false
false
false
true
Proxy get Symbol(Symbol.toStringTag)
Hohum
Proxy get Symbol(Symbol.toStringTag)
[object Hohum]
Number.prototype[@@toStringTag] read
object false true false
OverrideString
Number.prototype[@@toStringTag] read
object false true false
[object OverrideString]
Number.prototype[@@toStringTag] read
number false true false
OverrideString
Number.prototype[@@toStringTag] read
object false false false
[object OverrideString]
===*/

function primTypeTest() {
    var o;

    // When Object.prototype.toString() is applied to a primitive type,
    // the value is (at least conceptually) converted to an object and
    // a @@toStringTag lookup is done on the object.  As a result, setting
    // e.g. Number.prototype[@@toStringTag] should affect .toString()
    // conversion of a primitive value too.
    Object.defineProperty(Number.prototype, Symbol.toStringTag, {
        value: 'OverrideNumber',
        configurable: true
    });
    o = new Number(123);
    print(Object.prototype.toString.call(o));
    o = 123;
    print(Object.prototype.toString.call(o));

    // For summaries a @@toStringTag lookup is not yet done because it might
    // invoke Proxy or getter side effects.  So for now only the default legacy
    // class name is available.  Ideally summary string creation would do a
    // safe lookup (skipping Proxy and getter side effects) and then sanitize
    // the @@toStringTag string.  This would work fine in 99+% of cases.  Also
    // note that summary strings retain the primitive vs. object type difference
    // because it matters for diagnostics.
    o = new Number(123);
    try {
        o();
        print('never here');
    } catch (e) {
        print(e.message.indexOf('[object OverrideNumber]') >= 0);
        print(e.message.indexOf('[object Number]') >= 0);
        print(e.message.indexOf('123') >= 0);
    }
    o = 123;
    try {
        o();
        print('never here');
    } catch (e) {
        print(e.message.indexOf('[object OverrideNumber]') >= 0);
        print(e.message.indexOf('[object Number]') >= 0);
        print(e.message.indexOf('123') >= 0);
    }

    // If the target is a Proxy, a @@toStringTag lookup is made on the Proxy.
    var proxy = new Proxy({}, {
        get: function (targ, key, recv) {
            print('Proxy get', String(key));
            if (key === Symbol.toStringTag) {
                return 'Hohum';
            }
        }
    });
    print(proxy[Symbol.toStringTag]);
    print(Object.prototype.toString.call(proxy));

    // If @@toStringTag is a getter, the getter 'this' binding must be a
    // ToObject() coercion of the primitive value.  Normally the 'this'
    // binding would be the primitive value as is for a strict getter,
    // but the Object.prototype.toString() algorithm does an explicit
    // object coercion.
    Object.defineProperty(Number.prototype, Symbol.toStringTag, {
        get: function () {
            'use strict';
            print('Number.prototype[@@toStringTag] read');
            print(typeof this, this === 'foo', this === o, this === Number.prototype);
            return 'OverrideString';
        },
        configurable: true
    });
    o = new Number(123);
    print(o[Symbol.toStringTag]);
    print(Object.prototype.toString.call(o));
    o = 123;
    print(o[Symbol.toStringTag]);
    print(Object.prototype.toString.call(o));
}

try {
    primTypeTest();
} catch (e) {
    print(e.stack || e);
}
