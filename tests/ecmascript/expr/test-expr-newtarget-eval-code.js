/*
 *  new.target in eval code
 */

/*===
direct eval in program code
SyntaxError
indirect eval in program code
SyntaxError
direct eval in function code
undefined
false
function
true
indirect eval in function code
SyntaxError
eval-in-eval in function code
undefined
false
function
true
===*/

var myEval = eval;  // indirect eval call

// Not allowed in direct or indirect eval outside of a function.
try {
    print('direct eval in program code');
    eval('print(typeof new.target)');
} catch (e) {
    print(e.name);
}
try {
    print('indirect eval in program code');
    myEval('print(typeof new.target)');
} catch (e) {
    print(e.name);
}

// Allowed in direct eval inside a function call.
try {
    print('direct eval in function code');
    eval('(function test() { eval("print(typeof new.target); print(new.target === test);"); })()');
    eval('new (function test() { eval("print(typeof new.target); print(new.target === test);"); })');
} catch (e) {
    print(e.name);
}

// Not allowed in indirect eval inside a function call.
try {
    print('indirect eval in function code');
    eval('new (function test() { myEval("print(typeof new.target);"); })');
} catch (e) {
    print(e.name);
}

// This should be allowed (Firefox allows this) because GetNewTarget() just
// looks up [[NewTarget]] from the lexical environment and nested direct
// eval() calls just inherit the surrounding function's environment.
try {
    print('eval-in-eval in function code');
    eval('(function test() { eval("eval(\\"print(typeof new.target); print(new.target === test);\\");"); })()');
    eval('new (function test() { eval("eval(\\"print(typeof new.target); print(new.target === test);\\");"); })');
} catch (e) {
    print(e.stack);
}
