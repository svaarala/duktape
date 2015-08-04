/*===
1
2
===*/

try {
    eval("print(true ? 1 : 2);");
} catch (e) {
    print(e.name);
}

try {
    eval("print(false ? 1 : 2);");
} catch (e) {
    print(e.name);
}


/*===
SyntaxError
1 3
===*/

/* In "A ? B : C" both B and C are AssignmentExpressions, and
 * do not allow the comma operator.
 *
 * In the case of "true ? 1,2 : 3" there is no way to match
 * "1,2" against an AssignmentExpression, so a SyntaxError.
 *
 * In the case of "true ? 1 : 2,3" the ConditionalExpression
 * will match "true ? 1 : 2" and there will be a surrounding
 * comma expression, i.e.: "(true ? 1 : 2), 3".
 */

try {
    eval("print(true ? 1,2 : 3);");
} catch (e) {
    print(e.name);
}

try {
    eval("print(true ? 1 : 2,3);");
} catch (e) {
    print(e.name);
}

/*===
1 4
===*/

/* Here comma works as a normal argument separator. */

try {
    eval("print(true ? 1 : 2, false ? 3 : 4);");
} catch (e) {
    print(e.name);
}

/*===
C
D
F
G
===*/

/*
 *  The following:
 *
 *    A ? B ? C : D : E ? F : G
 *
 *  parses as:
 *
 *    A ? (B ? C : D) : (E ? F : G)
 */

try {
    eval("print(true ? true ? 'C' : 'D' : true ? 'F' : 'G');");
} catch (e) {
    print(e.name);
}

try {
    eval("print(true ? false ? 'C' : 'D' : true ? 'F' : 'G');");
} catch (e) {
    print(e.name);
}

try {
    eval("print(false ? true ? 'C' : 'D' : true ? 'F' : 'G');");
} catch (e) {
    print(e.name);
}

try {
    eval("print(false ? true ? 'C' : 'D' : false ? 'F' : 'G');");
} catch (e) {
    print(e.name);
}
