/*
 *  Hexadecimal literals like 0xf00
 */

/*===
3735928559
7.236083299654042e+134
SyntaxError
SyntaxError
SyntaxError
function
3735928559
===*/

try {
    print(eval('0xdeadBEEF'));
} catch (e) {
    print(e.name);
}

// Range is not limited.
try {
    print(eval('0xfedcba9876543210FEDCBA9876543210fedcba9876543210fedcba9876543210fedcba9876543210fedcba9876543210fedcba9876543210'));
} catch (e) {
    print(e.name);
}

// Anything beyond [a-fA-F] and decimal digits are rejected.
try {
    print(eval('0xf00g123'));
} catch (e) {
    print(e.name);
}
try {
    print(eval('0xf00g'));
} catch (e) {
    print(e.name);
}

// Fractions are not allowed, but the period gets parsed as a property access
// separator.  However, '123' is not allowed as a property access syntax sugar
// property name so SyntaxError.
try {
    print(eval('0xdeadbeef.123'));
} catch (e) {
    print(e.name);
}

// ... but here, 'function'.
try {
    print(eval('typeof 0xdeadbeef.toString'));
} catch (e) {
    print(e.name);
}

// Also allowed in strict mode.
try {
    print(eval("(function() { 'use strict'; return 0xdeadbeef; })();"));
} catch (e) {
    print(e.name);
}
