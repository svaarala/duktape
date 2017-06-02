/*
 *  Yielding from a Proxy 'apply' trap.
 */

/*===
myFunction yielding
1001
myFunction resumed
myThread yielding result 1
234
trap yielding
2001
trap resumed
myFunction yielding
1001
myFunction resumed
myThread yielding result 2
234
myThread done
thread-done
===*/

function myFunction() {
    print('myFunction yielding');
    Duktape.Thread.yield(1001);
    print('myFunction resumed');
    return 234;
}

function myThread() {
    var proxy = new Proxy(myFunction, {});
    var x = proxy();
    print('myThread yielding result 1');
    Duktape.Thread.yield(x);

    proxy = new Proxy(myFunction, {
        apply: function (targ, argArray, newTarget) {
            print('trap yielding');
            Duktape.Thread.yield(2001);
            print('trap resumed');
            return myFunction();
        }
    });
    x = proxy();
    print('myThread yielding result 2');
    Duktape.Thread.yield(x);

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
