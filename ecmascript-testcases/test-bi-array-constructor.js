function run(f) {
    try {
        f();
    } catch (e) {
        print(e.name);
    }
}

/*===
object 0 []
object 0 []
object 3 [null,null,null]
object 300 [null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null]
RangeError
RangeError
RangeError
RangeError
RangeError
RangeError
object 1 [null]
object 1 [true]
object 1 [false]
object 1 ["123"]
object 1 [[1,2]]
object 1 [{"foo":1,"bar":2}]
object 2 [1,2]
object 2 [[1,2],[3,4]]
object 20 [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20]
===*/

function asFunctionTest() {
    function p(x) {
        print(typeof x, x.length, JSON.stringify(x));
    }

    // no args case
    run(function() { p(Array()); });

    // one arg, a number
    run(function() { p(Array(0)); });
    run(function() { p(Array(3)); });
    run(function() { p(Array(300)); });
    run(function() { p(Array(-1)); });                        // ToUint32(len) !== len -> RangeError
    run(function() { p(Array(256*256*256*256)); });
    run(function() { p(Array(1.4)); });
    run(function() { p(Array(Number.NaN)); });
    run(function() { p(Array(Number.NEGATIVE_INFINITY)); });
    run(function() { p(Array(Number.POSITIVE_INFINITY)); });

    // one arg, not a number
    run(function() { p(Array(null)); });
    run(function() { p(Array(true)); });
    run(function() { p(Array(false)); });
    run(function() { p(Array('123')); });
    run(function() { p(Array([1,2])); });
    run(function() { p(Array({ foo: 1, bar: 2 })); });

    // two args, first arg may or may not be a number (no special case)
    run(function() { p(Array(1,2)); });
    run(function() { p(Array([1,2],[3,4])); });
    run(function() { p(Array(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20)); });
}

try {
    asFunctionTest();
} catch (e) {
    print(e);
}

/*===
object 0 []
object 0 []
object 3 [null,null,null]
object 300 [null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null]
RangeError
RangeError
RangeError
RangeError
RangeError
RangeError
object 1 [null]
object 1 [true]
object 1 [false]
object 1 ["123"]
object 1 [[1,2]]
object 1 [{"foo":1,"bar":2}]
object 2 [1,2]
object 2 [[1,2],[3,4]]
object 20 [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20]
===*/

function asConstructorTest() {
    function p(x) {
        print(typeof x, x.length, JSON.stringify(x));
    }

    // no args case
    run(function() { p(new Array()); });

    // one arg, a number
    run(function() { p(new Array(0)); });
    run(function() { p(new Array(3)); });
    run(function() { p(new Array(300)); });
    run(function() { p(new Array(-1)); });                        // ToUint32(len) !== len -> RangeError
    run(function() { p(new Array(256*256*256*256)); });
    run(function() { p(new Array(1.4)); });
    run(function() { p(new Array(Number.NaN)); });
    run(function() { p(new Array(Number.NEGATIVE_INFINITY)); });
    run(function() { p(new Array(Number.POSITIVE_INFINITY)); });

    // one arg, not a number
    run(function() { p(new Array(null)); });
    run(function() { p(new Array(true)); });
    run(function() { p(new Array(false)); });
    run(function() { p(new Array('123')); });
    run(function() { p(new Array([1,2])); });
    run(function() { p(new Array({ foo: 1, bar: 2 })); });

    // two args, first arg may or may not be a number (no special case)
    run(function() { p(new Array(1,2)); });
    run(function() { p(new Array([1,2],[3,4])); });
    run(function() { p(new Array(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20)); });
}

try {
    asConstructorTest();
} catch (e) {
    print(e);
}
