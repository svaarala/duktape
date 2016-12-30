/*
 *  Compare function behavior, specified more concretely in ES2015.
 */

/*===
true
true
false
1,2,3,4,5,6,7,8,9,10
1,2,3,4,5,6,7,8,9,10
1,2,3,4,5,6,7,8,9,10
1,2,3,4,4,4,5,6,6,7,8,9,10
===*/

function test() {
    var compareFnCalled = false;
    var valueOfCalled = false;
    var toStringCalled = false;
    var arr;

    function compareFn(a, b) {
        compareFnCalled = true;
        var ret = 0;
        if (a < b) { ret = -1; }
        if (a > b) { ret = 1; }
        return {
            valueOf: function () { valueOfCalled = true; return ret; },
            toString: function () { toStringCalled = true; return ret; }
        };
    }

    arr = [ 10, 4, 7, 3, 1, 2, 6, 5, 9, 8 ];

    // Compare callback counts and arguments are up to the implementation, but
    // we know it will be called at least once.  So, check that it gets called
    // and that the result value is ToNumber() coerced based on both side effects
    // and final sort result.

    arr.sort(compareFn);

    print(compareFnCalled);
    print(valueOfCalled);
    print(toStringCalled);
    print(arr);

    // ToNumber coercion means strings representing numbers can be returned.
    arr = [ 10, 4, 7, 3, 1, 2, 6, 5, 9, 8 ];
    arr.sort(function (a, b) {
        var ret = '0.0';
        if (a < b) { ret = '-1000'; }
        if (a > b) { ret = '0xdeadbeef'; }
        return ret;
    });
    print(arr);

    // Inf and -Inf should be treated like 1 and -1.
    arr = [ 10, 4, 7, 3, 1, 2, 6, 5, 9, 8 ];
    arr.sort(function (a, b) {
        var ret = 0/0;
        if (a < b) { ret = -1/0; }
        if (a > b) { ret = 1/0; }
        return ret;
    });
    print(arr);

    // If NaN is returned, it is treated like a +0.  We could return Nan for
    // every element but that would *not* guarantee a stable sort because
    // stability is not required (and not provided by current qsort approach).
    arr = [ 10, 4, 7, 4, 4, 3, 1, 6, 2, 6, 5, 9, 8 ];
    arr.sort(function (a, b) {
        var ret = 0/0;
        if (a < b) { ret = -1; }
        if (a > b) { ret = 1; }
        return ret;
    });
    print(arr);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
