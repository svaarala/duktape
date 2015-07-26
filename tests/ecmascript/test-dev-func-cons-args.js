/*
 *  E5 Section 15.3.2: function constructor argument list is not
 *  necessarily just identifier names, a call argument may contain
 *  multiple argument declarations.
 *
 *  The arguments joined with "," should parse as a FormalParameterList_opt.
 *  Apparently this means that the source can contain newlines and comments
 *  too!
 */

/*===
6
6
6
6
6
6
===*/

var f1, f2, f3, f4, f5, f6;

try {
    print(eval('f1 = new Function("a", "b", "c", "return a+b+c"); f1(1,2,3);'));
} catch (e) {
    print(e.name);
}

try {
    print(eval('f2 = new Function("a, b, c", "return a+b+c"); f2(1,2,3);'));
} catch (e) {
    print(e.name);
}

try {
    print(eval('f3 = new Function("a, b", "c", "return a+b+c"); f3(1,2,3);'));
} catch (e) {
    print(e.name);
}

try {
    print(eval('f4 = new Function("a /*first*/", "b /*second*/, c /*third*/", "return a+b+c"); f4(1,2,3);'));
} catch (e) {
    print(e.name);
}

try {
    /* It's unclear in the specification whether this should be a SyntaxError or not.
     * If one naively constructs a parse string as:
     *
     *    function (a, b, c //last) { return a+b+c }
     *
     * Then this would be a syntax error.  On the other hand, the specification does not
     * say that this should be done; rather, the argument list, joined by commas, should
     * be parsed as a FormalParameterList_opt and the body should be parsed as a
     * FunctionBody, separately.  So we interpret this to mean that this should actually
     * work correctly, as comments are allowed anywhere in source code.  The formal
     * argument list to be parsed is:
     *
     *    a,b, c // last
     *
     * This fails in V8 and Rhino, though.  For example, Rhino will complain:
     *
     *    SyntaxError: missing ) after formal parameters
     *
     * V8 (Node) will complain:
     *
     *    [SyntaxError: Unexpected token return]
     *
     */

    print(eval('f5 = new Function("a", "b, c // last", "return a+b+c"); f5(1,2,3);'));
} catch (e) {
    print(e.name);
}

try {
    /* Here the format argument list to be parsed would be:
     *
     *    a
     *    ,b
     *    ,c // last   [newline]
     *
     * Again, this should parse as a FormalParameterList_opt but fails to do so in
     * Rhino and V8.  Rhino will complain:
     *
     *    SyntaxError: unterminated string literal
     *
     * whereas V8 will complain:
     *
     *    [SyntaxError: Unexpected token ILLEGAL]
     */

    eval('f6 = new Function("a\n", "b\n,c // last\n", "return a+b+c"); f6(1,2,3);');
} catch (e) {
    print(e.name);
}

/*===
function () {if (1) {print('foo')}}
foo
SyntaxError
===*/

/* It would be tempting to implement the Function constuctor as follows:
 *
 *   1. Coerce the argument names and join with commas -> formals
 *   2. Coerce the function body -> body
 *   3. Let T = 'function(' + formals + ') { + body + '}'
 *
 * However, this would not be correct.  E5 Section 15.3.2.1 step 7
 * requires that 'formals' must parse as a FormalParameterList_opt
 * by itself.
 *
 * The test case below ensures that this is indeed the case.  The
 * following function:
 *
 *   function () {if (1) {print('foo')}}
 *   ==========--------===-------------=
 *
 * is split into parts in a sneaky way.
 */

var fake_arg = ") {if (1";
var fake_body = "print('foo')}";
var fake_val;

try {
    // this should work (parens used to ensure function expression parses)
    fake_val = 'function (' + fake_arg + ') {' + fake_body + '}';
    print(fake_val);
    eval('(' + fake_val + ') ()');
} catch (e) {
    print(e.name);
}

try {
    // this should not (but does, e.g. on Rhino)
    new Function(fake_arg, fake_body)();
} catch (e) {
    print(e.name);
}

/*===
OBJ1
OBJ2
OBJ3
7
===*/

/* The coercion order of arguments is specified to match function argument
 * order, i.e. ToString() coerce first the formal arguments and finally
 * the body.
 */

var obj1 = {
	toString: function() { print('OBJ1'); return 'x'; }
};
var obj2 = {
	toString: function() { print('OBJ2'); return 'y'; }
};
var obj3 = {
	toString: function() { print('OBJ3'); return 'print(x+y)'; }
};

try {
    new Function(obj1, obj2, obj3)(3,4);
} catch (e) {
    print(e.name);
}

/*===
undefined
foo
foo 1
foo 1 2
===*/

/* Misc additional tests for implementation-dependent reasons. */

try {
    // no arguments or body -> body evaluates to an empty string, so
    // should behave like "function() {}", i.e. return undefined
    print(new Function()());
} catch (e) {
    print(e.name);
}

try {
    // empty args
    new Function("print('foo')")();
} catch (e) {
    print(e.name);
}

try {
    // 1 arg
    new Function("x", "print('foo', x)")(1);
} catch (e) {
    print(e.name);
}

try {
    // 2 args
    new Function("x", "y", "print('foo', x, y)")(1, 2);
} catch (e) {
    print(e.name);
}
