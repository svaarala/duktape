/*===
P1.get called true get true
get called true foo true
<<<foo>>>
P1.get called true get true
get called true bar true
<<<bar>>>
===*/

var T1 = {};
var P1 = new Proxy(T1, {
    get: function (targ, prop, recv) {
        print('P1.get called', targ === T1, prop, recv === P1);

        if (prop === 'get') {
            return function (targ, prop, recv) {
                print('get called', targ === T2, prop, recv === P2);
                return '<<<' + prop + '>>>'
            };
        }
    }
});

var T2 = {};
var P2 = new Proxy(T2, P1);

print(P2.foo);
print(P2.bar);
