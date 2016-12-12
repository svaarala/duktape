/*
 *  https://github.com/svaarala/duktape/issues/492
 */

/*===

still here
foo
still here
TypeError
still here
===*/

function test() {
    // Caused an assert failure in Duktape 1.3.0 and prior, and with asserts
    // disabled memory unsafe behavior.
    try {
        print(String.prototype.replace(RegExp.prototype));
    } catch (e) {
        print(e.name);
    }
    print('still here');

    // Similar case
    try {
        print('foo'.replace(RegExp.prototype));
    } catch (e) {
        print(e.name);
    }
    print('still here');

    try {
        var m = RegExp.prototype.exec('foo');
        print(typeof m[0], JSON.stringify(m[0]));
    } catch (e) {
        print(e.name);
    }
    print('still here');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
