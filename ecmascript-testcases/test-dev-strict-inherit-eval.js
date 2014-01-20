/*
 *  Eval strictness inheritance varying containing code strictness and
 *  direct/indirect eval.
 */

/*===
non-strict !this false
direct eval
ok
indirect eval
ok
strict !this true
SyntaxError
indirect eval
ok
===*/

function nonStrictTest() {
    var myeval = eval;
    print('non-strict', '!this', !this);

    // Both direct and indirect eval code are naturally non-strict and
    // 'with' is allowed

    try {
        eval("print('direct eval'); with({}) {}; print('ok');");
    } catch (e) {
        print(e.name);
    }
    try {
        myeval("print('indirect eval'); with({}) {}; print('ok');");
    } catch (e) {
        print(e.name);
    }
}

function strictTest() {
    'use strict';
    var myeval = eval;
    print('strict', '!this', !this);

    // Direct eval inherits strictness from containing code.

    try {
        eval("print('direct eval'); with({}) {}; print('ok');");
    } catch (e) {
        print(e.name);
    }

    // Indirect eval does not inherit strictness from calling code.
    // This is not specified very clearly in e.g. Section 10.4, but
    // is evident from Section 10.1.1 which states:
    //
    //   Eval code is strict eval code if it begins with a Directive
    //   Prologue that contains a Use Strict Directive or if the call
    //   to eval is a direct call (see 15.1.2.1.1) to the eval function
    //   that is contained in strict mode code.

    try {
        myeval("print('indirect eval'); with({}) {}; print('ok');");
    } catch (e) {
        print(e.name);
    }
}

try {
    nonStrictTest();
    strictTest();
} catch (e) {
    print(e);
}
