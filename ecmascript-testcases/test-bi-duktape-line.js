/*===
23
true on line 26
constructor on line 30
getter on line 35
123
===*/

/* Duktape.line() was removed in Duktape 0.11.0, so this test was changed to ensure
 * that a simple replacement can provide the same functionality.
 */

Duktape.line = function () {
    'use duk notail';

    /* Tail calls are prevented to ensure calling activation exists.
     * Call stack indices: -1 = Duktape.act, -2 = getCurrentLine, -3 = caller
     */

    return (Duktape.act(-3) || {}).lineNumber;
};

print(Duktape.line())

if (true) {
    print("true on line", Duktape.line())
}

function MyConstructor() {
    print("constructor on line", Duktape.line());
}
new MyConstructor();

var obj = {
    get x() { print('getter on line', Duktape.line()); return 123; }
}
print(obj.x);
