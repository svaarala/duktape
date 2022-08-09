// Proxy target and handler may also be Proxies.

/*===
- get proxy1.PI
handler1.get: string PI
handler2.get: string PI
3.141592653589793
- set proxy1.foo
handler2.set: string foo 123
handler2.set: string foo 123
123
123
===*/

function chainedTargetTest() {
    //
    // handler1     handler2
    //    |            |
    //  proxy1  -->  proxy2 --> target

    var target = Math;
    var handler2 = {
        get: function (targ, key) {
            print('handler2.get:', typeof key, key);
            return targ[key];
        },
        set: function (targ, key, val) {
            print('handler2.set:', typeof key, key, val);
            targ[key] = val;
            return true;
        }
    };
    var proxy2 = new Proxy(target, handler2);
    var handler1 = {
        get: function (targ, key) {
            print('handler1.get:', typeof key, key);
            return targ[key];
        },
        set: function (targ, key, val) {
            print('handler2.set:', typeof key, key, val);
            targ[key] = val;
            return true;
        }
    };
    var proxy1 = new Proxy(proxy2, handler1);

    print('- get proxy1.PI');
    print(proxy1.PI);

    print('- set proxy1.foo');
    print(proxy1.foo = 123);
    print(Math.foo);
}

chainedTargetTest();
