/*
 *  https://github.com/svaarala/duktape/issues/111
 */

/*---
{
    "custom": true
}
---*/

/*===
65531
Error
Error
Error
===*/

function test(n) {
    var res = [];
    var i;
    var fn;

    res.push('(function func() { ');
    for (i = 0; i < n; i++) {
        res.push('    var arg' + i + ' = ' + i + ';');
    }
    res.push('    return arg' + (i - 1) + ';');
    res.push('})');

    res = res.join('\n');

    fn = eval(res);
    return fn();
}

try {
    print(test(65535 - 3));  // should work, -3 for shuffle regs
} catch (e) {
    print(e.name);
}

try {
    print(test(65536 - 3));  // should be rejected with internal error now
} catch (e) {
    print(e.name);
}

try {
    print(test(262143 - 3));  // should be rejected with internal error now
} catch (e) {
    print(e.name);
}

try {
    print(test(262144 - 3));  // should be rejected with internal error now
} catch (e) {
    print(e.name);
}
