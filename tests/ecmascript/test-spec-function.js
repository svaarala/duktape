/*
 *  Function compilation and execution (E5 Sections 13, 10.4.3).
 *
 *  There is a separate test for tail recursion, arguments object.
 */

/*** DUPLICATE ARGUMENT NAMES ***/

/*===
3 2
SyntaxError
===*/

/* Non-strict mode: last value to match a certain name remains bound,
 * see E5 Section 10.5.
 */
function nonstrict_dup(a,b,a) { print(a,b); }
nonstrict_dup(1,2,3);

/* Strict mode: compile-time error to declare a function with duplicate
 * argument names, see E5 Section 13.1.
 */
try {
    eval("'use strict';\n" +
         "function strict_dup(a,b,a) { print(a,b); }\n" +
         "strict_dup(1,2,3);");
} catch(e) {
    print(e.name);
}


/*** INVALID ARGUMENT NAMES (NON-STRICT MODE) ***/

/*===
1 2
SyntaxError
SyntaxError
1
===*/

/* Non-strict mode: 'eval' and 'arguments' are valid argument names,
 * because they are not reserved words.  All non-strict mode reserved
 * words are forbidden, because they do not match the 'Identifier'
 * production.
 *
 * XXX: test else (a Keyword), class (a FutureReservedWord), and an
 * additional keyword only illegal in strict mode.  Should really
 * test all reserved words here.
 */

function nonstrict_evalargs(eval, arguments) { print(eval, arguments); }
nonstrict_evalargs(1,2);

try {
    /* NB: do not 'print(else)' because that would be a separate,
     * different error.
     */
    eval("function nonstrict_else(else) { print('else'); }\n" +
         "nonstrict_else(1);");
} catch (e) {
    print(e.name);
}

try {
    eval("function nonstrict_class(class) { print('class'); }\n" +
         "nonstrict_class(1);");
} catch (e) {
    print(e.name);
}

function nonstrict_package(package) { print(package); }
nonstrict_package(1);


/*** INVALID ARGUMENT NAMES (STRICT MODE) ***/

/*===
SyntaxError
SyntaxError
SyntaxError
SyntaxError
SyntaxError
===*/

/* Strict mode: 'eval' and 'arguments' must cause a compile-time error.
 * See E5 Section 13.1.
 */

try {
    eval("'use strict'\n" +
         "function strict_eval(eval) { print(eval); }\n" +
         "strict_eval(1);");
} catch(e) {
    print(e.name);
}

try {
    eval("'use strict'\n" +
         "function strict_args(arguments) { print(arguments); }\n" +
         "strict_args(1);");
} catch(e) {
    print(e.name);
}

try {
    /* NB: do not 'print(else)' because that would be a separate,
     * different error.
     */
    eval("'use strict'\n" +
         "function strict_else(else) { print('else'); }\n" +
         "strict_else(1);");
} catch (e) {
    print(e.name);
}

try {
    eval("'use strict'\n" +
         "function strict_class(class) { print('class'); }\n" +
         "strict_class(1);");
} catch (e) {
    print(e.name);
}

try {
    eval("'use strict'\n" +
         "function strict_package(package) { print('package'); }\n" +
         "strict_package(1);");
} catch (e) {
    print(e.name);
}

/* XXX: add test for shadowing */
