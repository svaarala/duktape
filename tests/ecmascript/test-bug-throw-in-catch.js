/*
 *  Bug test case for thread termination caused by an uncaught error when a
 *  catch stack is simultaneously active.  The longjmp handler unwound the
 *  call stack and catch stacks in an incorrect order in this case.
 *  This was originally modified from test-dev-coroutine-basic.js.
 */

/*---
{
    "custom": true
}
---*/

/*===
resume tests
yielder starting
yielder arg: foo
yielded with 1
resumed with bar
yielded with 2
resumed with quux
yielded with 3
resumed with baz
Error: error 2
finished
===*/

function yielder(x) {
    var yield = Duktape.Thread.yield;

    print('yielder starting');
    print('yielder arg:', x);

    print('resumed with', yield(1));
    print('resumed with', yield(2));
    print('resumed with', yield(3));

    // A forever loop here was used to cause an execution step limit
    // error, which triggered the incorrect thread termination.  The
    // underlying cause is thread termination from an error uncaught
    // by the thread in question, with a catchstack simultaneously
    // active.  Here the critical thing was to throw an error from the
    // catch block, which is uncaught but the try-catch state is still
    // in place and was unwound in an incorrect order.

    try {
        throw new Error('error 1');
    } catch (e) {
        throw new Error('error 2');
    }

    print('yielder ending');
    return 123;
}

var t = new Duktape.Thread(yielder);

try {
    print('resume tests');
    print('yielded with', Duktape.Thread.resume(t, 'foo'));
    print('yielded with', Duktape.Thread.resume(t, 'bar'));
    print('yielded with', Duktape.Thread.resume(t, 'quux'));
    print('yielded with', Duktape.Thread.resume(t, 'baz'));
} catch (e) {
    print(e);
}

print('finished');
