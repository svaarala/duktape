/*
 *  https://github.com/svaarala/duktape/issues/131
 */

/*===
test1 2
test1notail 2
test1var 2
test2 2
test2notail 2
test2var 2
test3 2
test3notail 2
test3var 2
test4 2
test4notail 2
test4var 2
===*/

function test1() {
    return 0, 1, 2;
}

function test1notail() {
    'use duk notail';
    return 0, 1, 2;
}

function test1var() {
    var ret = (0, 1, 2);
    return ret;
}

function test2() {
    return 0, (function() { return 1; })(), 2;
}

function test2notail() {
    return 0, (function() { 'use duk notail'; return 1; })(), 2;
}

function test2var() {
    var ret = (0, (function() { return 1; })(), 2);
    return ret;
}

function test3() {
    return (function() { return 0, 1, 2; })();
}

function test3notail() {
    return (function() { 'use duk notail'; return 0, 1, 2; })();
}

function test3var() {
    var ret = (function() { return 0, 1, 2; })();
    return ret;
}

function test4() {
    return (function() { return 0, (function () { return 1; })(), 2 })();
}

function test4notail() {
    return (function() { return 0, (function () { 'use duk notail'; return 1; })(), 2 })();
}

function test4var() {
    var ret = (function() { return 0, (function () { return 1; })(), 2 })();
    return ret;
}

try {
    print('test1', test1());
    print('test1notail', test1notail());
    print('test1var', test1var());
    print('test2', test2());
    print('test2notail', test2notail());
    print('test2var', test2var());
    print('test3', test3());
    print('test3notail', test3notail());
    print('test3var', test3var());
    print('test4', test4());
    print('test4notail', test4notail());
    print('test4var', test4var());
} catch (e) {
    print(e.stack || e);
}
