/*===
1
1
SyntaxError
===*/

/* Allow leading and trailing whitespace, but no other garbage */

try {
    print(JSON.parse('1'));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse('\t\n  1  \t\n'));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse('1    x'));
} catch (e) {
    print(e.name);
}
