/*===
[object Undefined]
[object Null]
[object Boolean]
[object Boolean]
[object Number]
[object String]
[object Array]
[object Object]
[object Function]
[object RegExp]
[object Date]
[object Error]
[object JSON]
[object Math]
[object Function]
proxied
[object Array]
[object Object]
[object Function]
===*/

function basicTest() {
    function test(v) {
        print(Object.prototype.toString.call(v));
    }

    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('foo');
    test([1,2]);
    test({ foo: 1, bar: 2 });
    test(function(){});
    test(/foo/);
    test(new Date());
    test(new Error('foo'));
    test(JSON);
    test(Math);
    test(function f() { var indirect = eval; return indirect('this'); });  // indirect eval -> this=global
    print('proxied');
    test(new Proxy([ 1, 2, 3 ], {}));
    //test(new Proxy(new Proxy([ 1, 2, 3 ], {}), {}));
    test(new Proxy({ foo: 'bar' }, {}));
    test(new Proxy(Math.cos, {}));
}

try {
    basicTest();
} catch (e) {
    print(e);
}
