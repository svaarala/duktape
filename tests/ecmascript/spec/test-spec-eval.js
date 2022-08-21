/*
 *  Compilation and execution of eval code (E5 Sections 13, 10.4.2).
 */

var myEval = eval;  // for indirect evals

/*===
basic eval
foo
foo
===*/

print('basic eval');
print(eval("'foo'"));
print(myEval("'foo'"));

/*===
eval return value
123
number
===*/

/* var is an 'empty' statement, so 123 remains as the return value.  'test1'
 * gets declared to global object because the eval is a 'direct call' which
 * inherits the calling scope.
 */
print('eval return value');
print(eval("123; var test1=10;"));
print(typeof test1);

/*===
eval scope
number 123
number 234
number 123
undefined
object undefined
number
true
object undefined
undefined
false
===*/

/* Eval lexical and variable environment setup:
 *
 * - A direct call to eval of non-strict code inherits the environment of the
 *   calling context (both lexical and variable environment).  Eval code sees
 *   parent variables, declarations go to parent directly.
 *
 * - A direct call to eval of strict code gets a fresh environment mapped to
 *   both lexical and variable environments.  The fresh environment has the
 *   calling context as its parent, so eval code sees parent variables, while
 *   new declarations go to the new scope which is erased on exit.
 *
 * - An indirect call to eval of non-strict code inherits the global environment
 *   as its lexical and variable environment.  Eval code only sees the global
 *   environment (not a calling context if eval() is called from a function),
 *   declarations go to the global object.
 *
 * - An indirect call to eval of strict code gets a fresh environment mapped
 *   to both lexical and variable environments.  The fresh environment has the
 *   global environment as its parent, so eval sees global variables (but not
 *   the calling context's variables if called from a function).  New
 *   declarations go to the new scope which is erased on exit.
 */

function testEvalScope() {
    var x = 123;
    var global = new Function('return this;')();

    eval('print(typeof x, x); var y = 234;');
    print(typeof y, y);

    eval('"use strict"; print(typeof x, x); var z = 345');
    print(typeof z);

    myEval('print(typeof Math, typeof x); var test2a=10;');
    print(typeof test2a);
    print('test2a' in global);

    myEval('"use strict"; print(typeof Math, typeof x); var test2b=10;');
    print(typeof test2b);
    print('test2b' in global);
}

print('eval scope');
testEvalScope();

/*===
eval this binding
number object string this-binding
number object string this-binding
undefined object object
undefined object object
===*/

/* Eval 'this' binding:
 *
 * - Direct call to eval (strict or non-strict): inherit from calling context.
 *
 * - Indirect call to eval (strict or non-strict): global object.
 */

function testEvalThisBinding() {
    var x = 123;
    var global = new Function('return this;')();

    eval('print(typeof x, typeof this, typeof this.message, this.message);');
    eval('"use strict"; print(typeof x, typeof this, typeof this.message, this.message);');
    myEval('print(typeof x, typeof this, typeof this.Math);');
    myEval('"use strict"; print(typeof x, typeof this, typeof this.Math);');
}

print('eval this binding');
testEvalThisBinding.call({ message: 'this-binding' });
