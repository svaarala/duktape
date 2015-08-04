// XXX: these should be shared utils for Array testing

function printDesc(obj, key) {
    var pd = Object.getOwnPropertyDescriptor(obj, key);
    print(JSON.stringify(pd));
}

function dumpValue(v) {
    var i, n;
    var res = '';

    if (v === undefined) {
        return 'undefined';
    } else if (v === null) {
        return 'null';
    }

    n = v.length;
    res = (typeof v) + ' ' + (typeof n) + ' ' + n;
    if (typeof n === 'undefined') {
        return res;
    }

    n = Math.floor(n);
    tmp = [];
    for (i = 0; i < n; i++) {
        if (!v.hasOwnProperty(i)) {
            tmp.push('nonexistent');
        } else {
            x = v[i];
            tmp.push(typeof x + ':' + x);
        }
    }
    res = res + (tmp.length > 0 ? ' ' + tmp.join(',') : '');
    return res;
}

function test(this_value, args) {
    var t;
    var pre, post, res;

    pre = dumpValue(this_value);
    try {
        t = Array.prototype.push.apply(this_value, args);
        res = typeof t + " " + t;
    } catch (e) {
        res = e.name;
    }
    post = dumpValue(this_value);
    print(pre, '-->', res, '-->', post);
}

/*===
basic
object number 2 number:1,number:2 --> number 2 --> object number 2 number:1,number:2
object number 2 number:1,number:2 --> number 3 --> object number 3 number:1,number:2,number:3
object number 2 number:1,number:2 --> number 5 --> object number 5 number:1,number:2,number:3,number:4,number:5
object number 2 number:1,number:2 --> number 4 --> object number 4 number:1,number:2,object:3,4,object:5,6
object string 5 nonexistent,nonexistent,nonexistent,nonexistent,nonexistent --> number 5 --> object number 5 nonexistent,nonexistent,nonexistent,nonexistent,nonexistent
4294967295
4294967300
9
3
length getter
length getter
length setter 8
length getter
object number 3 nonexistent,nonexistent,nonexistent --> number 8 --> object number 8 nonexistent,nonexistent,nonexistent,number:1,number:2,number:3,number:4,number:5
8
object number 1 string:foo --> TypeError --> object number 1 string:foo
object number 1 string:foo --> TypeError --> object number 1 string:foo
3 setter 3
3 getter
object number 1 string:foo --> number 6 --> object number 6 string:foo,number:1,number:2,string:setter-3,number:4,number:5
{"value":"foo","writable":true,"enumerable":true,"configurable":true}
{"value":1,"writable":true,"enumerable":true,"configurable":true}
{"value":2,"writable":true,"enumerable":false,"configurable":false}
{"enumerable":false,"configurable":false}
{"value":4,"writable":true,"enumerable":true,"configurable":true}
===*/

print('basic');

function basicTest() {
    // simple cases

    test([1,2], []);
    test([1,2], [3]);
    test([1,2], [3,4,5]);

    // arrays are not flattened like in concat

    test([1,2], [[3,4],[5,6]]);

    // zero length push(), still updates length, i.e. converts from
    // string to number here

    t = { length: '5' };
    test(t, []);

    // if the final length is outside 32-bit range, it will NOT wrap
    // because it's not coerced with ToUint32() again; it will wrap
    // on the next call!  (Not using test() here to avoid insane
    // debug dump :)

    t = { length: 256*256*256*256 - 1 };  // max array length
    print(t.length);
    Array.prototype.push.call(t, 1, 2, 3, 4, 5);
    print(t.length);  // -> 256*256*256*256 + 4
    Array.prototype.push.call(t, 1, 2, 3, 4, 5);
    print(t.length);  // -> 4 + 5 = 9

    // length side effect - only written once at the end
    // note that test() debug printing interferes with this

    t = {
        mylen: 3,

        get length() { print('length getter'); return this.mylen; },
        set length(n) { print('length setter', n); this.mylen = n; }
    };
    print(t.mylen);
    //Array.prototype.push.call(t, 1, 2, 3, 4, 5);
    test(t, [1, 2, 3, 4, 5]);
    print(t.mylen);

    // if multiple elements are inserted and a [[Put]] fails in the middle,
    // some elements may be inserted but 'length' is not updated at all.
    // e.g. here '1' and '2' will be written, [[Put]] to '3' fails, and
    // 'length' should remain 1.
    // (V8 3.7.12.22 will just skip the failed [[Put]])

    t = Object.create(Object.prototype, {
        '0': { value: 'foo', writable: true, enumerable: true, configurable: true },
        // '1' is empty
        // '2' is empty
        '3': { value: 'baz', writable: false, enumerable: false, configurable: false },
        'length': { value: 1, writable: true, enumerable: true, configurable: true }
    });
    test(t, [1, 2, 3, 4, 5]);

    // Since [[Put]] is used, an attempt to write to a non-writable property is always
    // a TypeError, even if SameValue(oldVal, newVal) is true.  This is the case
    // because [[Put]] first calls [[CanPut]] which refuses an attempt to write
    // to a non-writable data property.

     t = Object.create(Object.prototype, {
        '0': { value: 'foo', writable: true, enumerable: true, configurable: true },
        '1': { value: 'bar', writable: false, enumerable: false, configurable: false },
        'length': { value: 1, writable: true, enumerable: true, configurable: true }
    });
    test(t, ['bar']);

    // Since [[Put]] is used, existing elements keep their attributes while new
    // elements get default [[Put]] attributes (writable, enumerable, configurable),
    // and setters get called.

    t = Object.create(Object.prototype, {
        '0': { value: 'foo', writable: true, enumerable: true, configurable: true },
        // '1' is empty (new element)
        '2': { value: 'quux', writable: true, enumerable: false, configurable: false },  // attributes kept
        '3': { get: function() { print('3 getter'); return this.my3; },
               set: function(v) { print('3 setter', v); this.my3 = 'setter-' + v; } },
        'length': { value: 1, writable: true, enumerable: true, configurable: true }
    });
    t.my3 = 'baz';
    test(t, [1, 2, 3, 4, 5]);
    printDesc(t, '0');
    printDesc(t, '1');
    printDesc(t, '2');
    printDesc(t, '3');
    printDesc(t, '4');
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
coercion
undefined --> TypeError --> undefined
null --> TypeError --> null
boolean undefined undefined --> number 0 --> boolean undefined undefined
boolean undefined undefined --> number 0 --> boolean undefined undefined
number undefined undefined --> number 0 --> number undefined undefined
string number 3 string:f,string:o,string:o --> TypeError --> string number 3 string:f,string:o,string:o
object number 2 number:1,number:2 --> number 2 --> object number 2 number:1,number:2
object undefined undefined --> number 0 --> object number 0
object number 2 string:foo,string:bar --> number 2 --> object number 2 string:foo,string:bar
===*/

print('coercion');

function coercionTest() {
    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('foo');
    test([1,2]);
    test({ foo: 1, bar: 2 });
    test({ '0': 'foo', '1': 'bar', length: 2 });

    // coercion: ToObject(this), [[Get]] "length", ToUint32(length)
}

try {
    coercionTest();
} catch (e) {
    print(e);
}

/*===
non-extensible
object number 3 number:1,number:2,number:3 --> TypeError --> object number 3 number:1,number:2,number:3
object number 3 string:foo,string:bar,string:quux --> TypeError --> object number 3 string:foo,string:bar,string:quux
===*/

print('non-extensible');

function nonExtensibleTest() {
    var t;

    // non-extensible array

    t = [1,2,3];
    Object.preventExtensions(t);
    test(t, [ 4 ]);

    // non-extensible object

    t = { '0': 'foo', '1': 'bar', '2': 'quux', length: 3 };
    Object.preventExtensions(t);
    test(t, [ 1 ]);
}

try {
    nonExtensibleTest();
} catch (e) {
    print(e);
}
