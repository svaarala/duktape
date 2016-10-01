/*
 *  JSON.stringify() fast path would handle boxed String and Number objects
 *  incorrectly in Duktape 1.4.x and before: the fast path would simply look
 *  up the primitive string or number value without actually invoking
 *  .toString() or .valueOf() as part of ToPrimitive() operation.
 *
 *  The difference shows up when .toString() and/or .valueOf() have been
 *  overridden.
 */

/*===
my .valueOf() called
my .toString() called
null
my .valueOf() called
my .toString() called
123
===*/

function testNumberCoercion() {
    var x = new Number(12345);
    x.toString = function myToString() {
        print('my .toString() called');
        return 'replaced';  // Note: Number coercion result can be a string, as long as it's primitive
    }
    x.valueOf = function myValueOf() {
        print('my .valueOf() called');
        return {};  // because return value is not primitive, ToPrimitive() must invoke .valueOf()
    }

    // The algorithm in E5.1 Section 15.12.3 calls ToNumber() on the value.
    // It first coerces using ToPrimitive() (with a number hint) which yields
    // "replaced" here.  That primitive value is then coerced using ToNumber()
    // which yields NaN which gets serialized as 'null'.

    print(JSON.stringify(x));

    // If the string coerces to a number the ToNumber() coercion for the string
    // will succeed.

    x.toString = function myToString() {
        print('my .toString() called');
        return '123';  // Note: Number coercion result can be a string, as long as it's primitive
    }

    print(JSON.stringify(x));
}

try {
    testNumberCoercion();
} catch (e) {
    print(e.stack || e);
}

/*===
my .toString() called
my .valueOf() called
"replaced"
my .toString() called
my .valueOf() called
"123"
===*/

function testStringCoercion() {
    var x = new String('my string');
    x.toString = function myToString() {
        print('my .toString() called');
        return {};  // because return value is not primitive, ToPrimitive() must invoke .valueOf()
    }
    x.valueOf = function myValueOf() {
        print('my .valueOf() called');
        return 'replaced';
    }

    // Similar to above, E5.1 Section 15.12.3 requires use of ToString() on
    // an object, which first coerces the object to a primitive value using
    // ToPrimitive (with a string hint, i.e. .toString() first) and then
    // uses ToString() again on the primitive value.

    print(JSON.stringify(x));

    x.valueOf = function myValueOf() {
        print('my .valueOf() called');
        return 123;
    }

    print(JSON.stringify(x));
}

try {
    testStringCoercion();
} catch (e) {
    print(e.stack || e);
}
