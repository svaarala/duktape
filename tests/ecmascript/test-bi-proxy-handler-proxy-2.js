/*===
P1.get called foo
aiee
===*/

var curr = {
    get: function (targ, prop, recv) {
        print('P1.get called', prop);
        return 'aiee';
    }
};
for (var i = 0; i < 1e6; i++) {
    curr = new Proxy(curr, {});
}

var finalProxy = new Proxy({}, curr);
print(finalProxy.foo);
