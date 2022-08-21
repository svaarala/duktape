/*===
SyntaxError
SyntaxError
===*/

/* Empty or pure white space is a SyntaxError. */

try {
    print(JSON.parse(''));
} catch (e) {
    print(e.name);
}

try {
    print(JSON.parse(' \n\r\t'));
} catch (e) {
    print(e.name);
}
