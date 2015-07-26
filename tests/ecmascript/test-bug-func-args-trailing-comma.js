/*===
SyntaxError
===*/

try {
    eval("print(1,2,)");
} catch (e) {
    print(e.name);
}
