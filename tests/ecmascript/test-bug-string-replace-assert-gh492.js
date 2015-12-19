/*
 *  https://github.com/svaarala/duktape/issues/492
 */

/*===
undefined
still here
undefinedfoo
still here
string ""
===*/

function test() {
    // Caused an assert failure in Duktape 1.3.0 and prior, and with asserts
    // disabled memory unsafe behavior.
    print(String.prototype.replace(RegExp.prototype));
    print('still here');

    // Similar case
    print('foo'.replace(RegExp.prototype));
    print('still here');

    // Ensure RegExp.prototype matches correctly (it's a RegExp instance too).
    var m = RegExp.prototype.exec('foo');
    print(typeof m[0], JSON.stringify(m[0]));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
