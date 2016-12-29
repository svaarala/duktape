/*
 *  ES2015 IdentifierName in object literal.
 */

/*===
{"foo":123,"Math":{},"bar":234}
{"foo":123,"Math":987,"bar":234}
===*/

function test() {
    var bar = 234;
    var obj;

    obj = {
        foo: 123,
        Math,
        bar,
        Math
    };
    print(JSON.stringify(obj));

    obj = {
        foo: 123,
        Math,
        bar,
        Math: 987
    };
    print(JSON.stringify(obj));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
