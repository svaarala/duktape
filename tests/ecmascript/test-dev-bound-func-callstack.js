/*
 *  The callstack and Duktape.act() reflect the final target function
 *  rather than the initial function when a bound function is invoked.
 *  Both behaviors would be useful in some use cases, this test case
 *  covers the current Duktape 1.x behavior.
 *
 *  The current behavior allows user code to use Duktape.act() to obtain
 *  a reference to the final target function of a bound function in the
 *  callstack.
 */

/*===
func called
typeof this: undefined
true
false
func
bound.fileName: dummy.js
bound.name: bound
func called
typeof this: number
true
false
func
===*/

function test() {
    function func() {
        'use strict';  // to avoid 'this' coercion
        print('func called');
        print('typeof this:', typeof this);
        print(Duktape.act(-2).function === func);
        print(Duktape.act(-2).function.fileName === 'dummy.js');
        print(Duktape.act(-2).function.name);
    }

    func();

    var bound = func.bind(123);
    Object.defineProperty(bound, 'fileName', { value: 'dummy.js' });
    Object.defineProperty(bound, 'name', { value: 'bound' });
    print('bound.fileName:', bound.fileName);
    print('bound.name:', bound.name);

    bound();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
