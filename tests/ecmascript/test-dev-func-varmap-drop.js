/*
 *  Function _Varmap can be dropped when there can be no runtime need for it.
 *  Exercise current conditions.  Needs to be executed with debugger support
 *  disabled (otherwise _Varmap will always be present).
 */

/*---
{
    "custom": true
}
---*/

/*@include util-object.js@*/

/*===
- pr(arg)
hello
prop count diff: 0
- Math.PI
3.141592653589793
prop count diff: 0
3.141592653589793
prop count diff: 0
- direct eval
3.141592653589793
prop count diff: 2
3.141592653589793
prop count diff: 0
- inner function
3.141592653589793
prop count diff: 1
3.141592653589793
prop count diff: 1
- try-catch without a slow path access in catch clause
3.141592653589793
prop count diff: 1
3.141592653589793
prop count diff: 0
- try-catch with a slow path access in catch clause
3.141592653589793
prop count diff: 1
3.141592653589793
prop count diff: 0
- with statement without a slow path access
3.141592653589793
prop count diff: 0
3.141592653589793
prop count diff: 0
- with statement with a slow path access
3.141592653589793
prop count diff: 1
3.141592653589793
prop count diff: 0
- arguments object access
3.141592653589793
object
prop count diff: 2
3.141592653589793
object
prop count diff: 0
- test eval creating a local variable for a function without _Varmap
123
prop count diff: 2
123
prop count diff: 0
- inner function without varmap, access outer function variables
123
prop count diff: 1
123
prop count diff: 1
===*/

function test() {
    var func;
    var propsBaseline;

    // Baseline case.
    func = function foo() {};
    func();
    propsBase = getObjectPropertyCount(func);
    function printPropDiff() {
        print('prop count diff: ' + (getObjectPropertyCount(func) - propsBase));
    }

    // Basic case: all identifier accesses are to register variables
    // and no slow path accesses are made.
    print('- pr(arg)');
    func = function foo(pr, arg) { pr(arg); };
    func(print, 'hello');
    printPropDiff();

    // Quite basic case: print and Math are slow path accesses that cannot
    // match statically declared locals.
    print('- Math.PI');
    func = function foo(a,b) { print(Math.PI); };
    func();
    printPropDiff();
    func = function foo() { print(Math.PI); };
    func();
    printPropDiff();

    // Eval in the function might introduce new variable declarations at
    // run time, so presence of a (direct) eval requires _Varmap.
    // If there are no register-bound variables in the _Varmap,
    // it can still be omitted.
    //
    // Note that eval() here also causes _Formals to be kept, unless there
    // are no formal arguments at all.
    print('- direct eval');
    func = function foo(a,b) { print(Math.PI); eval(1); };
    func();
    printPropDiff();
    func = function foo() { print(Math.PI); eval(1); };
    func();
    printPropDiff();

    // Inner functions could potentially need the _Varmap so any inner
    // function declarations (even if not invoked) cause _Varmap to be
    // kept at present.  It would be possible to detect if the inner
    // function(s) actually make any dangerous slow path accesses, and
    // only keep the outer function's _Varmap if really necessary.
    // However, the compiler doesn't have enough state right now to do
    // this.
    //
    // Note that an inner function always creates a register based local
    // variable, so that _Varmap is never empty even if there are no formal
    // arguments or variable declarations/consts.
    print('- inner function');
    func = function foo(a,b) { print(Math.PI); function inner() {}; };
    func();
    printPropDiff();
    func = function foo() { print(Math.PI); function inner() {}; };
    func();
    printPropDiff();

    // Try-catch statement's catch variable is handled using a dynamic
    // scope object.  Identifier accesses within the catch block use slow
    // path access, and for simplicity the _Varmap is kept if any slow
    // path accesses are made there.  _Varmap could be omitted if the catch
    // clause doesn't perform a slow path access in practice -- but this
    // won't happen now because the automatic catch clause prologue will
    // perform a slow path access to load the catch variable into a register.
    //
    // When there are no register-based declarations (no formals, no variables,
    // no inner functions) _Varmap is still omitted because it is empty.
    print('- try-catch without a slow path access in catch clause');
    func = function foo(a,b) { print(Math.PI); try { throw new Error('aiee'); } catch (e) {} };
    func();
    printPropDiff();
    func = function foo() { print(Math.PI); try { throw new Error('aiee'); } catch (e) {} };
    func();
    printPropDiff();

    // But with a slow path access in the catch clause the _Varmap is
    // kept, even if there's no way for the catch clause to run.
    print('- try-catch with a slow path access in catch clause');
    func = function foo(a,b) { print(Math.PI); try {} catch (e) { print(Math); } };
    func();
    printPropDiff();
    func = function foo() { print(Math.PI); try {} catch (e) { print(Math); } };
    func();
    printPropDiff();

    // Presence of a with statement alone is not enough to cause _Varmap to be
    // kept.
    print('- with statement without a slow path access');
    func = function foo(a,b) { print(Math.PI); with({}) {} };
    func();
    printPropDiff();
    func = function foo() { print(Math.PI); with({}) {} };
    func();
    printPropDiff();

    // But any variable access inside the with statement causes _Varmap to
    // be kept, unless there are no register-bound variables at all.
    print('- with statement with a slow path access');
    func = function foo(a,b) { print(Math.PI); with({}) { Math; } };
    func();
    printPropDiff();
    func = function foo() { print(Math.PI); with({}) { Math; } };
    func();
    printPropDiff();

    // Access to 'arguments' specifically causes _Varmap to be kept.  It's
    // handled as a special case in the compiler, so also worth testing
    // separately.
    print('- arguments object access');
    func = function foo(a,b) { print(Math.PI); print(typeof arguments); };
    func();
    printPropDiff();
    func = function foo() { print(Math.PI); print(typeof arguments); };
    func();
    printPropDiff();

    // Regardless of the optimizations above, eval must be able to create
    // local variables (even if _Varmap is dropped).  This happens in the
    // latter case without formals (so that _Formals.length is 0 and there
    // are no register bound variables).
    print('- test eval creating a local variable for a function without _Varmap');
    func = function foo(a,b) { eval('var z = 123;'); print(z); };
    func();
    printPropDiff();
    func = function foo() { eval('var z = 123;'); print(z); };
    func();
    printPropDiff();

    // Test that if an inner function has no _Varmap, it can still access
    // the outer function's varmap.
    print('- inner function without varmap, access outer function variables');
    func = function foo(a,b) { var z = 123; function inner() { print(z); } inner(); };
    func();
    printPropDiff();
    func = function foo() { var z = 123; function inner() { print(z); } inner(); };
    func();
    printPropDiff();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
