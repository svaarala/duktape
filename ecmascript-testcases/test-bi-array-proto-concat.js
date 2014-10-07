// Print types of elements too, because e.g. true and Boolean(true) print
// out the same normally ("true").  Also print non-existent properties
// explicitly.

function printArray(t) {
    var tmp = [];

    for (i = 0; i < t.length; i++) {
        var exists = t.hasOwnProperty(String(i));
        if (exists) {
            tmp.push(typeof t[i] + ':' + t[i]);
        } else {
            tmp.push('nonexistent');
        }
    }

    print(typeof t + ' ' + t.length + ' ' + tmp.join(','));
}

/*===
basic
TypeError
TypeError
TypeError
TypeError
object 5 object:true,number:1,number:2,string:foo,string:bar
object 5 object:false,number:1,number:2,string:foo,string:bar
object 5 object:123,number:1,number:2,string:foo,string:bar
object 5 object:foo,number:1,number:2,string:foo,string:bar
object 6 number:1,number:2,number:1,number:2,string:foo,string:bar
object 5 object:[object Object],number:1,number:2,string:foo,string:bar
object 4 number:1,number:2,number:3,number:4
object 101 1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,3
object 101 4,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,5,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,6
object 202 number:1,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,number:2,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,number:3,number:4,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,number:5,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,nonexistent,number:6
object 101 [object Object]
object 101 [object Object]
object 2 object:[object Object],object:[object Object]
object 4 object:true,number:1,number:2,number:3
object 4 object:[object Object],number:1,number:2,number:3
object 4 object:[object Object],number:1,number:2,number:3
object 5 string:foo,string:bar,number:1,number:2,number:3
object 4 nonexistent,nonexistent,nonexistent,string:elem
object 5 number:1,nonexistent,nonexistent,nonexistent,string:elem
object 4 string:foo,string:bar,nonexistent,nonexistent
object 8 number:1,number:1,number:2,string:foo,string:bar,nonexistent,nonexistent,number:4
===*/

print('basic');

function basicTest() {
    var t1, t2;

    function test(this_value, args) {
        var t;

        try {
            if (args.length == 0) {
                t = Array.prototype.concat.call(this_value);
            } else if (args.length == 1) {
                t = Array.prototype.concat.call(this_value, args[0]);
            } else if (args.length == 2) {
                t = Array.prototype.concat.call(this_value, args[0], args[1]);
            } else if (args.length == 3) {
                t = Array.prototype.concat.call(this_value, args[0], args[1], args[2]);
            } else {
                t = Array.prototype.concat.call(this_value, args[0], args[1], args[2], args[3]);
            }

            printArray(t);
        } catch (e) {
            print(e.name);
        }
    }

    // actual number of call arguments matter; undefined is treated
    // differently from an argument not given at all

    test(undefined, []);

    test(undefined, [ undefined ]);

    // basic types test
    // (note that 'this' argument is always ToObject() coerced, even for
    // strict functions)

    test(undefined, [ 1, 2, [ 'foo', 'bar' ] ]);
    test(null, [ 1, 2, [ 'foo', 'bar' ] ]);
    test(true, [ 1, 2, [ 'foo', 'bar' ] ]);
    test(false, [ 1, 2, [ 'foo', 'bar' ] ]);
    test(123, [ 1, 2, [ 'foo', 'bar' ] ]);
    test('foo', [ 1, 2, [ 'foo', 'bar' ] ]);
    test([1,2], [ 1, 2, [ 'foo', 'bar' ] ]);
    test({ foo: 1, bar: 2 }, [ 1, 2, [ 'foo', 'bar' ] ]);

    // concatenating two dense arrays

    t1 = [ 1, 2 ];
    t2 = [ 3, 4 ];
    test(t1, [ t2 ]);

    // concatenating two sparse arrays

    t1 = [ 1 ];
    t1[100] = 3;
    t1[50] = 2;
    t2 = [ 4 ];
    t2[100] = 6;
    t2[50] = 5;
    print(typeof t1, t1.length, t1);
    print(typeof t2, t2.length, t2);
    test(t1, [ t2 ]);

    // concatenating two non-arrays; here the objects don't fall into the
    // special handling in step 5.b and are put into the result array as is

    t1 = { '0': 1, '100': 2, length: 101 };
    t2 = { '0': 3, '100': 4, length: 101 };
    print(typeof t1, t1.length, t1);
    print(typeof t2, t2.length, t2);
    test(t1, [ t2 ]);

    // 'this' is ToObject() coerced and becomes the first element to be
    // processed in the loop of E5.1 Section 15.4.4.4, step 5.  If 'this'
    // is an array it falls into the special handling of step 5.b (this
    // is the typical case); otherwise it is used as the first element
    // as-is.
    //
    // (V8 seems to coerce 'true' to a primitive boolean value, not a
    // Boolean object.)

    test(true, [ 1, 2, 3 ]);  // ToObject(true) -> Boolean(true)

    test({ foo: 1, bar: 2 }, [ 1, 2, 3 ]);  // already an object

    test({ '0': 'foo', '1': 'bar', length: 2 }, [ 1, 2, 3 ]);  // no special treatment even if "array like"

    test([ 'foo', 'bar' ], [ 1, 2, 3 ]);   // Array -> "flattened"

    // An array in the argument list is "flattened".  The algorithm in E5.1
    // Section 15.4.4.4 won't insert missing elements into the result array
    // (the 'n' counter is incremented).  If no elements are inserted after
    // flattening, trailing non-existent elements won't increase the result
    // "length".
    //
    // Both V8 and Rhino seem to deviate from this: they will update the
    // result length even for trailing non-existent elements.
    //
    // This case is now tested separately by test-bi-array-proto-concat-nonstd-trailing.js
    // because the desired behavior is non-standard.

    t = [];
    t[3] = 'elem';
    printArray(t);
    test([1], [ t ]);

    t = [ 'foo', 'bar' ];
    t.length = 4;  // two "non-existent" elements
    printArray(t);
    test([1], [ 1, 2, t, 4 ]);  // '4' will update final length -> 8

/* Disabled, see test-bi-array-proto-concat-nonstd-trailing.js
    t = [ 'foo', 'bar' ];
    t.length = 4;  // two "non-existent" elements
    printArray(t);
    test([1], [ 1, 2, t ]);  // nothing follows non-existent elements, so final length should be 5, not 7
                             // Rhino and V8 will have final result length 7
*/
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
implant
object 2 object:true,string:foo
===*/

print('implant');

function implantTest() {
    'use strict';
    var t;
    var proto;

    // when concat() is invoked, this binding will the primitive 'true'
    // value, which is then coerced to an object in concat() step 1.

    Boolean.prototype.concat = Array.prototype.concat;
    t = true.concat('foo');
    printArray(t);

    // It would be nice to test a case where an implanted concat was used
    // with a "sub-class" of Array.  However, because of E5.1 Section
    // 15.4.4.4 step 5.b, the 'this' binding would not be treated as an
    // Array anyway, and the 'this' binding would thus go as is into the
    // result array, as happens above for the Boolean test.
}

try {
    implantTest();
} catch (e) {
    print(e);
}
