/*
 *  Minor implementation detail needed in duk_bi_json.c:duk__enc_value():
 *
 *  JSON.stringify() coercion of a String or Number object involves calling
 *  user .valueOf() and/or .toString() but the result can never be a function
 *  object.
 *
 *  This is a very minor detail, but was broken in Duktape 1.3.x and 1.4.x
 *  (worked correctly in Duktape 1.2.x and fixed in Duktape 1.5.x).  Testcase
 *  updates for Duktape 2.0 error message.
 */

// Error message is custom
/*---
{
    "custom": true
}
---*/

/*===
x.toString() called
x.valueOf() called
TypeError: coercion to primitive failed
x.toString() called
x.valueOf() called
TypeError: coercion to primitive failed
===*/

function test1(forceSlow) {
    var x = new String('my string');
    x.toString = function () {
        print('x.toString() called');
        return function dummy1() {};  // ignored by ToPrimitive()
    };
    x.valueOf = function () {
        print('x.valueOf() called');
        return function dummy2() {};  // ignored by ToPrimitive()
    }
    if (forceSlow) {
        x.toJSON = 1;  // non-callable so ignored, but forces out of fast path (at least in Duktape 1.5.x)
    }

    print(JSON.stringify(x));
}

try {
    test1(false);
} catch (e) {
    print(e);
}
try {
    test1(true);
} catch (e) {
    print(e);
}

/*===
x.valueOf() called
x.toString() called
TypeError: coercion to primitive failed
x.valueOf() called
x.toString() called
TypeError: coercion to primitive failed
===*/

function test2(forceSlow) {
    var x = new Number(1234);
    x.toString = function () {
        print('x.toString() called');
        return function dummy1() {};  // ignored by ToPrimitive()
    };
    x.valueOf = function () {
        print('x.valueOf() called');
        return function dummy2() {};  // ignored by ToPrimitive()
    }
    if (forceSlow) {
        x.toJSON = 1;  // non-callable so ignored, but forces out of fast path (at least in Duktape 1.5.x)
    }

    print(JSON.stringify(x));
}

try {
    test2(false);
} catch (e) {
    print(e);
}
try {
    test2(true);
} catch (e) {
    print(e);
}
