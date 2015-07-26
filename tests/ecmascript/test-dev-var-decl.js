/*===
undefined
undefined
123
123
===*/

/* Variable declaration is an 'empty' statement at the top level.
 * It doesn't contribute to an implicit return value.
 */

try {
    print(eval("var i;"));
} catch (e) {
    print(e.name);
}

try {
    print(eval("var i = 1;"));
} catch (e) {
    print(e.name);
}

try {
    print(eval("123; var i;"));
} catch (e) {
    print(e.name);
}

try {
    print(eval("123; var i = 1;"));
} catch (e) {
    print(e.name);
}

/*===
SyntaxError
works
SyntaxError
===*/

/* Variable name must be an Identifier, which exclused all
 * reserved words.  The set of reserved words is different
 * in strict mode.
 */

try {
    eval("var for = 1;");
} catch (e) {
    print(e.name);
}

try {
    /* 'implements' is not a reserved word in non-strict mode */
    eval("var implements = 'works'; print(implements);");
} catch (e) {
    print(e.name);
}

try {
    /* 'implements' IS a reserved word in strict mode */
    eval("'use strict'; var implements = 'works'; print(implements);");
} catch (e) {
    print(e.name);
}

/*===
works
works
SyntaxError
SyntaxError
===*/

/* Strict mode also prohibits use of 'eval' or 'arguments' as
 * variable name, even though they are not reserved words.
 *
 * These tests are made within a function, because attempts to
 * re-declare eval/arguments will result in a TypeError.
 */

try {
    eval("function f1() { var eval = 'works'; print(eval); }; f1();");
} catch (e) {
    print(e.name);
}

try {
    eval("function f2() { var arguments = 'works'; print(arguments); }; f2();");
} catch (e) {
    print(e.name);
}

try {
    eval("function f3() { 'use strict'; var eval = 'works'; print(eval); }; f3();");
} catch (e) {
    print(e.name);
}

try {
    eval("function f4() { 'use strict'; var arguments = 'works'; print(arguments); }; f4();");
} catch (e) {
    print(e.name);
}
