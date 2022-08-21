/*
 *  Specific test that when an error is propagating through a
 *  'finally' block, a try-catch with error caught doesn't
 *  affect the propagated value once finally is finished.
 *  Also checks that both the register and envrec binding for
 *  the catch variable is correct.
 */

/*===
finally
ignore quux
Error: foo
Error: foo
Error: bar
===*/

function test() {
    try {
        // this error really doesn't affect the test
        throw new Error('foo');
    } catch (e) {
        try {
            throw new Error('bar');
        } finally {
            print('finally');
            try { throw new Error('quux'); } catch (e) { print('ignore quux'); }

            print(e);
            eval('print(e)');
            // continue propagating original error ('bar')
        }
    }
}

try {
    test();
} catch (e) {
    print(e);
}
