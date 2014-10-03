// XXX: these should be shared utils for Array testing

function dumpValue(v) {
    var i, n, n_clipped;
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

    // clip dump length (n may be > 2**32)
    n = Math.floor(n);
    n_clipped = Math.min(n, 200);
    tmp = [];
    for (i = 0; i < n_clipped; i++) {
        if (!v.hasOwnProperty(i)) {
            tmp.push('nonexistent');
        } else {
            x = v[i];
            tmp.push(typeof x + ':' + x);
        }
    }
    res = res + (tmp.length > 0 ? ' ' + tmp.join(',') : '') + (n > n_clipped ? ',...' : '');
    return res;
}

function test(this_value, loop_count) {
    var t;
    var pre, post, res;

    if (loop_count === undefined) {
        loop_count = 1;
    }

    while (loop_count-- > 0) {
        pre = dumpValue(this_value);
        try {
            t = Array.prototype.pop.call(this_value);
            res = typeof t + " " + t;
        } catch (e) {
            res = e.name;
        }
        post = dumpValue(this_value);
        print(pre, '-->', res, '-->', post);
    }
}

/*===
basic
object number 3 number:1,number:2,number:3 --> number 3 --> object number 2 number:1,number:2
object number 2 number:1,number:2 --> number 2 --> object number 1 number:1
object number 1 number:1 --> number 1 --> object number 0
object number 0 --> undefined undefined --> object number 0
object number 0 --> undefined undefined --> object number 0
object number 4 number:1,nonexistent,nonexistent,number:2 --> number 2 --> object number 3 number:1,nonexistent,nonexistent
object number 3 number:1,nonexistent,nonexistent --> undefined undefined --> object number 2 number:1,nonexistent
object number 2 number:1,nonexistent --> undefined undefined --> object number 1 number:1
object number 1 number:1 --> number 1 --> object number 0
object number 0 --> undefined undefined --> object number 0
object number 0 --> undefined undefined --> object number 0
object number 11 number:1,nonexistent,nonexistent,nonexistent,nonexistent,number:2,nonexistent,nonexistent,nonexistent,nonexistent,number:3 --> number 3 --> object number 10 number:1,nonexistent,nonexistent,nonexistent,nonexistent,number:2,nonexistent,nonexistent,nonexistent,nonexistent
object number 10 number:1,nonexistent,nonexistent,nonexistent,nonexistent,number:2,nonexistent,nonexistent,nonexistent,nonexistent --> undefined undefined --> object number 9 number:1,nonexistent,nonexistent,nonexistent,nonexistent,number:2,nonexistent,nonexistent,nonexistent
object number 9 number:1,nonexistent,nonexistent,nonexistent,nonexistent,number:2,nonexistent,nonexistent,nonexistent --> undefined undefined --> object number 8 number:1,nonexistent,nonexistent,nonexistent,nonexistent,number:2,nonexistent,nonexistent
object number 8 number:1,nonexistent,nonexistent,nonexistent,nonexistent,number:2,nonexistent,nonexistent --> undefined undefined --> object number 7 number:1,nonexistent,nonexistent,nonexistent,nonexistent,number:2,nonexistent
object number 7 number:1,nonexistent,nonexistent,nonexistent,nonexistent,number:2,nonexistent --> undefined undefined --> object number 6 number:1,nonexistent,nonexistent,nonexistent,nonexistent,number:2
object number 6 number:1,nonexistent,nonexistent,nonexistent,nonexistent,number:2 --> number 2 --> object number 5 number:1,nonexistent,nonexistent,nonexistent,nonexistent
object number 5 number:1,nonexistent,nonexistent,nonexistent,nonexistent --> undefined undefined --> object number 4 number:1,nonexistent,nonexistent,nonexistent
object number 4 number:1,nonexistent,nonexistent,nonexistent --> undefined undefined --> object number 3 number:1,nonexistent,nonexistent
object number 3 number:1,nonexistent,nonexistent --> undefined undefined --> object number 2 number:1,nonexistent
object number 2 number:1,nonexistent --> undefined undefined --> object number 1 number:1
object number 1 number:1 --> number 1 --> object number 0
object number 0 --> undefined undefined --> object number 0
object number 0 --> undefined undefined --> object number 0
object number 3 number:1,string:2-undel,number:3 --> number 3 --> object number 2 number:1,string:2-undel
object number 2 number:1,string:2-undel --> TypeError --> object number 2 number:1,string:2-undel
object number 2 number:1,string:2-undel --> TypeError --> object number 2 number:1,string:2-undel
object number 2 number:1,string:2-undel --> TypeError --> object number 2 number:1,string:2-undel
object string 10 string:first,string:second,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent --> undefined undefined --> object number 9 string:first,string:second,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent
object number 9 string:first,string:second,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent --> undefined undefined --> object number 8 string:first,string:second,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent
object number 8 string:first,string:second,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent --> undefined undefined --> object number 7 string:first,string:second,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent
object number 7 string:first,string:second,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent --> undefined undefined --> object number 6 string:first,string:second,nonexistent,nonexistent,nonexistent,nonexistent
object number 6 string:first,string:second,nonexistent,nonexistent,nonexistent,nonexistent --> undefined undefined --> object number 5 string:first,string:second,nonexistent,nonexistent,nonexistent
object number 5 string:first,string:second,nonexistent,nonexistent,nonexistent --> undefined undefined --> object number 4 string:first,string:second,nonexistent,nonexistent
object number 4 string:first,string:second,nonexistent,nonexistent --> undefined undefined --> object number 3 string:first,string:second,nonexistent
object number 3 string:first,string:second,nonexistent --> undefined undefined --> object number 2 string:first,string:second
object number 2 string:first,string:second --> string second --> object number 1 string:first
object number 1 string:first --> string first --> object number 0
object number 0 --> undefined undefined --> object number 0
object number 0 --> undefined undefined --> object number 0
===*/

print('basic');

function basicTest() {
    var t;

    // dense array

    t = [1,2,3];
    test(t, 5);

    t = [1,,,2];
    test(t, 6);

    // sparse array

    t = [1];
    t[100] = 'x';   // use this to get into 'sparse mode' but use smaller length to minimize output
    t.length = 1;
    t[10] = 3;
    t[5] = 2;
    test(t, 13);

    // non-deletable element

    t = [ 1, 2, 3 ];
    Object.defineProperties(t, {
        '1': { value: '2-undel', writable: false, enumerable: true, configurable: false }
    });
    test(t, 4);

    // trailing non-existent elements

    t = { '0': 'first', '1': 'second', length: '10' };
    test(t, 12);
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
boolean undefined undefined --> undefined undefined --> boolean undefined undefined
boolean undefined undefined --> undefined undefined --> boolean undefined undefined
number undefined undefined --> undefined undefined --> number undefined undefined
string number 3 string:f,string:o,string:o --> TypeError --> string number 3 string:f,string:o,string:o
string number 3 string:f,string:o,string:o --> TypeError --> string number 3 string:f,string:o,string:o
string number 3 string:f,string:o,string:o --> TypeError --> string number 3 string:f,string:o,string:o
string number 3 string:f,string:o,string:o --> TypeError --> string number 3 string:f,string:o,string:o
object number 2 number:1,number:2 --> number 2 --> object number 1 number:1
object undefined undefined --> undefined undefined --> object number 0
object number 2 string:foo,string:bar --> string bar --> object number 1 string:foo
object string 0.9 --> undefined undefined --> object number 0
object number 5 string:first,string:second,nonexistent,nonexistent,nonexistent --> undefined undefined --> object number 4 string:first,string:second,nonexistent,nonexistent
object number 4 string:first,string:second,nonexistent,nonexistent --> undefined undefined --> object number 3 string:first,string:second,nonexistent
object number 3 string:first,string:second,nonexistent --> undefined undefined --> object number 2 string:first,string:second
object number 2 string:first,string:second --> string second --> object number 1 string:first
object number 1 string:first --> string first --> object number 0
object number 0 --> undefined undefined --> object number 0
object number 0 --> undefined undefined --> object number 0
object number 4294967301 string:first,string:second,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,... --> undefined undefined --> object number 4 string:first,string:second,nonexistent,nonexistent
object number 4 string:first,string:second,nonexistent,nonexistent --> undefined undefined --> object number 3 string:first,string:second,nonexistent
object number 3 string:first,string:second,nonexistent --> undefined undefined --> object number 2 string:first,string:second
object number 2 string:first,string:second --> string second --> object number 1 string:first
object number 1 string:first --> string first --> object number 0
object number 0 --> undefined undefined --> object number 0
object number 0 --> undefined undefined --> object number 0
length getter 3
1 getter
length getter 3
length setter 2
length getter 2
1 getter
object number 3 string:foo,string:bar,nonexistent --> undefined undefined --> object number 2 string:foo,string:bar
length getter 2
1 getter
length getter 2
1 getter
length setter 1
length getter 1
object number 2 string:foo,string:bar --> string bar --> object number 1 string:foo
length getter 1
length getter 1
length setter 0
length getter 0
object number 1 string:foo --> string foo --> object number 0
length getter 0
length getter 0
length setter 0
length getter 0
object number 0 --> undefined undefined --> object number 0
length getter 0
length getter 0
length setter 0
length getter 0
object number 0 --> undefined undefined --> object number 0
===*/

print('coercion');

function coercionTest() {
    // this coercion

    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('foo', 4);  // character are non-configurable
    test([1,2]);
    test({ foo: 1, bar: 2 });
    test({ '0': 'foo', '1': 'bar', length: 2 });

    // ToUint32('0.9') -> coerces to zero, return undefined, length will be written to number 0

    t = { length: '0.9' };
    test(t);

    // length goes through ToUint32(), so these should have no difference
    // (V8 3.7.12.22 seems to iterate through 4G elements here)

    t = { '0': 'first', '1': 'second', length: 5 };
    test(t, 7);
    t = { '0': 'first', '1': 'second', length: 256*256*256*256 + 5 };
    test(t, 7);

    // complex test with lots of side effect checks
    // note that pre/post printing confuses the output a bit

    t = Object.create(Object.prototype, {
        '0': {
            value: 'foo',
            writable: true,
            enumerable: true,
            configurable: true
        },
        '1': {
            get: function() {
                print('1 getter');
                return 'bar';
            },
            set: function() {
                print('1 setter');
            },
            enumerable: true,
            configurable: true
        },
        'length': {
            get: function() {
                print('length getter', this.mylen);
                return this.mylen;
            },
            set: function(v) {
                print('length setter', v);
                this.mylen = v;
            },
            enumerable: true,
            configurable: true
        }
    });
    t.mylen = 3;

    test(t, 5);
}

try {
    coercionTest();
} catch (e) {
    print(e);
}

/*===
delete/put prevented
object string 0 --> TypeError --> object string 0
object string 2 nonexistent,nonexistent --> TypeError --> object string 2 nonexistent,nonexistent
string number 3 string:f,string:o,string:o --> TypeError --> string number 3 string:f,string:o,string:o
object number 2 string:foo,string:bar --> TypeError --> object number 2 string:foo,string:bar
object number 2 string:foo,string:bar --> TypeError --> object number 2 string:foo,string:bar
object number 2 string:foo,string:bar --> TypeError --> object number 2 string:foo,string:bar
object number 2 string:foo,string:bar --> TypeError --> object number 2 string:foo,string:bar
object number 2 string:foo,string:bar --> TypeError --> object number 2 string:foo,string:bar
object number 2 string:foo,string:bar --> TypeError --> object number 2 string:foo,string:bar
===*/

/* Attempt to [[Delete]] entries or [[Put]] to "length" have the "Throw" flag
 * set in both non-strict and strict mode.  If attributes prevent deletion or
 * writing, a TypeError must result even in non-strict mode.
 */

print('delete/put prevented');

function deleteOrPutPreventedTest() {
    var t;

    // Length is zero and non-writable

    t = Object.create(Object.prototype, {
        'length': { value: '0', writable: false, enumerable: true, configurable: true }
    });
    test(t, 1);

    // Length is non-zero and non-writable

    t = Object.create(Object.prototype, {
        'length': { value: '2', writable: false, enumerable: true, configurable: true }
    });
    test(t, 1);

    // Characters are non-configurable, which causes [[Delete]] to fail

    test('foo', 1);

    // Same for this object; because the [[Put]] should fail with TypeError,
    // the (writable) length should not be updated

    t = Object.create(Object.prototype, {
        '0': { value: 'foo', writable: false, enumerable: true, configurable: false },
        '1': { value: 'bar', writable: false, enumerable: true, configurable: false },
        'length': { value: 2, writable: true, enumerable: true, configurable: true }
    });
    test(t, 3);

    // Length is not writable either but should have no effect, TypeError on attempt to [[Delete]] "1"

    t = Object.create(Object.prototype, {
        '0': { value: 'foo', writable: false, enumerable: true, configurable: false },
        '1': { value: 'bar', writable: false, enumerable: true, configurable: false },
        'length': { value: 2, writable: false, enumerable: true, configurable: false }
    });
    test(t, 3);
}

try {
    deleteOrPutPreventedTest();
} catch (e) {
    print(e);
}
