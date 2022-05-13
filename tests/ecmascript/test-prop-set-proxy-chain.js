/*===
undefined
123
123
undefined
123
123
undefined
H2.getOwnPropertyDescriptor: string foo
H1.getOwnPropertyDescriptor: string foo
H1.getOwnPropertyDescriptor: string foo
H2.defineProperty: string foo object {"value":123,"writable":true,"enumerable":true,"configurable":true}
H1.defineProperty: string foo object {"value":123,"writable":true,"enumerable":true,"configurable":true}
H1.getOwnPropertyDescriptor: string foo
123
123
undefined
H2.getOwnPropertyDescriptor: string 2001
H1.getOwnPropertyDescriptor: string 2001
H1.getOwnPropertyDescriptor: string 2001
H2.defineProperty: string 2001 object {"value":123,"writable":true,"enumerable":true,"configurable":true}
H1.defineProperty: string 2001 object {"value":123,"writable":true,"enumerable":true,"configurable":true}
H1.getOwnPropertyDescriptor: string 2001
123
123
undefined
H2.set: string bar 123
H1.set: string bar 123
123
123
undefined
H2.set: string 3001 123
H1.set: string 3001 123
123
123
===*/

try {
    var P1, P2, H1, H2, target;

    // With no 'set' trap, the [[Set]] propagates to target, and the final
    // [[Set]] handling is applied to the receiver, i.e. P2 here.  The final
    // [[Set]] handling uses [[DefineProperty]] which then again propagates
    // through the Proxy chain to the target.
    target = {};
    P1 = new Proxy(target, {});
    P2 = new Proxy(P1, {});
    print(target.foo);
    print(P2.foo = 123);
    print(target.foo);
    print(target[2001]);
    print(P2[2001] = 123);
    print(target[2001]);

    // Here no 'set' traps, but 'defineProperty' traps make the final set
    // handling visible.
    target = {};
    H1 = {
        defineProperty: function (targ, key, desc) {
            print('H1.defineProperty:', typeof key, key, typeof desc, JSON.stringify(desc));
            Object.defineProperty(targ, key, desc);
            return true;
        },
        getOwnPropertyDescriptor: function (targ, key) {
            print('H1.getOwnPropertyDescriptor:', typeof key, key);
            return Object.getOwnPropertyDescriptor(targ, key);
        }
    };
    H2 = {
        defineProperty: function (targ, key, desc) {
            print('H2.defineProperty:', typeof key, key, typeof desc, JSON.stringify(desc));
            Object.defineProperty(targ, key, desc);
            return true;
        },
        getOwnPropertyDescriptor: function (targ, key) {
            print('H2.getOwnPropertyDescriptor:', typeof key, key);
            return Object.getOwnPropertyDescriptor(targ, key);
        }
    };
    P1 = new Proxy(target, H1);
    P2 = new Proxy(P1, H2);
    print(target.foo);
    print(P2.foo = 123);
    print(target.foo);
    print(target[2001]);
    print(P2[2001] = 123);
    print(target[2001]);

    target = {};
    H1 = {
        set: function (targ, key, val) {
            print('H1.set:', typeof key, key, val);
            targ[key] = val;
            return true;
        }
    };
    H2 = {
        set: function (targ, key, val) {
            print('H2.set:', typeof key, key, val);
            targ[key] = val;
            return true;
        }
    };
    P1 = new Proxy(target, H1);
    P2 = new Proxy(P1, H2);
    print(target.bar);
    print(P2.bar = 123);
    print(target.bar);
    print(target[3001]);
    print(P2[3001] = 123);
    print(target[3001]);
} catch (e) {
    print(e.stack || e);
}
