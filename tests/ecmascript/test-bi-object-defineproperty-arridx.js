/*
 *  Object.defineProperty() array part behavior coverage.
 */

/*===
1000
foo foo undefined undefined
1000000001
foo foo undefined bar
1000
foo foo undefined undefined
1001
foo foo bar undefined
999
foo undefined foo undefined undefined undefined
done
===*/

function test() {
    var A;
    var i;

    // These maintain the array part.
    A = [];
    for (i = 0; i < 1000; i++) {
        Object.defineProperty(A, i, { value: 'foo', writable: true, enumerable: true, configurable: true });
    }
    print(A.length);
    print(A[0], A[999], A[1000], A[1e9]);
    //print(JSON.stringify(Duktape.info(A)));

    // This abandons it, array would become too sparse.
    Object.defineProperty(A, 1e9, { value: 'bar', writable: true, enumerable: true, configurable: true });
    print(A.length);
    print(A[0], A[999], A[1000], A[1e9]);
    //print(JSON.stringify(Duktape.info(A)));

    // These maintain the array part.
    A = [];
    for (i = 0; i < 1000; i++) {
        Object.defineProperty(A, i, { value: 'foo', writable: true, enumerable: true, configurable: true });
    }
    print(A.length);
    print(A[0], A[999], A[1000], A[1e9]);
    //print(JSON.stringify(Duktape.info(A)));

    // This abandons it, non-standard attributes.
    Object.defineProperty(A, 1000, { value: 'bar', writable: true, enumerable: false, configurable: true });
    print(A.length);
    print(A[0], A[999], A[1000], A[1e9]);
    //print(JSON.stringify(Duktape.info(A)));

    // These maintain the array part, despite there being gaps.
    A = [];
    for (i = 0; i < 1000; i += 2) {
        Object.defineProperty(A, i, { value: 'foo', writable: true, enumerable: true, configurable: true });
    }
    print(A.length);
    print(A[0], A[1], A[998], A[999], A[1000], A[1e9]);
    //print(JSON.stringify(Duktape.info(A)));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
print('done');
