/*
 *  Some curr_pc sync bugs when developing GH-294.
 */

/*---
custom: true
---*/

/*===
calling inner
inner called
-1 0
-2 27
-3 34
-4 39
inner returned
===*/

/* When a function call is made, act->curr_pc must be synced so that
 * inner functions can see correct numbers in e.g. stacktraces, debugger
 * GetCallStack, Duktape.act(), etc.
 */

function funcCallTest() {
    function inner() {
        print('inner called');
        for (i = -1; (t = Duktape.act(i)) != null; i--) {
            print(i, t.lineNumber);
        }
        return 123;
    }

    print('calling inner');
    inner();
    print('inner returned');
}

try {
    funcCallTest();
} catch (e) {
    print(e.stack || e);
}

/*===
testing
getter called
-1 0
-2 65
-3 73
-4 76
123
===*/

/* The executor may do an automatic function call when it accesses a setter.
 * Curr_pc must be synced for these too.  This is automatic if syncing is done
 * by call handling.
 */

function sideEffectTest() {
    var obj = {};
    Object.defineProperty(obj, 'prop', {
        get: function () {
            var i, t;
            print('getter called');
            for (i = -1; (t = Duktape.act(i)) != null; i--) {
                print(i, t.lineNumber);
            }
            return 123;
        }
    });

    print('testing');
    print(obj.prop);
}

sideEffectTest();
