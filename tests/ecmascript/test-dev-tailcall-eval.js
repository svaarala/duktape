/*
 *  The built-in eval() is not tailcalled.  In Duktape 2.1 and 2.0 tailcalls
 *  are also disabled if an unrelated function is bound to the identifier
 *  'eval'.  This is an artifact of the implementation, fixed in Duktape 2.2.
 */

/*---
{
    "custom": true
}
---*/

/*===
Reached 1000000
done
Reached 1000000
done
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

    // This won't in Duktape 2.0 and 2.1, but will in Duktape 2.2.
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
