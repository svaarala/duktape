/*===
63
SyntaxError
===*/

try {
    print(eval("077"));
} catch (e) {
    print(e.name);
}

try {
    // Octal must not be parsed in strict mode code; E5 Section 7.8.3.
    print(eval("(function() { 'use strict'; return 077; })();"));
} catch (e) {
    print(e.name);
}
