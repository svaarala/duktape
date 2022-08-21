/*
 *  Various lexer/parser cases for regexp literals.
 */

/*===
10
2
===*/

var val = 20;
var foo = 2;
var g = 5;

try {
    print(eval("val/2"));
} catch (e) {
    print(e.name);
}

try {
    // parses as (val)/(foo)/(g) = 20/2/5 = 2
    print(eval("val/foo/g"));
} catch (e) {
    print(e.name);
}

/*===
SyntaxError
ReferenceError
===*/

/* This is a SyntaxError, as the ++ has no base value.  (V8 seems to
 * cause a ReferenceError for this.)
 */

try {
    eval("++/foo/");
    print('never here');
} catch (e) {
    print(e.name);
}

/* This parses as a RegExp literal and causes an "invalid LHS" ReferenceError. */

try {
    eval("/foo/++");
    print('never here');
} catch (e) {
    print(e.name);
}

/*===
object
50
500
===*/

/* Specific cases for testing the advance() implementation. */

function returnObject() {
    return {
        return: 1000
    }
}

try {
    /* RegExp allowed after a 'return' statement. */
    print(eval("typeof (function() { return /foo/; })();"));
} catch (e) {
    print(e.name, e.message);
}

try {
    /*
     *  Don't allow a RegExp after a 'return' which occurs as a property access name
     *  (MemberExpression).
     *
     *  This breaks easily if 't' is used to determine whether RegExp can follow:
     *  'return' keyword allows a RegExp to follow, but 'return' as an IdentifierName
     *  does not.
     */

    print(eval("(function() { var foo = { return: 100 }; return foo.return/2; })();"));
} catch (e) {
    print(e.name);
}

try {
    /*
     *  Don't allow a RegExp after a 'return' which occurs as a property access name
     *  after a function call (CallExpression).
     */

    print(eval("returnObject().return/2"));
} catch (e) {
    print(e.name);
}
