/*===
[object Undefined]
[object Null]
[object Boolean]
[object Boolean]
[object Number]
[object String]
[object Array] proxied: [object Array]
[object Object] proxied: [object Object]
[object Function] proxied: [object Function]
[object RegExp] proxied: [object Object]
[object Date] proxied: [object Object]
[object Error] proxied: [object Object]
[object JSON] proxied: [object JSON]
[object Math] proxied: [object Math]
[object Function] proxied: [object Function]
===*/

function basicTest() {
    function test(v) {
        if ((typeof v === 'object' || typeof v === 'function') && v !== null) {
            print(Object.prototype.toString.call(v), 'proxied:', Object.prototype.toString.call(new Proxy(v, {})));
        } else {
            print(Object.prototype.toString.call(v));
        }
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
}

basicTest();
