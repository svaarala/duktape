/*
 *  Shebang comments.
 */

/*===
SyntaxError
SyntaxError
SyntaxError
SyntaxError
SyntaxError
SyntaxError
===*/

function test() {
    // Not allowed in eval(), even if option is enabled.
    try {
        print(eval('#!/dummy\n1+2'));
    } catch (e) {
        print(e.name);
    }
    try {
        print(eval(' #!/dummy\n1+2'));
    } catch (e) {
        print(e.name);
    }
    try {
        print(eval('\n#!/dummy\n1+2'));
    } catch (e) {
        print(e.name);
    }

    // Not allowed in new Function(), even if option is enabled.
    try {
        print(new Function('#!/dummy\nreturn 1+2;')());
    } catch (e) {
        print(e.name);
    }
    try {
        print(new Function(' #!/dummy\nreturn 1+2;')());
    } catch (e) {
        print(e.name);
    }
    try {
        print(new Function('\n#!/dummy\nreturn 1+2;')());
    } catch (e) {
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
