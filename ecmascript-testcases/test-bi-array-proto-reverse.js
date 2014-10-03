// XXX: shared test utils

function formatValue(v) {
    if (typeof v === 'function') {
        return 'function';
    }
    return typeof(v) + ':' + String(v);
}

function printDescriptor(obj, key) {
    var pd = Object.getOwnPropertyDescriptor(obj, key);
    if (!pd) {
        print('key=' + key, 'nonexistent');
        return;
    }

    print('key=' + key,
          'value=' + formatValue(pd.value),
          'writable=' + formatValue(pd.writable),
          'enumerable=' + formatValue(pd.enumerable),
          'configurable=' + formatValue(pd.configurable),
          'get=' + formatValue(pd.get),
          'set=' + formatValue(pd.set));
}

function dumpValue(x) {
    var i, n, clipped;
    var tmp=[];

    n = x.length;
    if (n > 1000) {
        n = 1000;
        clipped=true;
    }
    for (i = 0; i < n; i++) {
        if (x.hasOwnProperty(i)) {
            tmp.push(String(x[i]));
        } else {
            tmp.push('nonexistent');
        }
    }
    if (clipped) {
        tmp.push('...');
    }
    return tmp.join(',');
}

function test(this_value, args, suppress_print) {
    var t;
    if (args === undefined) {
        args = [];
    }

    try {
        t = Array.prototype.reverse.apply(this_value, args);
        if (suppress_print) {
            print('success');
        } else {
            if (t.length === undefined) {
                print(typeof t, 'nolength');
            } else {
                print(typeof t, t.length, dumpValue(t));
            }
        }
    } catch (e) {
        print(e.name);
    }
}

/*===
basic
object 0 
object 1 1
object 2 2,1
object 3 3,2,1
object 4 4,3,2,1
object 5 5,4,3,2,1
object 10 10,9,8,7,6,5,4,3,2,1
object 11 11,10,9,8,7,6,5,4,3,2,1
object 101 3,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,2,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,1
object 100 3,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,2,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,1
10000001 1 2
success
10000001 2 1
object 4 4,3,2,1
object 4 4,3,2,1
5 6
object 8 nonexistent,4,nonexistent,3,nonexistent,nonexistent,2,1
===*/

print('basic');

function basicTest() {
    var obj;

    // basic dense array tests (odd and even lengths)

    test([]);
    test([1]);
    test([1,2]);
    test([1,2,3]);
    test([1,2,3,4]);
    test([1,2,3,4,5]);
    test([1,2,3,4,5,6,7,8,9,10]);
    test([1,2,3,4,5,6,7,8,9,10,11]);

    // sparse

    obj = [1];
    obj[100] = 3;
    obj[50] = 2;
    test(obj);

    obj = [1];
    obj[99] = 3;
    obj[50] = 2;
    test(obj);

    // Extremely sparse array.  The standard algorithm spends a lot of
    // time on this.

    obj = [];
    obj[0] = 1;
    obj[10000000] = 2;
    print(obj.length, obj[0], obj[10000000]);
    test(obj, undefined, true);  // suppress printing
    print(obj.length, obj[0], obj[10000000]);

    // non-array

    obj = { '0': 1, '1': 2, '2': 3, '3': 4, length: 4 };
    test(obj);

    obj = { '0': 1, '1': 2, '2': 3, '3': 4, '4': 5, '5': 6, length: 4 };  // '4' and '5' are ignored
    test(obj);
    print(obj['4'], obj['5']);

    // combinations of upper/lower exists
    // exists: true true false false true false true false
    // reverse pairs: (true,false) (true,true) (false,false) (false,true)

    obj=[];
    obj[0] = 1;
    obj[1] = 2;
    obj[4] = 3;
    obj[6] = 4;
    obj.length = 8;
    test(obj);
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
attributes
true true false false false true false true
key=0 value=undefined:undefined writable=undefined:undefined enumerable=boolean:false configurable=boolean:true get=function set=function
key=1 value=undefined:undefined writable=undefined:undefined enumerable=boolean:false configurable=boolean:true get=function set=function
key=2 nonexistent
key=3 nonexistent
key=4 nonexistent
key=5 value=undefined:undefined writable=undefined:undefined enumerable=boolean:true configurable=boolean:true get=function set=function
key=6 nonexistent
key=7 value=undefined:undefined writable=undefined:undefined enumerable=boolean:true configurable=boolean:true get=function set=function
get 0
get 7
set 0 val7
set 7 val0
get 1
get 5
true false true false false false true true
key=0 value=undefined:undefined writable=undefined:undefined enumerable=boolean:false configurable=boolean:true get=function set=function
key=1 nonexistent
key=2 value=string:val5 writable=boolean:true enumerable=boolean:true configurable=boolean:true get=undefined:undefined set=undefined:undefined
key=3 nonexistent
key=4 nonexistent
key=5 nonexistent
key=6 value=string:val1 writable=boolean:true enumerable=boolean:true configurable=boolean:true get=undefined:undefined set=undefined:undefined
key=7 value=undefined:undefined writable=undefined:undefined enumerable=boolean:true configurable=boolean:true get=function set=function
key=0 value=string:foo writable=boolean:true enumerable=boolean:false configurable=boolean:true get=undefined:undefined set=undefined:undefined
key=1 nonexistent
key=0 value=string:foo writable=boolean:true enumerable=boolean:false configurable=boolean:true get=undefined:undefined set=undefined:undefined
key=1 nonexistent
===*/

print('attributes');

function attributesTest() {
    var t;

    // Since [[Get]] and [[Put]] are used, getter/setter calls are made normally.
    // When swapping two elements:
    //   - If both exists, call order is: lower get, upper get, lower set, upper set
    //   - If only lower exists: lower get; no other side effects (lower is deleted, upper is [[Put]]'d as a plain property)
    //   - If only upper exists: upper get; no other side effects (upper is deleted, lower is [[Put]]'d as a plain property)
    //   - If neither exists, no side effects
    //
    // Here:
    //   - 0 and 7: both exist
    //   - 1 and 6: 1 exists, 6 does not
    //   - 2 and 5: 5 exists, 2 does not
    //   - 3 and 4: neither exists

    t = [];
    t.my0 = 'val0';
    t.my1 = 'val1';
    t.my5 = 'val5';
    t.my7 = 'val7';
    Object.defineProperties(t, {
        '0': {
            enumerable: false,
            configurable: true,
            get: function() { print('get 0'); return this.my0; },
            set: function(v) { print('set 0', v); this.my0 = v; }
        },
        '1': {
            enumerable: false,
            configurable: true,
            get: function() { print('get 1'); return this.my1; },
            set: function(v) { print('set 1', v); this.my1 = v; }
        },
        // '2' skipped
        // '3' skipped
        // '4' skipped
        '5': {
            enumerable: true,
            configurable: true,
            get: function() { print('get 5'); return this.my5; },
            set: function(v) { print('set 5', v); this.my5 = v; }
        },
        // '6' skipped
        '7': {
            enumerable: true,
            configurable: true,
            get: function() { print('get 7'); return this.my7; },
            set: function(v) { print('set 7', v); this.my7 = v; }
        },
    });
    print(t.hasOwnProperty(0), t.hasOwnProperty(1),
          t.hasOwnProperty(2), t.hasOwnProperty(3),
          t.hasOwnProperty(4), t.hasOwnProperty(5),
          t.hasOwnProperty(6), t.hasOwnProperty(7));

    for (i = 0; i < 8; i++) {
        printDescriptor(t, String(i));
    }

    t.reverse();

    print(t.hasOwnProperty(0), t.hasOwnProperty(1),
          t.hasOwnProperty(2), t.hasOwnProperty(3),
          t.hasOwnProperty(4), t.hasOwnProperty(5),
          t.hasOwnProperty(6), t.hasOwnProperty(7));

    for (i = 0; i < 8; i++) {
        printDescriptor(t, String(i));
    }

    // Here, '0' is not enumerable, but when it gets swapped to position 1,
    // the specification algorithm calls [[Put]] with key "1" (and [[Delete]]
    // with key "0").  The property "1" will have default properties, and will
    // be enumerable.

    t = [];
    Object.defineProperties(t, {
        '0': { value: 'foo', writable: true, enumerable: false, configurable: true },
        'length': { value: 2 }
    });
    printDescriptor(t, '0');
    printDescriptor(t, '1');
    [].reverse();
    printDescriptor(t, '0');
    printDescriptor(t, '1');
}

try {
    attributesTest();
} catch (e) {
    print(e);
}

/*===
protected
1 2
TypeError
2 2
1 2
TypeError
1 2
1 1
TypeError
1 1
1 undefined
TypeError
1 undefined
undefined 1
TypeError
1 1
1 undefined
TypeError
undefined undefined
undefined 1
TypeError
undefined 1
===*/

/* All [[Get]], [[Put]], [[Delete]] operations have Throw=true */

print('protected');

function protectedTest() {
    var obj;

    // non-writable property; '1' is not writable but '0' must still get updated
    // ([[Put]] "0" happens before [[Put]] "1")

    obj = {};
    Object.defineProperties(obj, {
        '0': { value: 1, writable: true, enumerable: true, configurable: true },
        '1': { value: 2, writable: false, enumerable: true, configurable: true }
    });
    obj.length = 2;
    print(obj['0'], obj['1']);
    test(obj);
    print(obj['0'], obj['1']);

    // non-writable property; '0' is not writable, '1' must not get updated
    // ([[Put]] "0" happens before [[Put]] "1")

    obj = {};
    Object.defineProperties(obj, {
        '0': { value: 1, writable: false, enumerable: true, configurable: true },
        '1': { value: 2, writable: true, enumerable: true, configurable: true }
    });
    obj.length = 2;
    print(obj['0'], obj['1']);
    test(obj);
    print(obj['0'], obj['1']);

    // non-writable property; even though values match with SameValue, [[CanPut]]
    // should TypeError

    obj = {};
    Object.defineProperties(obj, {
        '0': { value: 1, writable: false, enumerable: true, configurable: true },
        '1': { value: 1, writable: false, enumerable: true, configurable: true }
    });
    obj.length = 2;
    print(obj['0'], obj['1']);
    test(obj);
    print(obj['0'], obj['1']);

    // non-configurable property, cannot be deleted; [[Delete]] for '0' happens
    // before [[Put]] for '1', so '1' should not be set

    obj = {};
    Object.defineProperties(obj, {
        '0': { value: 1, writable: true, enumerable: true, configurable: false }
    });
    obj.length = 2;
    print(obj['0'], obj['1']);
    test(obj);
    print(obj['0'], obj['1']);

    // non-configurable property, cannot be deleted; [[Put]] for '0' happens
    // before [[Delete]] for '1', so '1' *SHOULD* be set

    obj = {};
    Object.defineProperties(obj, {
        '1': { value: 1, writable: true, enumerable: true, configurable: false }
    });
    obj.length = 2;
    print(obj['0'], obj['1']);
    test(obj);
    print(obj['0'], obj['1']);

    // non-extensible object, new property cannot be added; [[Delete]] '0' happens
    // before [[Put]] '1', so [[Delete]] should succeed

    obj = {};
    Object.defineProperties(obj, {
        '0': { value: 1, writable: true, enumerable: true, configurable: true }
    });
    obj.length = 2;
    Object.preventExtensions(obj);
    print(obj['0'], obj['1']);
    test(obj);
    print(obj['0'], obj['1']);

    // non-extensible object, new property cannot be added; [[Put]] '0' happens
    // before [[Delete]] '1'; since [[Put]] fails, also won't [[Delete]]

    obj = {};
    Object.defineProperties(obj, {
        '1': { value: 1, writable: true, enumerable: true, configurable: true }
    });
    obj.length = 2;
    Object.preventExtensions(obj);
    print(obj['0'], obj['1']);
    test(obj);
    print(obj['0'], obj['1']);
}

try {
    protectedTest();
} catch (e) {
    print(e);
}

/*===
coercion
TypeError
TypeError
object nolength
object nolength
object nolength
object 0 
TypeError
object 3 3,2,1
object nolength
object 3 quux,bar,foo
success
3.9 quux bar foo baz
success
4294967299.9 quux bar foo baz
success
-4294967292.1 baz quux bar foo
success
4 quux bar foo baz
===*/

print('coercion');

function coercionTest() {
    var obj;

    // this coercion

    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('');     // length = 0 -> no action
    test('foo');  // has numeric properties but they're not writable/configurable -> TypeError
    test([1,2,3]);
    test({ foo: 1, bar: 2 });
    test({ '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 3 });

    // length coercion

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: '3.9' };
    test(obj, undefined, true);
    print(obj.length, obj['0'], obj['1'], obj['2'], obj['3']);

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 256*256*256*256 + 3.9 };
    test(obj, undefined, true);
    print(obj.length, obj['0'], obj['1'], obj['2'], obj['3']);

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: -256*256*256*256 + 3.9 };  // coerces to 4
    test(obj, undefined, true);
    print(obj.length, obj['0'], obj['1'], obj['2'], obj['3']);

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: {
        toString: function() { return 4; },
        valueOf: function() { return 3; }
    } };
    test(obj, undefined, true);
    print(obj.length, obj['0'], obj['1'], obj['2'], obj['3']);
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
