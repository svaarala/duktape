/*
 *  Error handling callstack headroom (GH-191)
 *
 *  Check that relaxed callstack limit during error creation is still bounded
 *  so that an errCreate callback doing unbounded recursion is caught.  The
 *  result is a DoubleError.
 */

/*---
{
    "custom": true
}
---*/

/*===
DoubleError: error in error handling
===*/

try {
    function recurse() {
        var dummy;
        recurse();
        dummy = 1;
    }

    Duktape.errCreate = function(e) {
        recurse();
        return e;
    };

    function f() { f(); }
    f();
} catch(e) {
    print(e);
}
