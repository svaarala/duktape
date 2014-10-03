function test(this_value, args) {
    var t;

    try {
        t = Array.prototype.lastIndexOf.apply(this_value, args);
        print(typeof t, t);
    } catch (e) {
        print(e.name);
    }
}

/*===
basic
number 3
number 4
number 5
number -1
number 50
number 100
number -1
number -1
number 0
number 1
number 2
number -1
number 5
number -1
number -1
number -1
number -1
number 0
number 0
number 0
number 3
number 3
number 3
number 0
number 0
number 0
number 0
number 3
number 3
number 3
number 3
number 3
number 3
number 0
number 3
number 2
===*/

print('basic');

function basicTest() {
    var obj;
    var i;

    // dense array

    test([1,2,3,1,2,3], [1]);
    test([1,2,3,1,2,3], [2]);
    test([1,2,3,1,2,3], [3]);
    test([1,2,3,1,2,3], [4]);

    // sparse array

    obj = [1];
    obj[100] = 3;
    obj[50] = 2;
    test(obj, [2]);

    // non-array object

    obj = { '0': 'foo', '50': 'bar', '100': 'quux', '200': 'baz', length: 101 };
    test(obj, ['quux']);
    test(obj, ['baz']);  // not found, because length limits

    // zero length

    test([], ['ignored']);

    // fromIndex

    test([1,2,3,1,2,3], [1, 2]);
    test([1,2,3,1,2,3], [2, 2]);
    test([1,2,3,1,2,3], [3, 2]);
    test([1,2,3,1,2,3], [4, 2]);

    // fromIndex not given, same as len-1; if given as undefined, coerces to 0 (!)

    test([1,2,3,1,2,3], [3]);
    test([1,2,3,1,2,3], [3, undefined]);

    // fromIndex range

    var indexlist = [ Number.NEGATIVE_INFINITY, -100, -7, -6, -5, -4, -3, -2, -1, -0, +0,
                      1, 2, 3, 4, 5, 6, 7, Number.POSITIVE_INFINITY, Number.NaN ];
    for (i = 0; i < indexlist.length; i++) {
        test([1,2,3,1,2,3], [1, indexlist[i]]);
    }

    // comparison is with strict equals, negative and positive zero are equal

    test([-0,-0,+0,+0], [+0]);     // matches at 0
    test(['',0,false], [false]);   // matches at 2, "falsy" values don't match
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
coercion
TypeError
TypeError
number -1
number -1
number -1
number -1
number 0
number -1
number 3
number 5
number -1
number -1
number -1
number -1
fromIndex valueOf
number 2
4
3 getter
2 getter
1 getter
number 1
3 getter
2 getter
1 getter
0 getter
number -1
3 getter
2 getter
1 getter
number 1
3 getter
2 getter
1 getter
0 getter
number -1
===*/

print('coercion');

function coercionTest() {
    var obj;
    var idxobj;

    // this coercion

    test(undefined, [1]);
    test(null, [1]);
    test(true, [1]);
    test(false, [1]);
    test(123, [1]);
    test('quux', [1]);
    test([1,2], [1]);
    test({ foo: 1, bar: 2 }, [1]);

    // length coercion

    test([1,1,1,1,1,1], [1, '3.9']);
    test([1,1,1,1,1,1], [1, 256*256*256*256 + 3.9]);
    test([1,1,1,1,1,1], [1, -256*256*256*256 + 3.9]);

    // zero array length check happens before fromIndex coercion

    test([], [1, {
        toString: function() { print('fromIndex toString'); return 4; },
        valueOf: function() { print('fromIndex valueOf'); return 3; },
    }]);

    test({ '0': 'foo', length: 0 }, [1, {
        toString: function() { print('fromIndex toString'); return 4; },
        valueOf: function() { print('fromIndex valueOf'); return 3; },
    }]);

    // ToUint32(length) == 0 suffices for a fast exit without fromIndex coercion

    test({ '0': 'foo', length: 256*256*256*256 }, [1, {
        toString: function() { print('fromIndex toString'); return 4; },
        valueOf: function() { print('fromIndex valueOf'); return 3; },
    }]);

    // fromIndex coercion

    idxobj = {
        toString: function() { print('fromIndex toString'); return '1.9'; },
        valueOf: function() { print('fromIndex valueOf'); return '0.29e1'; },  // = 2.9 -> 2
    }

    test([1,1,1,1,1], [1, idxobj]);

    // element [[Get]] side effects

    idxobj = {
        toString: function() { print('fromIndex toString'); return '0.9'; },
        valueOf: function() { print('fromIndex valueOf'); return '0.9'; },
    }

    obj = [];
    Object.defineProperties(obj, {
        '0': {
            get: function() { print('0 getter'); return 'foo'; },
            set: function() { print('0 setter'); },
            enumerable: true, configurable: true
        },
        '1': {
            get: function() { print('1 getter'); return 'bar'; },
            set: function() { print('1 setter'); },
            enumerable: true, configurable: true
        },
        '2': {
            get: function() { print('2 getter'); return 'quux'; },
            set: function() { print('2 setter'); },
            enumerable: true, configurable: true
        },
        '3': {
            get: function() { print('3 getter'); return 'baz'; },
            set: function() { print('3 setter'); },
            enumerable: true, configurable: true
        },
    });
    print(obj.length);

    test(obj, ['bar']);
    test(obj, ['nonexistent']);

    // same for a non-array

    obj = {};
    Object.defineProperties(obj, {
        '0': {
            get: function() { print('0 getter'); return 'foo'; },
            set: function() { print('0 setter'); },
            enumerable: true, configurable: true
        },
        '1': {
            get: function() { print('1 getter'); return 'bar'; },
            set: function() { print('1 setter'); },
            enumerable: true, configurable: true
        },
        '2': {
            get: function() { print('2 getter'); return 'quux'; },
            set: function() { print('2 setter'); },
            enumerable: true, configurable: true
        },
        '3': {
            get: function() { print('3 getter'); return 'baz'; },
            set: function() { print('3 setter'); },
            enumerable: true, configurable: true
        },
    });
    obj.length = 4;

    test(obj, ['bar']);
    test(obj, ['nonexistent']);
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
