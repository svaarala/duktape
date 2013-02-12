/*
 *  Tests on compiler recursion limits.
 */

/* FIXME: any other recursion points?
 *
 * Object and array literals?
 */

var t;

/*===
1
Error
===*/

/* Parenthesis; expression recursion */

function buildParenExpr(count) {
    var t = '1';
    var i;

    for (i = 0; i < count; i++) {
        t = '(' + t + ')';
    }

    return t;
}

try {
    /* a reasonable nest count should of course compile */
    print(eval(buildParenExpr(25)));
} catch (e) {
    print(e.name);
}

try {
    /* an internal error without other trouble should happen */
    print(eval(buildParenExpr(1000)));
} catch (e) {
    print(e.name);
}

/*===
innermost block
Error
===*/

/* Statement recursion */

function buildBlockExpr(count) {
    var t = "'innermost block';";  // implicit return value
    var i;

    for (i = 0; i < count; i++) {
        t = "if (true) { " + t + "}";
    }

    return t;
}

try {
    // reasonable count
    print(eval(buildBlockExpr(15)));
} catch (e) {
    print(e.name);
}

try {
    print(eval(buildBlockExpr(1000)));
} catch (e) {
    print(e.name);
}

/*===
innermost func
Error
===*/

/* Function recursion */

function buildFuncExpr(count) {
    var t = "(function(){ print('innermost func'); })";
    var i;

    for (i = 0; i < count; i++) {
        // t: function expression (not called)
        t = "(function(){" + t + "()" + "})";
    }

    t = t + "()";  // call the outermost
    return t;
}

try {
    // reasonable
    eval(buildFuncExpr(10));
} catch (e) {
    print(e.name);
}

try {
    eval(buildFuncExpr(1000));
} catch (e) {
    print(e.name);
}

