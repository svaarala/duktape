function test(this_value, args) {
    try {
        var t = Array.prototype.reduceRight.apply(this_value, args);
        print(typeof t, t);
    } catch (e) {
        print(e.name);
    }
}

function adder(prev, curr, index, obj) {
    print('adder', typeof prev + ':' + prev, typeof curr + ':' + curr, typeof index, index, typeof obj);
    return prev + curr;
}

/*===
basic
TypeError
number 100
number 1
adder number:100 number:1 number 0 object
number 101
adder number:2 number:1 number 0 object
number 3
adder number:100 number:2 number 1 object
adder number:102 number:1 number 0 object
number 103
adder number:5 number:4 number 3 object
adder number:9 number:3 number 2 object
adder number:12 number:2 number 1 object
adder number:14 number:1 number 0 object
number 15
adder number:100 number:5 number 4 object
adder number:105 number:4 number 3 object
adder number:109 number:3 number 2 object
adder number:112 number:2 number 1 object
adder number:114 number:1 number 0 object
number 115
adder number:10 number:9 number 8 object
adder number:19 number:8 number 7 object
adder number:27 number:7 number 6 object
adder number:34 number:6 number 5 object
adder number:40 number:5 number 4 object
adder number:45 number:4 number 3 object
adder number:49 number:3 number 2 object
adder number:52 number:2 number 1 object
adder number:54 number:1 number 0 object
number 55
adder number:5 number:4 number 50 object
adder number:9 number:3 number 2 object
adder number:12 number:2 number 1 object
adder number:14 number:1 number 0 object
number 15
adder number:5 number:4 number 3 object
adder number:9 number:3 number 2 object
adder number:12 number:2 number 1 object
adder number:14 number:1 number 0 object
number 15
3 true
2 true
1 true
0 true
undefined undefined
adder number:3 number:2 number 1 object
adder number:5 number:1 number 0 object
number 6
adder undefined:undefined number:3 number 2 object
adder number:NaN number:2 number 1 object
adder number:NaN number:1 number 0 object
number NaN
===*/

print('basic');

function basicTest() {
    var obj;

    // basic combinations of different input length and presence of an
    // initial value

    test([], [adder])
    test([], [adder, 100])
    test([1], [adder])
    test([1], [adder, 100])
    test([1,2], [adder])
    test([1,2], [adder, 100])
    test([1,2,3,4,5], [adder])
    test([1,2,3,4,5], [adder, 100])

    // dense array

    test([1,2,3,4,5,6,7,8,9,10], [adder]);

    // sparse array

    obj = [1,2,3];
    obj[100] = 5;
    obj[50] = 4;
    test(obj, [adder]);

    // non-array

    obj = {
        '0': 1, '1': 2, '2': 3, '3': 4, '4': 5, length: 5
    };
    test(obj, [adder]);

    // check callback object argument

    obj = {
        '0': 1, '1': 2, '2': 3, '3': 4, '4': 5, length: 5
    }
    test(obj, [ function (prev, curr, index, o) {
        print(index, obj === o);
    } ]);

    // undefined initialValue is different from an initialValue not
    // given at all

    test([1,2,3], [adder]);
    test([1,2,3], [adder, undefined]);
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
inherited
adder string:quux string:bar number 1 object
adder string:quuxbar string:foo number 0 object
string quuxbarfoo
adder string:baz string:quux number 2 object
adder string:bazquux string:bar number 1 object
adder string:bazquuxbar string:foo number 0 object
string bazquuxbarfoo
===*/

/* Since [[HasProperty]] and [[Get]] are used, inherited properties
 * must "show through" properly, for both arrays and objects.
 */

print('inherited');

function inheritedTest() {
    var obj;
    var proto;

    obj = [];
    obj[0] = 'foo';
    Array.prototype[1] = 'bar';
    obj[2] = 'quux';
    test(obj, [ adder ]);
    delete Array.prototype[1];

    proto = Object.create(Object.prototype);
    proto[0] = 'foo';
    proto[1] = 'bar';
    proto[3] = 'baz';
    proto.length = 5;
    obj = Object.create(proto);
    obj[2] = 'quux';  // no 'length' in obj own properties
    test(obj, [ adder ]);
}

try {
    inheritedTest();
} catch (e) {
    print(e);
}

/*===
mutation
callback quux bar 1 foo,bar,quux
callback quuxbar foo 0 foo,bar,quux,baz
string quuxbarfoo
callback quux bar 1 foo,bar,quux
callback quuxbar foo 0 foo,bar,
string quuxbarfoo
callback quux bar 1 [object Object]
callback quuxbar foo 0 [object Object]
string quuxbarfoo
callback quux bar 1 [object Object]
callback quuxbar foo 0 [object Object]
string quuxbarfoo
callback bar foo 0 [object Object]
string barfoo
callback baz quux 2 [object Object]
callback bazquux bar 1 [object Object]
callback bazquuxbar foo 0 [object Object]
string bazquuxbarfoo
===*/

print('mutation');

function mutationTest() {
    var obj;

    // Mutation: adding elements to end of array after iteration starts
    // -> new elements not included

    obj = [ 'foo', 'bar', 'quux' ];
    test(obj, [function(prev, curr, index, obj) {
        print('callback', prev, curr, index, obj);
        obj['3'] = 'baz';  // not included
        return prev + curr;
    }]);

    // Mutation: deleting an element inside the initial length range
    // -> deleted element not included in iteration

    obj = [ 'foo', 'bar', 'quux' ];
    test(obj, [function(prev, curr, index, obj) {
        print('callback', prev, curr, index, obj);
        delete obj['2'];  // not included
        return prev + curr;
    }]);

    // Mutation: if 'length' is updated during processing, the updated
    // length is not respected.

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', length: 3 };
    test(obj, [function(prev, curr, index, obj) {
        print('callback', prev, curr, index, obj);
        obj['3'] = 'baz';
        obj.length = 4;  // not respected
        return prev + curr;
    }]);

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', length: 3 };
    test(obj, [function(prev, curr, index, obj) {
        print('callback', prev, curr, index, obj);
        obj.length = 0;  // not respected
        return prev + curr;
    }]);

    // Mutation: non-existent elements can be added and will be processed
    // as long as they are within the initial length range.

    obj = { '0': 'foo', '1': 'bar', length: 4 };
    test(obj, [function(prev, curr, index, obj) {
        print('callback', prev, curr, index, obj);
        obj['2'] = 'quux';
        obj['3'] = 'baz';
        return prev + curr;
    }]);

    // Mutation: deleted elements are skipped.

    obj = { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 4 };
    test(obj, [function(prev, curr, index, obj) {
        print('callback', prev, curr, index, obj);
        delete obj['2'];
        return prev + curr;
    }]);
}

try {
    mutationTest();
} catch (e) {
    print(e);
}

/*===
coercion
TypeError
TypeError
TypeError
TypeError
TypeError
adder string:o string:o number 1 object
adder string:oo string:f number 0 object
string oof
adder number:3 number:2 number 1 object
adder number:5 number:1 number 0 object
number 6
number 123
TypeError
adder string:quux string:bar number 1 object
adder string:quuxbar string:foo number 0 object
string quuxbarfoo
adder string:quux string:bar number 1 object
adder string:quuxbar string:foo number 0 object
string quuxbarfoo
adder string:baz string:quux number 2 object
adder string:bazquux string:bar number 1 object
adder string:bazquuxbar string:foo number 0 object
string bazquuxbarfoo
adder string:quux string:bar number 1 object
adder string:quuxbar string:foo number 0 object
string quuxbarfoo
length valueOf
adder string:quux string:bar number 1 object
adder string:quuxbar string:foo number 0 object
string quuxbarfoo
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
length valueOf
TypeError
length getter
2 getter
callback 2
initialValue toString()
1 getter
callback 1
0 getter
callback 0
string initial3rd2nd1st
===*/

print('coercion');

function coercionTest() {
    var obj;

    // this

    test(undefined, [ adder ]);
    test(null, [ adder ]);
    test(true, [ adder ]);
    test(false, [ adder ]);
    test(123, [ adder ]);
    test('foo', [ adder ]);
    test([1,2,3], [ adder ]);
    test({ foo: 1, bar: 2 }, [ adder, 123 ]);  // object is empty, initial value -> returned as is
    test({ foo: 1, bar: 2 }, [ adder ]);       // object is empty, no initial value -> TypeError

    // length

    test({ '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 3.9 }, [ adder ]);
    test({ '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: 256*256*256*256 + 3.9 }, [ adder ]);  // coerces to 3
    test({ '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: -256*256*256*256 + 3.9 }, [ adder ]);  // coerces to 4
    test({ '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: '3.9' }, [ adder ]);
    test({ '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: {
        toString: function() { print('length toString'); return 4; },
        valueOf: function() { print('length valueOf'); return 3; },
    } }, [ adder ]);

    // length is not coerced if this coercion fails

    test(null, [ {
        toString: function() { print('length toString'); return 4; },
        valueOf: function() { print('length valueOf'); return 3; },
    } ]);

    // callback must be callable, otherwise TypeError (occurs after length coercion)

    test([1,2,3], [ undefined ]);
    test([1,2,3], [ null ]);
    test([1,2,3], [ true ]);
    test([1,2,3], [ false ]);
    test([1,2,3], [ 123 ]);
    test([1,2,3], [ 'foo' ]);
    test([1,2,3], [ [1,2] ]);
    test([1,2,3], [ { foo: 1, bar: 2 } ]);

    test({ '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', length: {
        toString: function() { print('length toString'); return 4; },
        valueOf: function() { print('length valueOf'); return 3; },
    } }, [ true ]);

    // coercion order test

    obj = Object.create(Object.prototype, {
        '0': {
             get: function() { print('0 getter'); return '1st'; },
             set: function() { print('0 setter'); throw new Error('setter called') },
             enumerable: true,
             configurable: true
        },
        '1': {
             get: function() { print('1 getter'); return '2nd'; },
             set: function() { print('1 setter'); throw new Error('setter called') },
             enumerable: true,
             configurable: true
        },
        '2': {
             get: function() { print('2 getter'); return '3rd'; },
             set: function() { print('2 setter'); throw new Error('setter called') },
             enumerable: true,
             configurable: true
        },
        'length': {
             get: function() { print('length getter'); return '4.9'; },  // coerces to 4
             set: function() { print('length setter'); throw new Error('setter called') },
             enumerable: true,
             configurable: true
        },
    });

    test(obj, [
        function callback(prev, curr, index, obj) {
            print('callback', index);  // avoid side effects here
            return String(prev) + String(curr);
        },
        {
            // initial value is not coerced by the algorithm, but by the callback code
            toString: function() { print('initialValue toString()'); return 'initial'; },
            valueOf: function() { print('initialValue valueOf()'); return 'initial-notused'; }
        }
    ]);
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
