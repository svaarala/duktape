/*===
closure throw
123
closure throw
123
closure throw
===*/

/* This was broken at an early point.  Catch binding variable would be mapped
 * to a certain register for the duration of the catch clause.  A closure
 * created from the catch clause would work incorrectly once the original
 * invocation exited the catch clause.
 */

function f() {
    var e = 123;
    var func;

    try {
        throw 'throw';
    } catch (e) {
        func = function() { print('closure', e); }
        func();
    }

    print(e);
    func();
    print(e);

    return func;
}

f()();
