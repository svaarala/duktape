/*===
myFunc called
myRetval
P2.apply called false true ["foo",123,"bar"]
p2Retval
done
===*/

// Basic case: no traps, just proceed to target.
var target = function myFunc() {
    print('myFunc called');
    return 'myRetval';
};

var P1 = new Proxy(target, {
});

var P2 = new Proxy(P1, {
});

var P3 = new Proxy(P2, {
});

print(P3());

// Trap in the middle of a Proxy chain.
var target = function myFunc() {
    print('myFunc called');
    return 'myRetval';
};

var P1 = new Proxy(target, {
});

var P2 = new Proxy(P1, {
    apply: function (targ, argThis, argList) {
        print('P2.apply called', targ == P3, argThis == void 0, JSON.stringify(argList));
        return 'p2Retval';
    }
});

var P3 = new Proxy(P2, {
});

print(P3('foo', 123, 'bar'));
print('done');
