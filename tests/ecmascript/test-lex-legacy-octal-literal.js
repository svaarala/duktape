/*
 *  Legacy octal literals like 0777.
 */

/*===
63
1.0429624198832568e+93
78
99
78.123
SyntaxError
function
SyntaxError
SyntaxError
===*/

// Legacy octal notation is allowed in non-strict mode.
try {
    print(eval('077'));
} catch (e) {
    print(e.name);
}

// Range is not limited.
try {
    print(eval('07777777777777777766666666666666655555555555544444444444444443333333333333322222222222222211111111111111'));
} catch (e) {
    print(e.name);
}

// If the literal starts like octal but contains 8 or 9, it gets parsed as
// decimal.
try {
    print(eval('078'));
    print(eval('099'));
} catch (e) {
    print(e.name);
}

// If the literal contains 8 or 9, it can then also have fractions.
try {
    print(eval('078.123'));
} catch (e) {
    print(e.name);
}

// However, if the literal doesn't contain 8 or 9, fractions are rejected
// (i.e. a period doesn't trigger "parse as decimal instead" behavior).
try {
    print(eval('077.123'));
} catch (e) {
    print(e.name);
}

// An octal literal can be followed by a period, indicating property access.
try {
    print(eval('typeof 077.toString'));
} catch (e) {
    print(e.name);
}

// But a decimal literal with leading zero cannot.
try {
    print(eval('typeof 078.toString'));
} catch (e) {
    print(e.name);
}

// Legacy octal must not be parsed in strict mode code; E5 Section 7.8.3.
// Also required by ES2015.
try {
    print(eval("(function() { 'use strict'; return 077; })();"));
} catch (e) {
    print(e.name);
}
