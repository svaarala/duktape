/*===
TypeError
TypeError
TypeError
boolean object [object Boolean]
boolean object [object Boolean]
number object [object Number]
string object [object String]
object object [object Array]
object object [object Object]
===*/

function basicTest() {
    function test(o, is_noarg) {
        var t;

        try {
           var t;

           if (is_noarg) {
               t = Object.prototype.valueOf.call();
           } else {
               t = Object.prototype.valueOf.call(o);
           }
           print(typeof o, typeof t, Object.prototype.toString.call(t));
        } catch (e) {
           print(e.name);
        }
    }

    test(undefined, true);
    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('foo');
    test([1,2]);
    test({ foo: 1, bar: 2 });
}

try {
    basicTest();
} catch (e) {
    print(e);
}
