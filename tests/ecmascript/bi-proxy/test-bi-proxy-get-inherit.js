/*===
Proxy get called
this: object [object Object]
targ: object [object Object]
key: string 123
recv: object [object Object] undefined
recv is O: false
dummy
Proxy get called
this: object [object Object]
targ: object [object Object]
key: string foo
recv: object [object Object] undefined
recv is O: false
dummy
set O.myName
Proxy get called
this: object [object Object]
targ: object [object Object]
key: string 123
recv: object [object Object] O
recv is O: true
dummy
Proxy get called
this: object [object Object]
targ: object [object Object]
key: string foo
recv: object [object Object] O
recv is O: true
dummy
===*/

function test() {
    var P = new Proxy({}, {
        get: function (targ, key, recv) {
            if (key === '123' || key === 'foo') {
                print('Proxy get called');
                print('this:', typeof this, this);
                print('targ:', typeof targ, targ);
                print('key:', typeof key, key);
                print('recv:', typeof recv, recv, recv.myName);
                print('recv is O:', recv === O);
                return 'dummy';
            } else {
                return targ[key];
            }
        }
    });

    print(P[123]);
    print(P.foo);

    var O = Object.create(P);
    print('set O.myName');
    O.myName = 'O';
    print(O[123]);
    print(O.foo);
}

test();
