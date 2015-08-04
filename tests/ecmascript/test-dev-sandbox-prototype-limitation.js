/*
 *  There is currently (Duktape 1.0) no way to prevent user code from
 *  accessing built-in prototype objects through plain values.  For
 *  example, a plain string implicitly inherits from the original
 *  String.prototype.
 */

/*===
FOO
false
undefined
undefined
===*/

function test() {
    var proto;

    // Implicit inheritance example
    print("foo".toUpperCase());

    // This is not possible because String.prototype is not configurable
    proto = {};
    String.prototype = proto;
    print(String.prototype === proto);

    // Even if a sandboxing environment were to provide an empty String.prototype
    // or not even expose 'String' at all, "foo".toUpperCase would still resolve
    // to the built-in function.  This is not demonstrated here.

    // This is sufficient to prevent access to toUpperCase.
    delete String.prototype.toUpperCase;
    print(typeof String.prototype.toUpperCase);
    print(typeof "foo".toUpperCase);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
