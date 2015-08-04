/*
 *  Function with large 'nregs', large enough to require a resize before
 *  duk_set_top().
 */

/*===
4
-> 3
-> 3
12
-> 11
-> 11
28
-> 27
-> 27
60
-> 59
-> 59
124
-> 123
-> 123
252
-> 251
-> 251
508
-> 507
-> 507
1020
-> 1019
-> 1019
2044
-> 2043
-> 2043
4092
-> 4091
-> 4091
8188
-> 8187
-> 8187
16380
-> 16379
-> 16379
32764
-> 32763
-> 32763
65532
-> 65531
-> 65531
===*/

function createFunc(n) {
    var res = [];
    var i;

    res.push('(function func() { ');
    for (i = 0; i < n; i++) {
        res.push('    var arg' + i + ' = ' + i + ';');
    }
    res.push('    return arg' + (i - 1) + ';');
    res.push('})');

    return res.join('\n');
}

function test() {
    var src, fn;
    var i;

    // Max nregs/nargs for an Ecmascript function is now limited to 16 bits
    // i.e. 65535.  Take shuffle regs into account.

    for (i = 8; i <= 65536; i *= 2) {
        print(i - 1 - 3);
        src = createFunc(i - 1 - 3);
        fn = eval(src);

        // This happens using an ecma-to-ecma call
        print('->', fn());

        // This happens (currently) using a C-to-ecma call because call() and
        // apply() are implemented as plain C functions
        print('->', fn.call());
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
