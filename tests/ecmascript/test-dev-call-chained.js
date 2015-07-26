/*
 *  Production for:
 *
 *    f(1).g(2);
 *
 *  comes from:
 *
 *    LeftHandSideExpression -> CallExpression
 *                           -> CallExpression Arguments
 *                           -> (CallExpression . IdentifierName) Arguments
 *                           -> ((MemberExpression Arguments) . IdentifierName) Arguments
 *                           -> ((PrimaryExpression Arguments) . IdentifierName) Arguments
 *                           -> ((Identifer Arguments) . IdentifierName) Arguments
 *
 *  So, the original expression is equivalent to:
 *
 *    (f(1)).g(2);
 *
 *  Note that the first name is an Identifier, i.e. does not allow reserved words.
 *  However, the latter is not, so this is valid:
 *
 *    foo(1).if(2)
 */

/*===
f 1
g 2
f 1
g 2
f 3
g 4
===*/

function f(x) {
    print('f', x);
    return {
        "g": function(y) {
                 print('g', y);
             }
    }
}

function z(x, y) {
    f(x).g(y);
}

try {
    f(1).g(2);
} catch (e) {
    print(e.name);
}

try {
    (f(1)).g(2);
} catch (e) {
    print(e.name);
}

try {
    z(3, 4);
} catch (e) {
    print(e.name);
}
