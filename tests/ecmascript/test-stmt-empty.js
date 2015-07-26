/*
 *  Empty statement (E5 Section 12.3).
 */

/*===
undefined
undefined
undefined
===*/

/* Simple basic test: wherever a Statement production is allowed, an
 * EmptyStatement is allowed.
 */

function basicTest() {
    print(eval(";"));
    print(eval("if (1);else 2;"));
    print(eval("if (1);else;"));
}

try {
    basicTest();
} catch (e) {
    print(e.stack || e);
}

/*===
SyntaxError
SyntaxError
SyntaxError
SyntaxError
===*/

/*
 *  Empty statement requires a semicolon but it's also part of automatic
 *  semicolon insertion.  On the other hand E5 Section 7.9 states that:
 *
 *      However, there is an additional overriding condition on the preceding
 *      rules: a semicolon is never inserted automatically if the semicolon
 *      would then be parsed as an empty statement or if that semicolon would
 *      become one of the two semicolons in the header of a for statement
 *      (see 12.6.3).
 *
 *  As a result an automatic semicolon is never allowed for an empty statement
 *  and all cases below are SyntaxErrors.
 */

function semicolonTest1() {
    // This is a SyntaxError because there is no newline before the offending
    // EOF so automatic semicolon is not allowed.
    print(eval("if (123)"));
}

function semicolonTest2() {
    // Here an automatic semicolon would otherwise be allowed (there is a
    // newline before the offending semicolon) but the empty statement
    // special case rejects this too.
    print(eval("if (123)\n"));
}

function semicolonTest3() {
    // No newline before offending '}', SyntaxError.
    print(eval("(function () { if (123) })()"));
}

function semicolonTest4() {
    // Automatic semicolon would otherwise be allowed, but the empty statement
    // special case rejects it.
    print(eval("(function () { if (123)\n})()"));
}

[ semicolonTest1, semicolonTest2, semicolonTest3, semicolonTest4 ].forEach(function (fn) {
    try {
        fn();
    } catch (e) {
        print(e.name);
    }
});
