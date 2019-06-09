/*
 *  https://github.com/svaarala/duktape/issues/2021
 */

/*===
error
done
===*/

function basicTest() {
    function test(x) {
        t = new basicTest( 'i' );
    }
    var values = [];
    var i;
    test(values [0]);
}

try {
    basicTest();
    print('success');
} catch (e) {
    // Ignore specific error, expectation is to run out of
    // call stack.
    print('error');
}

print('done');
