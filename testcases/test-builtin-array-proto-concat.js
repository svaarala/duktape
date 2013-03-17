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
object 5 boolean:true,number:1,number:2,string:foo,string:bar
object 5 boolean:false,number:1,number:2,string:foo,string:bar
object 5 number:123,number:1,number:2,string:foo,string:bar
object 5 string:foo,number:1,number:2,string:foo,string:bar
object 6 number:1,number:2,number:1,number:2,string:foo,string:bar
object 5 object:[object Object],number:1,number:2,string:foo,string:bar
object 4 number:1,number:2,number:3,number:4
object 4 boolean:true,number:1,number:2,number:3
object 4 object:[object Object],number:1,number:2,number:3
object 4 object:[object Object],number:1,number:2,number:3
object 5 string:foo,string:bar,number:1,number:2,number:3
object 4 nonexistent,nonexistent,nonexistent,string:elem
object 5 number:1,nonexistent,nonexistent,nonexistent,string:elem
object 4 string:foo,string:bar,nonexistent,nonexistent
object 8 number:1,number:1,number:2,string:foo,string:bar,nonexistent,nonexistent,number:4
object 4 string:foo,string:bar,nonexistent,nonexistent
object 5 number:1,number:1,number:2,string:foo,string:bar
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

    test(undefined, [ 1, 2, [ 'foo', 'bar' ] ]);
    test(null, [ 1, 2, [ 'foo', 'bar' ] ]);
    test(true, [ 1, 2, [ 'foo', 'bar' ] ]);
    test(false, [ 1, 2, [ 'foo', 'bar' ] ]);
    test(123, [ 1, 2, [ 'foo', 'bar' ] ]);
    test('foo', [ 1, 2, [ 'foo', 'bar' ] ]);
    test([1,2], [ 1, 2, [ 'foo', 'bar' ] ]);
    test({ foo: 1, bar: 2 }, [ 1, 2, [ 'foo', 'bar' ] ]);

    // concatenating two arrays

    t1 = [ 1, 2 ];
    t2 = [ 3, 4 ];
    test(t1, t2);

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

    t = [];
    t[3] = 'elem';
    printArray(t);
    test([1], [ t ]);

    t = [ 'foo', 'bar' ];
    t.length = 4;  // two "non-existent" elements
    printArray(t);
    test([1], [ 1, 2, t, 4 ]);  // '4' will update final length -> 8

    t = [ 'foo', 'bar' ];
    t.length = 4;  // two "non-existent" elements
    printArray(t);
    test([1], [ 1, 2, t ]);  // nothing follows non-existent elements, so final length should be 5, not 7
                             // Rhino and V8 will have final result length 7
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

    Boolean.prototype.concat = Array.prototype.concat;

    // when concat() is invoked, this binding will the primitive 'true'
    // value, which is then coerced to an object in concat() step 1.

    t = true.concat('foo');
    printArray(t);
}

try {
    implantTest();
} catch (e) {
    print(e);
}

