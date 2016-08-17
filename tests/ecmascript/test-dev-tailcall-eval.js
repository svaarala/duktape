/*
 *  The built-in eval() is not tailcalled.  In Duktape 2.x tailcalls are also
 *  disabled if an unrelated function is bound to the identifier 'eval'.  This
 *  is an artifact of the implementation.
 */

/*---
{
    "custom": true
}
---*/

/*===
Reached 1000000
done
RangeError
===*/

function test() {
    var notEval = function myNotEval(count) {
        if (count >= 1e6) {
            print('Reached', count);
            return 'done';
        } else {
            return notEval(count + 1);
        }
    }

    var eval = function myFakeEval(count) {
        if (count >= 1e6) {
            print('Reached', count);
            return 'done';
        } else {
            return eval(count + 1);
        }
    }

    // This will tail call properly.
    try {
        print(notEval(0));
    } catch (e) {
        print(e.name);
    }

    // This won't, because of the identifier 'eval' in use.
    try {
        print(eval(0));
    } catch (e) {
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
