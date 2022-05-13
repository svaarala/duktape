/*
 *  Strict function as an arguments 'caller' property.
 *
 *  In ES2015 Arguments has exotic behavior where arguments.caller cannot
 *  evaluate to a strict function (https://262.ecma-international.org/6.0/#sec-arguments-exotic-objects)
 *  (even if it is set to such value).  This behavior was removed in ES2017.
 */

/*===
functions
0 0 fail to set "caller": TypeError
0 0 TypeError
0 1 fail to set "caller": TypeError
0 1 TypeError
0 2 fail to set "caller": TypeError
0 2 TypeError
0 3 fail to set "caller": TypeError
0 3 TypeError
0 4 fail to set "caller": TypeError
0 4 TypeError
1 0 fail to set "caller": TypeError
1 0 TypeError
1 1 fail to set "caller": TypeError
1 1 TypeError
1 2 fail to set "caller": TypeError
1 2 TypeError
1 3 fail to set "caller": TypeError
1 3 TypeError
1 4 fail to set "caller": TypeError
1 4 TypeError
2 0 fail to set "caller": TypeError
2 0 TypeError
2 1 fail to set "caller": TypeError
2 1 TypeError
2 2 fail to set "caller": TypeError
2 2 TypeError
2 3 fail to set "caller": TypeError
2 3 TypeError
2 4 fail to set "caller": TypeError
2 4 TypeError
3 0 fail to set "caller": TypeError
3 0 TypeError
3 1 fail to set "caller": TypeError
3 1 TypeError
3 2 fail to set "caller": TypeError
3 2 TypeError
3 3 fail to set "caller": TypeError
3 3 TypeError
3 4 fail to set "caller": TypeError
3 4 TypeError
non-strict arguments, no formals
bar
undefined
0 string
1 function
2 function
3 function
4 function
non-strict arguments, with formals
bar
undefined
0 string
1 function
2 function
3 function
4 function
strict arguments, no formals
bar
undefined
pass in es2017
strict arguments, with formals
bar
undefined
pass in es2017
===*/

var baseFunctionCreators = [
    function () {
        return function nonStrictNonBoundBase () {}
    },
    function () {
        return function strictNonBoundBase () { 'use strict'; }
    },
    function () {
        var f = function nonStrictBoundBase() {};
        return f.bind('dummy');
    },
    function () {
        var f = function strictBoundBase() { 'use strict' };
        return f.bind('dummy');
    }
];

function nonStrictFunc() {
}
function strictFunc() {
    'use strict';
}
var nonStrictBoundFunc = nonStrictFunc.bind('dummy');
var strictBoundFunc = strictFunc.bind('dummy');

var callerPropertyValues = [
    'whatever',
    nonStrictFunc,
    strictFunc,
    nonStrictBoundFunc,
    strictBoundFunc
];

function testFunctions() {
    var i, j;
    var base, val;

    for (i = 0; i < baseFunctionCreators.length; i++) {
        for (j = 0; j < callerPropertyValues.length; j++) {
            base = baseFunctionCreators[i]();

            // Now fails for all functions because of an inherited
            // Function.prototype.caller which throws unconditionally.
            try {
                base.caller = callerPropertyValues[j];
            } catch (e) {
                print(i, j, 'fail to set "caller": ' + e.name);
            }

            try {
                val = base.caller;
                print(i, j, typeof val);
            } catch (e) {
                print(i, j, e.name);
            }
        }
    }
}

// Quite counterintuitively, a non-strict function with *no formals* will get
// an arguments object no special [[Get]] behavior -- not even special behavior
// for 'caller'.  This happens because the arguments object will not get a
// [[ParameterMap]] in E.5 Section 10.6 step 12 and then later in Section 10.6:
//
//     The [[Get]] internal method of an arguments object for a non-strict mode
//     function with formal parameters when called with a property name P performs
//     the following steps: [...]

function testArgumentsNonStrictNoFormals() {
    var val;
    var i;

    print(arguments[1]);  // just a test to see 'arguments' is proper

    try {
        val = arguments.caller;
        print(typeof val);
    } catch (e) {
        print(e.name);
    }

    for (i = 0; i < callerPropertyValues.length; i++) {
        arguments.caller = callerPropertyValues[i];
        try {
            val = arguments.caller;
            print(i, typeof val);
        } catch (e) {
            print(i, e.name);
        }
    }
}

function testArgumentsNonStrictWithFormals(whatever) {
    var val;
    var i;

    print(arguments[1]);  // just a test to see 'arguments' is proper

    try {
        val = arguments.caller;
        print(typeof val);
    } catch (e) {
        print(e.name);
    }

    for (i = 0; i < callerPropertyValues.length; i++) {
        arguments.caller = callerPropertyValues[i];
        try {
            val = arguments.caller;
            print(i, typeof val);
        } catch (e) {
            print(i, e.name);
        }
    }
}

function testArgumentsStrictNoFormals() {
    'use strict';

    var val;

    print(arguments[1]);  // just a test to see 'arguments' is proper

    try {
        val = arguments.caller;
        print(typeof val);
    } catch (e) {
        print(e.name);
    }
}

function testArgumentsStrictWithFormals(whatever) {
    'use strict';

    var val;

    print(arguments[1]);  // just a test to see 'arguments' is proper

    try {
        val = arguments.caller;
        print(typeof val);
    } catch (e) {
        print(e.name);
    }
}

print('functions');
try {
    testFunctions();
} catch (e) {
    print(e);
}

print('non-strict arguments, no formals');
try {
    testArgumentsNonStrictNoFormals('foo', 'bar', 'quux');
} catch (e) {
    print(e);
}

print('non-strict arguments, with formals');
try {
    testArgumentsNonStrictWithFormals('foo', 'bar', 'quux');
} catch (e) {
    print(e);
}

print('strict arguments, no formals');
try {
    testArgumentsStrictNoFormals('foo', 'bar', 'quux');
    print('pass in es2017');
} catch (e) {
    print(e);
}

print('strict arguments, with formals');
try {
    testArgumentsStrictWithFormals('foo', 'bar', 'quux');
    print('pass in es2017');
} catch (e) {
    print(e);
}
