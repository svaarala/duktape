/*
 *  https://github.com/svaarala/duktape/issues/413
 *  https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/parseInt#ECMAScript_5_Removes_Octal_Interpretation
 */

/*===
123
83
===*/

function test() {
    print(parseInt('0123'));     // no automatic octal detection
    print(parseInt('0123', 8));  // parse octal with explicit radix
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
