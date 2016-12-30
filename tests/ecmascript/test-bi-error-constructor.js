/*===
true
true
true
true
true
true
true
bar
===*/

function test() {
    // Error constructor inheritance chains were changed in ES2015.
    // E.g. TypeError constructor internal prototype is the Error
    // constructor rather than Function.prototype directly.

    print(Object.getPrototypeOf(Error) === Function.prototype);
    print(Object.getPrototypeOf(EvalError) === Error);
    print(Object.getPrototypeOf(RangeError) === Error);
    print(Object.getPrototypeOf(ReferenceError) === Error);
    print(Object.getPrototypeOf(SyntaxError) === Error);
    print(Object.getPrototypeOf(TypeError) === Error);
    print(Object.getPrototypeOf(URIError) === Error);

    // Demonstrate using an actual inherited property access.
    Error.foo = 'bar';
    print(RangeError.foo);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
