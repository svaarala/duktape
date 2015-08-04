/*
 *  A bit messy direct eval tests.
 */

var global = this;
var orig_eval = eval;

/*===
false
"this" value
10
20
false
"this" value
20
20
5
===*/

/* Direct eval call needs to (1) be referenced with the name 'eval',
 * and (2) bind to the original eval() function.
 *
 * It does NOT need to be bound through the original environment
 * record, i.e. the global object.  A local variable with the name
 * 'eval' and which points to the built-in eval function is OK.
 */

var x = 5;

function f1() {
    var x = 10;

    // direct eval, non-strict eval code ->
    // * this binding maintained
    // * lexical/variable environment maintained

    print(eval("this === global"));
    print("" + eval("this"));
    print(eval("x"));           // binds to f1()'s var 'x'
    eval("var x = 20");         // modifies f1()'s var 'x'
    print(x);

    // direct eval, strict eval code ->
    // * this binding maintained
    // * lexical/variable environment is a fresh one

    print(eval("'use strict'; this === global"));
    print("" + eval("'use strict'; this"));
    print(eval("'use strict'; x"));           // "bleeds out", matches f1()'x var 'x'
    eval("'use strict'; var x = 30;");        // own env, no change to our 'x'
    print(x);
}

try {
    f1.call('"this" value');
    print(x);
} catch (e) {
    print(e.name);
}
/*===
true
Infinity
5
50
20
true
Infinity
15
50
20
15
20
===*/

function f2() {
    var x = 50;
    var indirect_eval = eval;

    // indirect_eval("x") and indirect_eval("var x = 15") fails
    // in V8 for some reason

    // indirect eval call, non-strict eval code ->
    // * this binding is global object
    // * lexical/variable environment is the global object

    print(indirect_eval("this === global"));
    print(indirect_eval("this.Number.POSITIVE_INFINITY"));
    print(indirect_eval("x"));    // binds to global 'x'
    indirect_eval("var x = 15");  // to global object
    indirect_eval("var y = 20");  // to global object
    print(x);                     // our 'x'
    print(y);                     // still prints, bleeds to global object

    // indirect eval call, strict eval code
    // * this binding is global object
    // * lexical/variable environment is a fresh one

    print(indirect_eval("'use strict'; this === global"));
    print(indirect_eval("'use strict'; this.Number.POSITIVE_INFINITY"));
    print(indirect_eval("'use strict'; x"));  // bleeds out, binds to global 'x'
    indirect_eval("'use strict'; var x = 100");  // own copy
    indirect_eval("'use strict'; var y = 200");  // own copy
    print(x);                     // our 'x'
    print(y);                     // still prints, bleeds to global object
}

try {
    f2.call('"this" value');
    print(x);
    print(y);
} catch (e) {
    print(e.name);
}

/*===
"this" value
===*/

/* Direct eval call through a local variable of name 'eval'.
 * Note that we can't simply say 'var eval = eval' because
 * that would ALWAYS assign undefined to 'eval' (variable
 * declarations happen before any right-hand-sides are
 * evaluated).
 */

function f3() {
    var eval = orig_eval;

    // this is a direct eval -> this binding should remain
    print("" + eval('this'));
}

try {
    f3.call('"this" value');
} catch (e) {
    print(e.name);
}

/*===
false
fake eval
true
false
===*/


/* An illustration that a certain eval() call may change from a direct eval
 * to an indirect one dynamically.  There are other ways to get the same
 * effect.
 *
 * The impact is that whenever a variable named 'eval' is encountered by
 * the compiler, it must always generate slow path code for it.  In
 * particular, CSVAR must be used for call setup (not CSREG).
 */

var binding_obj = {};

function wrapper() {
    var ret;
    with (binding_obj) {
        ret = function() {
            // direct eval   --> this_binding is caller's "this binding" --> prints false
            // indirect eval --> this_binding is global object --> prints true
            print(eval('this === global'));
        }
    }
    return ret;
}

try {
    var func = wrapper();
    var my_obj = { func: func };  // call through this to get a this binding != global object

    // initially, binding_obj has no 'eval' binding, hence direct eval
    my_obj.func();

    // add 'intercepting' eval binding, hence indirect eval
    binding_obj.eval = function(x) { print('fake eval'); return orig_eval(x); };
    my_obj.func();

    // remove intercepting binding, again a direct eval
    delete binding_obj.eval;
    my_obj.func();
} catch (e) {
    print(e.name);
}

/*===
forced this
Infinity
Infinity
===*/

/* Is it a direct eval call if an "eval()" target is a bound function
 * whose target function is the built-in eval?
 *
 * The answer in the spec is no: the result of GetValue() for "eval"
 * must be the built-in eval function for the call to be considered
 * direct.
 *
 * Check that we follow these semantics.
 */

var bound_eval = eval.bind('bound this');  // bind 'this' to 'bound this'
var indirect_eval = eval;

function bound1() {
    // direct eval, this should bound to 'forced this'
    print("" + eval("this"));
}

function bound2() {
    // indirect eval through a bound function, this should be
    // bound to the global object
    print(bound_eval("this.Number.POSITIVE_INFINITY"));
}

function bound3() {
    // indirect eval through a bound function, but eval code is
    // strict; this should still be bound to the global object
    print(bound_eval("'use strict'; this.Number.POSITIVE_INFINITY"));
}

try {
    /* The 'this' binding for eval() itself (here, the string "bound this")
     * should not really matter; the 'this' binding for the evaluated code
     * should still be as specified in E5 Section 10.4.2.
     *
     * The initial [[Call]] would happen using semantics from E5 Section
     * 15.3.4.5.1.  That algorithm would [[Call]] the built-in eval
     * function (E5 Section 10.4.2) which would ignore any 'this' binding.
     */

    bound1.call('forced this');
} catch (e) {
    print(e.name);
}

try {
    bound2.call('forced this');
} catch (e) {
    print(e.name);
}

try {
    bound3.call('forced this');
} catch (e) {
    print(e.name);
}

/*===
NaN
NaN
bar
bar
===*/

/* The "this binding" for a direct eval is always the caller's current this
 * binding; this is the case for both strict and non-strict eval code.
 *
 * For an indirect eval, "this binding" is set to the global object.
 */

function thisBinding_indirect_nonstrict() {
    var my_eval = eval;
    my_eval('print(this.NaN)');
}

function thisBinding_indirect_strict() {
    var my_eval = eval;
    my_eval('"use strict"; print(this.NaN)');
}

function thisBinding_direct_nonstrict() {
    eval('print(this.foo)');
}

function thisBinding_direct_strict() {
    eval('"use strict"; print(this.foo)');
}

try {
    thisBinding_indirect_nonstrict.call({foo:'bar'});
} catch (e) {
    print(e.name);
}

try {
    thisBinding_indirect_strict.call({foo:'bar'});
} catch (e) {
    print(e.name);
}

try {
    thisBinding_direct_nonstrict.call({foo:'bar'});
} catch (e) {
    print(e.name);
}

try {
    thisBinding_direct_strict.call({foo:'bar'});
} catch (e) {
    print(e.name);
}
