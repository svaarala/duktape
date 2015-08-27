/*
 *  Memory unsafety issue when developing GH-294.
 */

/*===
f
g
Error: myerror
===*/

function f() {
    print('f');
    function g() {
        print('g');
        throw new Error('myerror');
    }
    g();
}

try {
    // Calling as f() does not trigger the error, but calling through
    // apply() does.  The internal difference is that apply() goes through
    // duk_handle_call().
    f.apply();
} catch (e) {
    print(e);
}
