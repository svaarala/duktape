/*===
set
object {"name":"handler"}
object {"name":"target"}
string "foo"
number 123
object {"name":"target"}
123
set
object {"name":"handler"}
object {"name":"target"}
string "foo"
number 123
object {"name":"child"}
123
set
object {"name":"handler"}
object {"name":"target"}
string "10"
number 123
object {"name":"target"}
123
set
object {"name":"handler"}
object {"name":"target"}
string "10"
number 123
object {"name":"child"}
123
===*/

function test() {
    var P = new Proxy({ name: 'target' }, {
        set: function (a,b,c,d) {
            'use strict';
            print('set');
            print(typeof this, JSON.stringify(this));
            print(typeof a, JSON.stringify(a));
            print(typeof b, JSON.stringify(b));
            print(typeof c, JSON.stringify(c));
            print(typeof d, JSON.stringify(d));
        },
        name: 'handler'
    });
    var O = Object.create(P, { name: { value: 'child', enumerable: true } });
    print(P.foo = 123);
    print(O.foo = 123);
    print(P[10] = 123);
    print(O[10] = 123);
}

test();
