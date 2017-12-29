/*===
@@toPrimitive true number
NaN
@@toPrimitive true default
PrimValue
@@toPrimitive true number
NaN
@@toPrimitive true default
PrimValue
1
true
@@toPrimitive true number
TypeError
0 1234
1 1234
2 TypeError
3 TypeError
4 TypeError
5 TypeError
123
Symbol.prototype[@@toPrimitive] string
undefined
123
Symbol.prototype.toString
Symbol.prototype.valueOf
432
Symbol.prototype.valueOf
432
undefined
123
===*/

function basicTest() {
    // @@toPrimitive on an object argument, own or inherited property.
    var obj = { [Symbol.toPrimitive]: function (hint) {
            print('@@toPrimitive', this === obj, hint);
            return 'PrimValue';
        }
    };
    print(+obj);
    print("" + obj);
    var obj = Object.create({ [Symbol.toPrimitive]: function (hint) {
            print('@@toPrimitive', this === obj, hint);
            return 'PrimValue';
        }
    });
    print(+obj);
    print("" + obj);

    // @@toPrimitive on a primitive value; doesn't inherit from prototype,
    // primitive value is returned as is.
    Object.defineProperty(Boolean.prototype, Symbol.toPrimitive, {
        value: function (hint) {
            print('@@toPrimitive', hint);
            return 'PrimValue';
        },
        configurable: true
    });
    var val = true;
    print(+val);
    print("" + val);

    // If @@toPrimitive returns a non-primitive value, a TypeError is thrown.
    // For this test, plain buffers and lightfuncs are considered objects.
    try {
        var obj = { [Symbol.toPrimitive]: function (hint) {
                print('@@toPrimitive', this === obj, hint);
                return {};
            }
        };
        print(+obj);
    } catch (e) {
        print(e.name);
    }

    // @@toPrimitive is looked up using GetMethod() specification function
    // which treats undefined and null the same.
    [ void 0, null, 123, true, [], {} ].forEach(function (toprim, idx) {
        var obj = {
            [Symbol.toPrimitive]: toprim,
            valueOf: function () { return 1234; }
        };
        try {
            print(idx, +obj);
        } catch (e) {
            print(idx, e.name);
        }
    });

    // For Symbols ToPrimitive() coercion is based on Symbol.prototype's
    // @@toPrimitive.  It can be replaced and even removed in which case
    // the ordinary algorithm is used.  ToString() and ToNumber() are not
    // useful for accessing ToPrimitive() because the primitive Symbol
    // value will cause a TypeError.  ToPropertyKey() coercion is more
    // useful.
    var obj = Object(Symbol.for('foo'));
    var tmp = {};
    tmp[obj] = 123;
    print(tmp[Symbol.for('foo')]);
    Object.defineProperty(Symbol.prototype, Symbol.toPrimitive, {
        value: function (hint) {
            print('Symbol.prototype[@@toPrimitive]', hint);
            return 321;
        },
        configurable: true
    });
    var obj = Object(Symbol.for('foo'));
    var tmp = {};
    tmp[obj] = 123;
    print(tmp[Symbol.for('foo')]);
    print(tmp[321]);
    Object.defineProperty(Symbol.prototype, Symbol.toPrimitive, {
        writable: true
    });
    delete Symbol.prototype[Symbol.toPrimitive];
    Symbol.prototype.valueOf = function () {
        print('Symbol.prototype.valueOf');
        return 432;
    };
    Symbol.prototype.toString = function () {
        print('Symbol.prototype.toString');
        return 543;
    };
    var obj = Object(Symbol.for('foo'));
    var tmp = {};
    tmp[obj] = 123;
    print(+obj);
    print("" + obj);
    print(tmp[432]);
    print(tmp[543]);
}

try {
    basicTest();
} catch (e) {
    print(e.stack || e);
}
