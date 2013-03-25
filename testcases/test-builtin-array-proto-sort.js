/*
 *  Array.prototype.sort() tests.
 *
 *  The current implementation is qsort() with a random pivot, which makes
 *  this test non-deterministic unless the internal randomizer state is
 *  initialized with a fixed seed.
 */

function dumpValue(v) {
    var i, n;
    var tmp;
    var t;

    n = v.length;
    tmp = [];
    for (i = 0; i < n; i++) {
        if (v.hasOwnProperty(i)) {
            t = v[i];
            if (typeof t === 'function') {
                tmp.push('function');
            } else if (typeof t === 'object') {
                tmp.push('object');
            } else {
                tmp.push(String(t));
            }
        } else {
            tmp.push('nonexistent');
        }
    }

    return tmp.join(',');
}

function test(this_value, args) {
    var t;

    try {
        t = Array.prototype.sort.apply(this_value, args);
        print(dumpValue(t));
    } catch (e) {
        print(e.name);
    }
}

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
          'configurable=' + pd.configurable);
}

/*===
===*/

print('basic');

function basicTest() {
    /* Note that all cases without an explicit compare function will sort
     * elements as strings after ToString() coercion, e.g. 1 and 2 are
     * compared with string comparison "1" vs "2".  Thus, "1" < "10" < "2".
     */

    // empty case

    test([], []);
    test({ length: 0 }, []);
    test({}, []);

    // one element case

    test([1], []);

    // a few different lengths, in various orders

    test([1,2], []);
    test([2,1], []);
    test([1,2,3], []);
    test([1,3,2], []);
    test([2,1,3], []);
    test([2,3,1], []);
    test([3,1,2], []);
    test([3,2,1], []);

    // a few different lengths with some equal elements

    test([1,1], []);
    test([1,1,2], []);
    test([1,2,1], []);
    test([2,1,1], []);

    // undefined elements will be pushed to the end, and 'nonexistent' elements
    // even farther

    obj = [1,2,undefined,3,4,undefined,5,6,undefined,7,8,undefined,9,10];
    obj.length = 20;
    test(obj, []);

    // sparse array

    obj = [1,5,10];
    obj[100] = 2;
    obj[101] = 7;
    obj[102] = 13;
    obj[50] = 12;
    test(obj, []);

    // elements are compared as strings by default

    test([1,2,3,4,5,6,7,8,9,10], []);
    test([2,4,6,8,10,1,3,5,7,9], []);

    // explicit sort function for numbers, ascending and descending

    test([1,2,3,4,5,6,7,8,9,10], [ function(a,b) { return a-b; } ]);
    test([2,4,6,8,10,1,3,5,7,9], [ function(a,b) { return b-a; } ]);
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
===*/

print('exhaustive');

function cloneArray(x) {
    return x.map(function(x) { return x; });
}

function makeNonArray(x) {
    var res = {};
    var i;

    for (i = 0; i < x.length; i++) {
        if (x.hasOwnProperty(i)) {
            res[i] = x[i];
        }
    }
    res.length = x.length;

    return res;
}

function exhaustiveSortTest(input, make_sparse, make_nonarray) {
    function f(arr, rem) {
        var i, n;
        var new_arr, new_rem;
        var tmp;

        n = rem.length;
        if (n == 0) {
            if (make_sparse) {
                n = arr.length;
                arr[10000] = 'foo';
                delete arr[10000];
                arr.length = n;
            } else if (make_nonarray) {
                arr = makeNonArray(arr);
            }

            tmp = dumpValue(arr);
            Array.prototype.sort.call(arr);
            print(tmp, '--', dumpValue(arr));
        } else {
            for (i = 0; i < n; i++) {
                new_arr = cloneArray(arr);
                new_rem = cloneArray(rem);
                new_arr.push(new_rem.splice(i, 1)[0]);
                f(new_arr, new_rem);
            }
        }
    }

    f([], input);
}

try {
    exhaustiveSortTest([1,2,3], false, false);
    exhaustiveSortTest([1,2,3], true, false);
    exhaustiveSortTest([1,2,3], false, true);
    exhaustiveSortTest([1,2,3,3,4,5], false, false);
    exhaustiveSortTest([1,2,3,3,4,5], true, false);
    exhaustiveSortTest([1,2,3,3,4,5], false, true);
} catch (e) {
    print(e);
}

/*===
===*/

// http://en.wikipedia.org/wiki/Linear_congruential_generator#Parameters_in_common_use

print('random strings');

var rnd_x = 123;
function prng(max) {
    rnd_x = (rnd_x * 16807) % 0x7fffffff;
    return rnd_x % max;
}

function rndString(maxLen) {
    var len = prng(maxLen);
    var i;
    var res = '';

    for (i = 0; i < len; i++) {
        res += String.fromCharCode(0x41 + prng(26));
    }

    return res;
}

function randomStringsTest() {
    var arr = [];
    var i;
    var str1, str2;

    for (i = 0; i < 10000; i++) {
        arr.push(rndString(8));
    }

    print(arr);

    arr.sort();
    str1 = arr.toString();
    print(str1);

    arr.sort();
    str2 = arr.toString();
    print(str1 === str2);
}

randomStringsTest();

/*===
===*/

print('attributes');

function attributeTest() {
    var obj;

    // attributes are kept when both swapped elements exist: two [[Get]]+[[Put]]
    // pairs are used

    obj = [];
    Object.defineProperties(obj, {
        '0': { value: 'foo', writable: true, enumerable: false, configurable: true },
        '1': { value: 'bar', writable: true, enumerable: true, configurable: true },
    });
    printDesc(obj, '0'); printDesc(obj, '1'); printDesc(obj, 'length');
    obj.sort();
    printDesc(obj, '0'); printDesc(obj, '1'); printDesc(obj, 'length');

    // same test but make attributes non-configurable; this must not prevent sorting
    // because writability is sufficient when swapped pairs exist

    obj = [];
    Object.defineProperties(obj, {
        '0': { value: 'foo', writable: true, enumerable: false, configurable: false },
        '1': { value: 'bar', writable: true, enumerable: true, configurable: false },
    });
    printDesc(obj, '0'); printDesc(obj, '1'); printDesc(obj, 'length');
    obj.sort();
    printDesc(obj, '0'); printDesc(obj, '1'); printDesc(obj, 'length');

    // if only one end of the swapped pair exists, the other is [[Delete]]'d
    // and [[Put]] will create a property with default [[Put]] attributes, i.e.
    // writable, configurable, and enumerable

    obj = [];
    Object.defineProperties(obj, {
        '1': { value: 'bar', writable: true, enumerable: false, configurable: true },
    });
    printDesc(obj, '0'); printDesc(obj, '1'); printDesc(obj, 'length');
    obj.sort();
    printDesc(obj, '0'); printDesc(obj, '1'); printDesc(obj, 'length');

    // same test but '1' is non-configurable -> TypeError
    // the final state of 'obj' is implementation defined: the specification doesn't
    // mandate whether or not the [[Put]] for '0' should precede the [[Delete]] for '1'.

    obj = [];
    Object.defineProperties(obj, {
        '1': { value: 'bar', writable: true, enumerable: false, configurable: false },
    });
    printDesc(obj, '0'); printDesc(obj, '1'); printDesc(obj, 'length');
    obj.sort();
    printDesc(obj, '0'); printDesc(obj, '1'); printDesc(obj, 'length');
}

try {
    attributeTest();
} catch (e) {
    print(e);
}

/*===
===*/

print('comparefunction');

function compareFunctionTest() {
    // the default comparison function compares strings with ToString(); verify
    // its behavior first

    // basic ascending numeric sort

    test([ '1', 5, '3e0', '9.8', { valueOf: function() { return '7' } }],
         [ function(a,b) { return Number(a) - Number(b); } ]);

    // basic descending numeric sort

    test([ '1', 5, '3e0', '9.8', { valueOf: function() { return '7' } }],
         [ function(a,b) { return Number(b) - Number(a); } ]);

    // compareFn TypeError for a non-callable compareFn only occurs when
    // it is about to be called for the first time

   test([1], [ 'non-callable' ]);  // *no* TypeError
   test([1,2], [ 'non-callable' ]);  // -> TypeError
   test([1,2], [ {} ]);  // -> TypeError
   test([1,2], [ { toLocaleString: 'non-callable' } ]);  // -> TypeError

    // compareFn only gets called if both SortCompare() arguments exist
    // and neither is 'undefined'.  In this test only one element exists
    // and is not undefined, so no compareFn calls can be made

   obj = [];
   obj.length = 10;
   obj[0] = undefined;
   obj[2] = undefined;
   obj[3] = 'foo';
   obj[6] = undefined;
   obj[7] = undefined;
   obj[9] = undefined;
   test(obj, [ function(a,b) { print('should never be called'); } ]);
}

try {
    compareFunctionTest();
} catch (e) {
    print(e);
}

/*
automatic comparefn
manual comparefn

already ascending
already descending
random

sparse array with an inherited index showing through
-> impl dependent, what does e.g. node do?

non-extensible object -> impl dep
non-configurable elem (below length) -> impl dep
non-writable elem (below length) -> impl dep
accessor elem (below length)

non-configrable elem above length -> NOT impl dep
non-writable elem (above length) -> NOT impl dep
accessor elem (above length)
*/

