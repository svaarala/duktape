/*===
SyntaxError
===*/

try {
    eval('new (function test() { print(typeof new.foo); })');
} catch (e) {
    print(e.name);
}
