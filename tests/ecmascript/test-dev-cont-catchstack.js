/*
 *  Test that we can continue after a catchstack limit error.
 *
 *  Since Duktape 2.2.0 there isn't an explicit catch stack limit: catchers
 *  are allocated normally and an out-of-memory may occur when trying to
 *  allocate a catcher.  This test would now fail with callstack limit and
 *  is disabled.
 */

// Skipped since Duktape 2.2.0: no explicit catchstack limits.
/*---
{
    "custom": true,
    "skip": true
}
---*/

/*===
RangeError: callstack limit
true
still here
===*/

/* The current limits for callstack and catchstack are the same
 * (10000), so a recursive call with more than one catcher should
 * ensure that the catchstack is exhausted first.
 *
 * This only works (= hits catchstack limit) if valstack limit is
 * significantly higher (currently 100000).
 */

function f1() {
    try {
        try {
            try {
                try {
                    f1();
                } finally {
                }
            } finally {
            }
        } finally {
        }
    } finally {
    }
}

try {
    f1();
} catch (e) {
    print(e.name + ': ' + e.message);

    // ensure that it is indeed the catchstack which failed; Rhino and V8
    // will fail this test of course
    print(/catch/i.test(e.message));
}

print('still here');

/*===
6765
===*/

/* Test that we can continue normally.  Just a simple recursive call test here. */

function fib(x) {
    if (x == 0) { return 0; }
    if (x == 1) { return 1; }
    return fib(x - 1) + fib(x - 2);
}

try {
    print(fib(20));
} catch (e) {
    print(e.name);
}
