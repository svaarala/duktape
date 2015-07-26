/*
 *  Duktape 0.11.0 would allow an empty throw statement.
 */

/*===
SyntaxError
SyntaxError
SyntaxError
===*/

try {
    eval('try { throw; } catch (e) { print(e); }');
} catch (e) {
    print(e.name);
}

try {
    eval('try { throw } catch (e) { print(e); }');
} catch (e) {
    print(e.name);
}

try {
    eval('try { throw\n } catch (e) { print(e); }');
} catch (e) {
    print(e.name);
}
