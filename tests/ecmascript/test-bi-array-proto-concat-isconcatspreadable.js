/*===
[1,2,3,"foo","bar"]
[1,2,3,"foo","bar"]
[1,2,3,["foo","bar"]]
[1,2,3,{"0":"foo","1":"bar","2":"quux","3":"baz","length":3}]
[1,2,3,"foo","bar","quux"]
[1,2,3,{"0":"zero","1":"one","2":"two","length":3},"bar","quux"]
get Symbol(Symbol.isConcatSpreadable)
get length
has 0
get 0
has 1
get 1
has 2
get 2
[1,2,3,"foo","bar","quux"]
get Symbol(Symbol.isConcatSpreadable)
get length
has 0
get 0
has 1
has 2
[1,2,3,"foo",null,null]
6
[1,2,3,"foo","bar"]
[1,2,3,["foo","bar"]]
[1,2,3,["foo","bar"]]
[1,2,3,"foo","bar"]
[1,2,3,["foo","bar"]]
[1,2,3,"foo","bar"]
[1,2,3,["foo","bar"]]
[1,2,3,"foo","bar"]
[1,2,3,["foo","bar"]]
get @@isConcatSpreadable
[1,2,3,["foo","bar"]]
[1,2,3,"foobar"]
[1,2,3,4,5,6,[7,8]]
[1,2,3,4,5,6,[7,8]]
===*/

function printJson(x) {
    print(JSON.stringify(x));
}

function basicTest() {
    var arr, arg, res;

    // Normal concat.
    arr = [ 1, 2, 3 ];
    res = arr.concat('foo', 'bar');
    printJson(res);

    // Arrays are always concat spreadable by default.
    arr = [ 1, 2, 3 ];
    res = arr.concat([ 'foo', 'bar' ]);
    printJson(res);

    // Array concat spreadable behavior can be prevented using a symbol.
    arr = [ 1, 2, 3 ];
    arg = [ 'foo', 'bar' ];
    Object.defineProperty(arg, Symbol.isConcatSpreadable, { value: false });
    res = arr.concat(arg);
    printJson(res);

    // Custom objects can also be flattened using a symbol.
    arr = [ 1, 2, 3 ];
    arg = { 0: 'foo', 1: 'bar', 2: 'quux', 3: 'baz', length: 3 };
    res = arr.concat(arg);
    printJson(res);
    arr = [ 1, 2, 3 ];
    arg = { 0: 'foo', 1: 'bar', 2: 'quux', 3: 'baz', length: 3 };  // baz skipped
    Object.defineProperty(arg, Symbol.isConcatSpreadable, { value: true });
    res = arr.concat(arg);
    printJson(res);

    // Concat only spreads one level of the input.
    arr = [ 1, 2, 3 ];
    arg = { 0: 'foo', 1: 'bar', 2: 'quux', 3: 'baz', length: 3 };  // baz skipped
    Object.defineProperty(arg, Symbol.isConcatSpreadable, { value: true });
    arg[0] = { 0: 'zero', 1: 'one', 2: 'two', length: 3 };
    Object.defineProperty(arg[0], Symbol.isConcatSpreadable, { value: true });
    res = arr.concat(arg);
    printJson(res);

    // Proxy target.
    var proxy = new Proxy({}, {
        get: function (targ, key, recv) {
            print('get', String(key));
            if (key === Symbol.isConcatSpreadable) {
                return true;
            }
            if (String(key) === '0') {
                return 'foo';
            }
            if (String(key) === '1') {
                return 'bar';
            }
            if (String(key) === '2') {
                return 'quux';
            }
            if (key === 'length') {
                return 3;
            }
            return void 0;
        },
        has: function (targ, key) {
            print('has', String(key));
            if (key === Symbol.isConcatSpreadable) {
                return true;
            }
            if (String(key) === '0') {
                return true;
            }
            if (String(key) === '1') {
                return true;
            }
            if (String(key) === '2') {
                return true;
            }
            if (key === 'length') {
                return true;
            }
            return false;
        }
    });
    arr = [ 1, 2, 3 ];
    arg = proxy;
    res = arr.concat(arg);
    printJson(res);

    // Proxy with gaps; gaps are preserved because of 'has' trap calls.
    var proxy = new Proxy({}, {
        get: function (targ, key, recv) {
            print('get', String(key));
            if (key === Symbol.isConcatSpreadable) {
                return true;
            }
            if (String(key) === '0') {
                return 'foo';
            }
            if (String(key) === '1') {
                return 'bar';
            }
            if (String(key) === '2') {
                return 'quux';
            }
            if (key === 'length') {
                return 3;
            }
            return void 0;
        },
        has: function (targ, key) {
            print('has', String(key));
            if (key === Symbol.isConcatSpreadable) {
                return true;
            }
            if (String(key) === '0') {
                return true;
            }
            if (key === 'length') {
                return true;
            }
            return false;
        }
    });
    arr = [ 1, 2, 3 ];
    arg = proxy;
    res = arr.concat(arg);
    printJson(res);
    print(res.length);

    // @@isConcatSpreadable coercion: missing or undefined is treated as not
    // present, anything else (including null) is ToBoolean() coerced.
    [ void 0, null, false, true, 0, 1, '', 'foo' ].forEach(function (val) {
        arr = [ 1, 2, 3 ];
        arg = [ 'foo', 'bar' ];
        Object.defineProperty(arg, Symbol.isConcatSpreadable, { value: val });
        res = arr.concat(arg);
        printJson(res);
    });

    // @@isConcatSpreadable can be inherited.
    arr = [ 1, 2, 3 ];
    arg = [ 'foo', 'bar' ];
    proto = {};
    Object.setPrototypeOf(arg, proto);
    Object.defineProperty(proto, Symbol.isConcatSpreadable, { value: false });
    res = arr.concat(arg);
    printJson(res);

    // @@isConcatSpreadable can be a getter (in this test it's also inherited).
    arr = [ 1, 2, 3 ];
    arg = [ 'foo', 'bar' ];
    proto = {};
    Object.setPrototypeOf(arg, proto);
    Object.defineProperty(proto, Symbol.isConcatSpreadable, {
        get: function () {
            print('get @@isConcatSpreadable');
            return false;
        },
    });
    res = arr.concat(arg);
    printJson(res);

    // Primitive values are never concat spreadable, even if their prototype
    // has @@isConcatSpreadable.  Demonstrate using a plain string.
    arr = [ 1, 2, 3 ];
    arg = 'foobar';
    Object.defineProperty(String.prototype, Symbol.isConcatSpreadable, {
        value: true,
        configurable: true
    });
    res = arr.concat(arg);
    printJson(res);

    // IsArray() is used for detecting arrays, and must follow proxy chains
    arr = [ 1, 2, 3 ];
    res = arr.concat([ 4, 5, 6, [ 7, 8 ] ]);
    printJson(res);
    arr = [ 1, 2, 3 ];
    res = arr.concat(new Proxy([ 4, 5, 6, [ 7, 8 ] ], {}));
    printJson(res);
}

try {
    basicTest();
} catch (e) {
    printJson(e.stack || e);
}
