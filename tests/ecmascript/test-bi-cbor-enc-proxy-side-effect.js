/*===
ownKeys
get: foo
get: bar
a263666f6f63464f4f6362617263424152
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

    var res = CBOR.encode(P);
    print(Duktape.enc('hex', res));
    print(JSON.stringify(CBOR.decode(res)));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}

print('done');
