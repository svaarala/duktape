/*
 *  Some ES2015 (and ES2016) enumeration order tests
 *
 *  https://github.com/svaarala/duktape/pull/1054
 *
 *  Duktape 2.x applies the ES2015 [[OwnPropertyKeys]] enumeration order also
 *  in for-in, Object.keys(), and duk.enum(), even though neither ES2015 nor
 *  ES2016 requires it.
 */

/*---
{
    "custom": true
}
---*/

/*===
2
3
foo
bar
quux
2
foo
bar
4
0 yes
2 yes
3 yes
4294967292 yes
4294967293 yes
4294967294 yes
0.0 no
-0 no
0.1 no
4294967297 no
4294967296 no
4294967295 no
0
4
3
2
1
3
0
1
2
4
0
3
1
2
4
0
3
1
2
4
["a","b","foo","d","e"]
0 a
1 b
3 d
4 e
2 foo
a
b
foo
d
e
===*/

function test() {
    var a, b, c, d, e;
    var k;

    // Basic case: for own properties array indices come first, followed by
    // keys in insert order.
    a = { foo: 1, 2: 'bar', 3: 'quux', bar: 4, quux: 5 };
    for (k in a) { print(k); }

    // When properties are inherited, the sorting rule is applied to each
    // inheritance level in turn (eliminating duplicates).
    a = { 4: 'foo', foo: 'shadowed', 2: 'shadowed' };
    b = { foo: 1, 2: 'bar', bar: 'quux' };
    b.__proto__ = a;
    for (k in b) { print(k); }

    // Array indices are recognized specially as canonical strings for
    // integers [0,2**32-2].
    a = { 0: 'yes', '0.0': 'no', '-0': 'no', '0.1': 'no', 3: 'yes', 2: 'yes',
          4294967297: 'no', 4294967296: 'no', 4294967295: 'no',
          4294967294: 'yes', 4294967293: 'yes', 4294967292: 'yes' };
    for (k in a) { print(k, a[k]); }

    // Another example, multiple inheritance levels.
    a = { 1: 'foo' }; b = { 2: 'foo' }; c = { 3: 'foo' }; d = { 4: 'foo' }; e = [ 'foo' ];
    e.__proto__ = d; d.__proto__ = c; c.__proto__ = b; b.__proto__ = a;
    for (k in e) { print(k); }

    // Array inheritance.
    a = [ 1,1,1,1,1 ]; b = { 3: 'foo' };
    b.__proto__ = a;
    for (k in b) { print(k); }

    // Array gaps.
    a = [ 1,1,1,1,1 ]; b = [1,,,1];
    b.__proto__ = a;
    for (k in b) { print(k); }

    // Parent properties are array-like and get sorted.
    a = { 4: true, 0: true, 3: true, 1: true, 2: true }; b = [1,,,1];
    b.__proto__ = a;
    for (k in b) { print(k); }

    // Array with gap; JSON.stringify() must include inherited properties
    // (this differs from how objects are handled, as they only serialize
    // own properties).
    a = { 2: 'foo' }; b = ['a','b',,'d','e']; b.__proto__ = a;
    print(JSON.stringify(b));

    // For same Array with gap case, Array.prototype.forEach() must see
    // the keys in sorted order regardless of whether keys are inherited.
    // So this order differs from for-in!
    a = { 2: 'foo' }; b = ['a','b',,'d','e']; b.__proto__ = a;
    for (k in b) { print(k, b[k]); }
    Array.prototype.forEach.call(b, function (v) { print(v); });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}

/*===
all done
===*/

function randomOrderTest() {
    var i;
    var vals;
    var obj;
    var t;

    for (i = 0; i < 1e4; i++) {
        obj = {};
        vals = [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ];
        obj.foo = 1;
        while (vals.length > 0) {
            t = vals.splice(Math.floor(Math.random() * vals.length), 1);
            obj[t[0]] = 'foo';
        }
        obj.bar = 1;
        vals = [];
        for (var k in obj) { vals.push(k); }
        if (JSON.stringify(vals) !== '["0","1","2","3","4","5","6","7","8","9","foo","bar"]') {
            throw new Error('failed: ' + JSON.stringify(vals));
        }
    }

    print('all done');
}

try {
    randomOrderTest();
} catch (e) {
    print(e.stack || e);
}
