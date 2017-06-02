/*
 *  Yielding from a Proxy 'construct' trap.
 */

/*===
MyConstructor yielding
1001
MyConstructor resumed
myThread yielding result 1
234
trap yielding
2001
trap resumed
MyConstructor yielding
1001
MyConstructor resumed
myThread yielding result 2
234
myThread done
thread-done
===*/

function MyConstructor() {
    print('MyConstructor yielding');
    Duktape.Thread.yield(1001);
    print('MyConstructor resumed');
    return { foo: 234 };
}

function myThread() {
    var proxy = new Proxy(MyConstructor, {});
    var x = new proxy();
    print('myThread yielding result 1');
    Duktape.Thread.yield(x.foo);

    proxy = new Proxy(MyConstructor, {
        construct: function (targ, argArray, newTarget) {
            print('trap yielding');
            Duktape.Thread.yield(2001);
            print('trap resumed');
            return new MyConstructor();
        }
    });
    x = new proxy();
    print('myThread yielding result 2');
    Duktape.Thread.yield(x.foo);

    print('myThread done');
    return 'thread-done';
}

function test() {
    var t = new Duktape.Thread(myThread);
    print(Duktape.Thread.resume(t, 3001));
    print(Duktape.Thread.resume(t, 3002));
    print(Duktape.Thread.resume(t, 3003));
    print(Duktape.Thread.resume(t, 3004));
    print(Duktape.Thread.resume(t, 3005));
    print(Duktape.Thread.resume(t, 3006));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
