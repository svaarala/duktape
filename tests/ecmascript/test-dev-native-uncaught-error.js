/*
 *  Debugger testcase to test caught and uncaught error thrown from
 *  native code.
 */

/* Disabled by default, run manually. */

/*---
{
    "skip": true
}
---*/

function test() {
    // Caught errors thrown by native code
    [1, 2, 3].forEach(function (v) {
        print(v);
        try {
            decodeURIComponent('%ff');
        } catch (e) {
            print(e);
        }
    });

    // Uncaught errors thrown by native code
    [1, 2, 3].forEach(function (v) {
        print(v);
        decodeURIComponent('%ff');
    });
}

test();
