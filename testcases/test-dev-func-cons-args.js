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
     * work correctly.  The formal argument list to be parsed is:
     *
     *    a,b, c // last
     *
     * This fails in V8 and Rhino, though.  For example, Rhino will complain:
     *
     *    SyntaxError: missing ) after formal parameters
     *
     * which seems incorrect because FormalParameterList_opt does NOT include parentheses
     * at all (just the identifiers and commas).
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


