/*
 *  Test case for https://github.com/svaarala/duktape/issues/2529:
 *  Check that Object.assign works for a large number of parameters
 * 
 * Used to throw:
 * RangeError: cannot push beyond allocated stack
 *    at [anon] (duk_api_stack.c:4316) internal
 *    at assign () native strict preventsyield
 *    at test (tests/ecmascript/bug/test-bug-object-assign.js:25)
 *    at global (tests/ecmascript/bug/test-bug-object-assign.js:30) preventsyield
 *    error in executing file tests/ecmascript/bug/test-bug-object-assign.js
 */

/*===
499
===*/

function test() {
    var arguments = [{}];

    for (var i = 0; i < 500; i++) {
        arguments.push({key: i});
    }

    var result = Object.assign.apply(Object, arguments);

    print(result.key);    
}

test();
