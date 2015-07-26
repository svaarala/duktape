/*
 *  Direct eval detection handling in duk_bi_global.c refers to the calling
 *  activation, so tail calls might interfere with the process.
 *
 *  Check that this works correctly.  Direct eval code inherits strictness
 *  from the calling code, which can be used to detect whether or not the
 *  handling is correct.  Strict eval doesn't get a this binding, so '!this'
 *  will be true (false for non-strict eval code).
 */

/*===
eval false
f0 false
f0s false
f1 false
f1s true
f2 false
f2s true
===*/

function f0() {
    var ret;
    var my_eval = eval;
    ret = my_eval('!this');  // indirect eval
    return ret;
}

function f0s() {
    'use strict';
    var ret;
    var my_eval = eval;
    ret = my_eval('!this');  // indirect eval
    return ret;
}

function f1() {
    var ret;
    ret = eval('!this');
    return ret;
}

function f1s() {
    'use strict';
    var ret;
    ret = eval('!this');
    return ret;
}

function f2() {
    return eval('!this');
}

function f2s() {
    'use strict';
    return eval('!this');
}

try {
    print('eval', eval('!this'));
    print('f0', f0());
    print('f0s', f0s());
    print('f1', f1());
    print('f1s', f1s());
    print('f2', f2());
    print('f2s', f2s());
} catch (e) {
    print(e);
}
