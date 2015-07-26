/*
 *  If a non-strict context contains a strict function, where do 'strict'
 *  semantics actually begin?
 *
 *  Clearly, all the statements inside the function obey strict semantics.
 *  What about the argument list of the function?  For instance, consider:
 *
 *    (function (a,a) { 'use strict'; return a; })(1,2);
 *
 *  Strict code cannot contain a function expression with two arguments
 *  of the same name.  However, in this case the function expression itself
 *  exists in non-strict code.
 *
 *  E5 Section 11.2.5 refers to Section 13 for the semantics of evaluating
 *  a FunctionExpression.  The specification text for a FunctionExpression
 *  is as follows:
 *
 *    The production
 *
 *    FunctionExpression : function ( FormalParameterList_opt ) { FunctionBody }
 *
 *    is evaluated as follows:
 *
 *    1. Return the result of creating a new Function object as specified in
 *       13.2 with parameters specified by FormalParameterList_opt and body
 *       specified by FunctionBody. Pass in the LexicalEnvironment of the
 *       running execution context as the Scope. Pass in true as the Strict
 *       flag if the FunctionExpression is contained in strict code or if
 *       its FunctionBody is strict code.
 *
 *  This refers to Section 13.2 for creating a function object.  The 'Strict'
 *  flag for that "call" is set to true here, because FunctionBody is strict
 *  code.  However, the algorithm of Section 13.2 does no strict-mode related
 *  checks for duplicate argument names.
 *
 *  E5 Section 13.1 describes strict mode restrictions, which include:
 *
 *    It is a SyntaxError if any Identifier value occurs more than once within
 *    a FormalParameterList of a strict mode FunctionDeclaration or
 *    FunctionExpression.
 *
 *  Does this apply?  The FunctionExpression itself is *not* in strict mode.
 *  FormalParameterList is presumably evaluated in non-strict mode.
 *
 *  When building functions with the Function constructor, the specification
 *  clearly states that strict mode restrictions are checked and applied
 *  after function parsing: E5 Section 15.3.2.1, step 10.  Thus it would
 *  seem reasonable to have the same semantics for other function parsing
 *  cases, too.
 *
 *  In any case, V8 will throw SyntaxError in this case, and we follows that
 *  behavior.  Test cases below check for this expected behavior.
 *
 *  Note that the strict mode restrictions of E5 Section 13.1 are a bit
 *  awkward to check from an implementation perspective: while we're parsing
 *  a function expression (in particular, a possible name and the argument
 *  list) we don't yet know that the function may turn out to be strict.
 *  The optional function name and the argument list need to be checked
 *  at some point, when we know we're dealing with a strict function.
 */

/*===
2
SyntaxError
===*/

/* Duplicate argument names */

try {
    // non-strict function -> acceptable syntax, latter parameter
    // is bound to 'a'
    eval("(function (a,a) { print(a); })(1,2);");
} catch (e) {
    print(e.name);
}

try {
    // SyntaxError
    eval("(function (a,a) { 'use strict'; print(a); })(1,2);");
} catch (e) {
    print(e.name);
}

/*===
1
SyntaxError
1
SyntaxError
===*/

/* Argument named 'eval' or 'arguments' */

try {
    eval("(function (eval) { print(eval); })(1);");
} catch (e) {
    print(e.name);
}

try {
    eval("(function (eval) { 'use strict'; print(eval); })(1);");
} catch (e) {
    print(e.name);
}

try {
    eval("(function (arguments) { print(arguments); })(1);");
} catch (e) {
    print(e.name);
}

try {
    eval("(function (arguments) { 'use strict'; print(arguments); })(1);");
} catch (e) {
    print(e.name);
}

/*===
1
SyntaxError
1
SyntaxError
===*/

/* Function name 'eval' or 'arguments' */

try {
    eval("(function eval(a) { print(a); })(1);");
} catch (e) {
    print(e.name);
}

try {
    eval("(function eval(a) { 'use strict'; print(a); })(1);");
} catch (e) {
    print(e.name);
}

try {
    eval("(function arguments(a) { print(a); })(1);");
} catch (e) {
    print(e.name);
}

try {
    eval("(function arguments(a) { 'use strict'; print(a); })(1);");
} catch (e) {
    print(e.name);
}
