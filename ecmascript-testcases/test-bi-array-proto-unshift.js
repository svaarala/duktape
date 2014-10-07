// XXX: util
function printDesc(obj, key) {
    var pd = Object.getOwnPropertyDescriptor(obj, key);
    if (!pd) {
        print(key, 'nonexistent');
        return;
    }
    print(key,
          'value=' + pd.value,
          'writable=' + pd.writable,
          'enumerable=' + pd.enumerable,
          'configurable=' + pd.configurable,
          'get=' + typeof pd.get,
          'set=' + typeof pd.set);
}

function dumpValue(v) {
    var i, n;
    var tmp = [];

    if (v === undefined) {
        return 'undefined';
    } else if (v === null) {
        return 'null';
    }

    n = Math.floor(v.length);
    for (i = 0; i < n; i++) {
        if (v.hasOwnProperty(i)) {
            tmp.push(v[i]);
        } else {
            tmp.push('nonexistent');
        }
    }
    return typeof v + ' ' + v.length + ' ' + tmp.join(',');
}

function test(this_value, args, no_pre_post) {
    var t;
    if (args === undefined) {
        args = [];
    }

    try {
        if (!no_pre_post) {
            print('pre', dumpValue(this_value));
        }
        t = Array.prototype.unshift.apply(this_value, args);
        print(typeof t, t);
    } catch (e) {
        print(e.name);
    }

    if (!no_pre_post) {
        print('post', dumpValue(this_value));
    }
}

/*===
basic
pre object 0 
number 0
post object 0 
pre object 1 1
number 1
post object 1 1
pre object 2 1,2
number 2
post object 2 1,2
pre object 3 1,2,3
number 3
post object 3 1,2,3
pre object 0 
number 1
post object 1 foo
pre object 1 1
number 2
post object 2 foo,1
pre object 2 1,2
number 3
post object 3 foo,1,2
pre object 3 1,2,3
number 4
post object 4 foo,1,2,3
pre object 0 
number 2
post object 2 foo,bar
pre object 1 1
number 3
post object 3 foo,bar,1
pre object 2 1,2
number 4
post object 4 foo,bar,1,2
pre object 3 1,2,3
number 5
post object 5 foo,bar,1,2,3
pre object 0 
number 3
post object 3 foo,bar,quux
pre object 1 1
number 4
post object 4 foo,bar,quux,1
pre object 2 1,2
number 5
post object 5 foo,bar,quux,1,2
pre object 3 1,2,3
number 6
post object 6 foo,bar,quux,1,2,3
pre object 101 1,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,2,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,3
number 103
post object 103 foo,bar,1,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,2,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,3
pre object 101 1,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,2,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,3
number 102
post object 102 foo,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,baz,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,quux,1,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,2,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,3
pre object 3 1,2,3
number 5
post object 5 foo,bar,1,2,3
pre object 3 1,2,3
number 5
post object 5 foo,bar,1,2,3
pre object 10 nonexistent,nonexistent,3,nonexistent,nonexistent,6,nonexistent,nonexistent,nonexistent,nonexistent
number 12
post object 12 foo,bar,nonexistent,nonexistent,3,nonexistent,nonexistent,6,nonexistent,nonexistent,nonexistent,nonexistent
false undefined
pre object undefined 
number 0
post object 0 
true 0
===*/

print('basic');

function basicTest() {
    var obj, obj2;

    // basic dense array tests

    test([]);
    test([1]);
    test([1,2]);
    test([1,2,3]);

    test([], ['foo']);
    test([1], ['foo']);
    test([1,2], ['foo']);
    test([1,2,3], ['foo']);

    test([], ['foo','bar']);
    test([1], ['foo','bar']);
    test([1,2], ['foo','bar']);
    test([1,2,3], ['foo','bar']);

    test([], ['foo','bar','quux']);
    test([1], ['foo','bar','quux']);
    test([1,2], ['foo','bar','quux']);
    test([1,2,3], ['foo','bar','quux']);

    // sparse array tests

    obj=[1]; obj[100]=3; obj[50]=2;
    test(obj, ['foo', 'bar']);

    obj=[1]; obj[100]=3; obj[50]=2;
    obj2=['foo']; obj2[100]='quux'; obj2[50]='baz';
    test(obj, [obj2]);  // obj2 is inserted "as is"

    // non-array tests

    obj = { '0': '1', '1': '2', '2': '3', length: 3 };  // basic
    test(obj, ['foo','bar']);

    obj = { '0': '1', '1': '2', '2': '3', '3': 'out-4', '4': 'out-5', '5': 'out-6', length: 3 };  // out of length range elements
    test(obj, ['foo','bar']);

    obj = { '2': '3', '5': '6', length: 10 };  // sparse this
    test(obj, ['foo','bar']);

    // if a non-array has no 'length', it will be treated like zero length
    // (ToUint32(undefined) = 0); in particular, step 10 writes a length field
    // to the object

    obj = {};
    print(obj.hasOwnProperty('length'), obj.length);
    test(obj);
    print(obj.hasOwnProperty('length'), obj.length);
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
attributes
2 getter
3 getter
pre object 4 1,2,3,4
3 getter
2 getter
3 setter 2
2 setter 1
number 6
2 getter
3 getter
post object 6 foo,bar,1,2,3,4
2 getter
3 getter
6 foo bar 1 2 3 4
length value=6 writable=true enumerable=true configurable=true get=undefined set=undefined
0 value=foo writable=true enumerable=false configurable=false get=undefined set=undefined
1 value=bar writable=true enumerable=true configurable=true get=undefined set=undefined
2 value=undefined writable=undefined enumerable=false configurable=false get=function set=function
3 value=undefined writable=undefined enumerable=true configurable=true get=function set=function
4 value=3 writable=true enumerable=true configurable=true get=undefined set=undefined
5 value=4 writable=true enumerable=true configurable=true get=undefined set=undefined
pre object 6 1,2,nonexistent,4,nonexistent,6
number 9
post object 9 foo,bar,quux,1,2,nonexistent,4,nonexistent,6
9 foo bar quux 1 2 undefined 4 undefined 6
length value=9 writable=true enumerable=true configurable=true get=undefined set=undefined
0 value=foo writable=true enumerable=false configurable=true get=undefined set=undefined
1 value=bar writable=true enumerable=false configurable=true get=undefined set=undefined
2 value=quux writable=true enumerable=true configurable=true get=undefined set=undefined
3 value=1 writable=true enumerable=false configurable=true get=undefined set=undefined
4 value=2 writable=true enumerable=true configurable=true get=undefined set=undefined
5 nonexistent
6 value=4 writable=true enumerable=true configurable=true get=undefined set=undefined
7 nonexistent
8 value=6 writable=true enumerable=true configurable=true get=undefined set=undefined
===*/

print('attributes');

function attributesTest() {
    var obj;

    // Step 6 makes room for unshifted value by copying values upwards,
    // but values in the empty space are not [[Delete]]'d, so the slots
    // will retain their attributes (and e.g. accessors remain accessors).
    // The elements can also be non-configurable because they're not deleted.

    obj = {}
    obj.my2 = 3;
    obj.my3 = 4;
    Object.defineProperties(obj, {
        '0': { value: 1, writable: true, enumerable: false, configurable: false },
        '1': { value: 2, writable: true, enumerable: true, configurable: true },
        '2': { get: function() { print('2 getter'); return this.my2; },
               set: function(v) { print('2 setter', v); this.my2 = v; },
               enumerable: false, configurable: false },
        '3': { get: function() { print('3 getter'); return this.my3; },
               set: function(v) { print('3 setter', v); this.my3 = v; },
               enumerable: true, configurable: true }
    });
    obj.length = 4;
    test(obj, ['foo','bar']);
    print(obj.length, obj[0], obj[1], obj[2], obj[3], obj[4], obj[5]);
    printDesc(obj, 'length');
    printDesc(obj, '0');
    printDesc(obj, '1');
    printDesc(obj, '2');
    printDesc(obj, '3');
    printDesc(obj, '4');
    printDesc(obj, '5');

    // Step 6 copies values upwards by the required amount, e.g. if 3 values
    // are unshifted, len-1 is copied to len+2, len-2 is copied to len+1,
    // and so on, '0' is copied to '3'.  If the source exists, a [[Get]]+[[Put]]
    // sequence is used; otherwise a [[Delete]] is used.  When both the source
    // and destination exist, the destination will retain its attributes.
    //
    // As above, the elements in the "room" for unshift are not [[Delete]]'d,
    // so they retain their attributes.

    obj = {}
    Object.defineProperties(obj, {
        // 0->3: both exist
        // 1->4: source exists, target does not
        // 2->5: source does not exist, target does
        // 3->6: source exists, target does not
        // 4->7: neither exist
        // 5->8: source exists, target does not

        '0': { value: 1, writable: true, enumerable: false, configurable: true },
        '1': { value: 2, writable: true, enumerable: false, configurable: true },
        // 2 does not exist
        '3': { value: 4, writable: true, enumerable: false, configurable: true },
        // 4 does not exist
        '5': { value: 6, writable: true, enumerable: false, configurable: true },
    });
    obj.length = 6;
    test(obj, ['foo','bar','quux']);
    print(obj.length, obj[0], obj[1], obj[2], obj[3], obj[4], obj[5], obj[6], obj[7], obj[8]);
    printDesc(obj, 'length');
    for (i = 0; i < 9; i++) {
        printDesc(obj, String(i));
    }
}

try {
    attributesTest();
} catch (e) {
    print(e);
}

/*===
mutation
===*/

print('mutation');

function mutationTest() {
    // XXX
}

try {
    mutationTest();
} catch (e) {
    print(e);
}

/*===
protected
false undefined
pre object undefined 
TypeError
post object undefined 
false undefined
===*/

print('protected');

function protectedTest() {
    var obj;

    // non-extensible object without 'length'; a 'length' property
    // cannot be added

    obj = {};
    Object.preventExtensions(obj);
    print(obj.hasOwnProperty('length'), obj.length);
    test(obj);
    print(obj.hasOwnProperty('length'), obj.length);

    // XXX: copy loop, side effects
    // XXX: copy loop, protected

    // final length is not updated until step 10 (mostly matters for
    // non-arrays)
}

try {
    protectedTest();
} catch (e) {
    print(e.name);
}

/*===
coercion
pre undefined
TypeError
post undefined
pre null
TypeError
post null
pre boolean undefined 
number 0
post boolean undefined 
pre boolean undefined 
number 0
post boolean undefined 
pre number undefined 
number 0
post number undefined 
pre string 3 f,o,o
TypeError
post string 3 f,o,o
pre object 2 1,2
number 2
post object 2 1,2
pre object undefined 
number 0
post object 0 
string 3.9 foo bar quux baz undefined undefined undefined
number 5
number 5 1 2 foo bar quux undefined undefined
number 4294967299.9 foo bar quux baz undefined undefined undefined
number 5
number 5 1 2 foo bar quux undefined undefined
number -4294967292.1 foo bar quux baz undefined undefined undefined
number 6
number 6 1 2 foo bar quux baz undefined
length toString
object 3 foo bar quux baz undefined undefined undefined
length valueOf
number 5
number 5 1 2 foo bar quux undefined undefined
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
    test('foo');  // TypeError, because attempt to write non-writable length must fail
    test([1,2]);
    test({ foo: 1, bar: 2 });

    // length coercion

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: '3.9' };
    print(typeof obj.length, obj.length, obj[0], obj[1], obj[2], obj[3], obj[4], obj[5], obj[6]);
    test(obj, [1,2], true);
    print(typeof obj.length, obj.length, obj[0], obj[1], obj[2], obj[3], obj[4], obj[5], obj[6]);

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 256*256*256*256 + 3.9 };
    print(typeof obj.length, obj.length, obj[0], obj[1], obj[2], obj[3], obj[4], obj[5], obj[6]);
    test(obj, [1,2], true);
    print(typeof obj.length, obj.length, obj[0], obj[1], obj[2], obj[3], obj[4], obj[5], obj[6]);

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: -256*256*256*256 + 3.9 };
    print(typeof obj.length, obj.length, obj[0], obj[1], obj[2], obj[3], obj[4], obj[5], obj[6]);
    test(obj, [1,2], true);
    print(typeof obj.length, obj.length, obj[0], obj[1], obj[2], obj[3], obj[4], obj[5], obj[6]);

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: {
        toString: function() { print('length toString'); return 3; },
        valueOf: function() { print('length valueOf'); return 3; },
    } };
    print(typeof obj.length, obj.length, obj[0], obj[1], obj[2], obj[3], obj[4], obj[5], obj[6]);
    test(obj, [1,2], true);
    print(typeof obj.length, obj.length, obj[0], obj[1], obj[2], obj[3], obj[4], obj[5], obj[6]);
}

try {
    coercionTest();
} catch (e) {
    print(e.name);
}
