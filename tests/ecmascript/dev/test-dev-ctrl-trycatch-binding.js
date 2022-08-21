/*===
foo
bar
foo
===*/

try {
    throw 'foo';
} catch (e) {
    print(e);

    try {
        throw 'bar';
    } catch (e) {
        /* new, shadowing lex env here */
        print(e);
        /* old lex env restored before inner try completes */
    }

    print(e);
}
