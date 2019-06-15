/*---
{
    "skip": true
}
---*/

/*===
get: toJSON
ownKeys
get: foo
get: bar
{"foo":"FOO","bar":"BAR"}
done
===*/

function test() {
    var O = { foo: 1, bar: 2 };  // no 'quux' in target
    var P = new Proxy(O, {
        ownKeys: function (targ) {
            print('ownKeys');
            return [ 'foo', 'bar', 'quux' ];
        },
        get: function (targ, key, recv) {
            print('get:', key);
            if (key === 'foo') {
                return 'FOO';
            } else if (key === 'bar') {
                return 'BAR';
            } else if (key === 'quux') {
                return 'QUUX';
            } else {
                return void 0;
            }
        }
    });

    var res = JSON.stringify(P);
    print(res);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}

print('done');
