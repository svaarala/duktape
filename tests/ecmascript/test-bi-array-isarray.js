/*===
boolean false
boolean false
boolean false
boolean false
boolean false
boolean false
boolean true
boolean false
boolean false
===*/

function basicTest() {
    function test(x) {
        var t = Array.isArray(x);
        print(typeof t, t);
    }

    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('foo');
    test([1,2]);
    test({ foo: 1, bar:2 });
    test(function(){});
}

try {
    basicTest();
} catch (e) {
    print(e);
}
