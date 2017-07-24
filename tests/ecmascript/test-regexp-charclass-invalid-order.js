/*===
/[a-z]/
SyntaxError
===*/

function test() {
    try {
        print(eval('/[a-z]/'));
        print(eval('/[z-a]/'));
    } catch (e) {
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
