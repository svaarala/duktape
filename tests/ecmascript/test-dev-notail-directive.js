/*
 *  Duktape has a custom 'use duk notail' directive which prevents tailcalls.
 *  This matters to special functions which rely on callstack shape.
 */

/*===
test1: act test1func useNotailDirectiveTest
test2: act test2func test2 useNotailDirectiveTest
===*/

function test1func() {
    // tailcall, call stack indices:
    // -1 = Duktape.act, -2 = test1func, -3 = useNotailDirectiveTest
    // Duktape.act.name is not printed directly because it may be a lightfunc
    print('test1:',
          Duktape.act(-1).function === Duktape.act ? 'act' : '???',
          Duktape.act(-2).function.name,
          Duktape.act(-3).function.name);
}

function test1() {
    return test1func();
}

function test2func() {
    'use duk notail';

    // no tailcall, call stack indices:
    // -1 = Duktape.act, -2 = test1func, -3 = test1, -4 = useNotailDirectiveTest
    // Duktape.act.name is not printed directly because it may be a lightfunc
    print('test2:',
          Duktape.act(-1).function === Duktape.act ? 'act' : '???',
          Duktape.act(-2).function.name,
          Duktape.act(-3).function.name,
          Duktape.act(-4).function.name);
}
function test2() {
    return test2func();
}

function useNotailDirectiveTest() {
    test1();
    test2();
}

try {
    useNotailDirectiveTest();
} catch (e) {
    print(e);
}
