/*===
throw
throw
throw
===*/

/* This was broken at an early point: the compiler would use a function-wide
 * register binding for 'e' and look up the argument / variable declaration
 * instead of the (dynamic) catch binding.
 *
 * h() would actually work because 'with' turns off fast path identifier
 * lookups for its duration.
 */

function f(e) {
    try {
        throw 'throw';
    } catch (e) {
        print(e);
    }
}

function g() {
    var e = 'var';

    try {
        throw 'throw';
    } catch (e) {
        print(e);
    }
}

function h() {
    var e = 'var';

    with ({}) {
        try {
            throw 'throw';
        } catch (e) {
            print(e);
        }
    }
}

f('arg');
g();
h();
