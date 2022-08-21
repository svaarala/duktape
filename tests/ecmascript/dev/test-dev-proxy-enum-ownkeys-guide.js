/*
 *  Proxy enum/ownKeys example in guide.
 */

/*===
ownKeys trap
foo
bar
ownKeys trap
foo,bar
ownKeys trap
foo,bar
===*/

var target = {
    foo: 1,
    bar: 2,
    _quux: 3,
    _baz: 4
};

var proxy = new Proxy(target, {
    // Obsoleted trap.
    enumerate: function (targ) {
        print('enumerate trap');
        return Object.getOwnPropertyNames(targ)
                     .filter(function (v) { return v[0] !== '_'; });
    },

    ownKeys: function (targ) {
        // this binding: handler table
        // targ: underlying plain object (= target, above)

        print('ownKeys trap');
        return Object.getOwnPropertyNames(targ)
                     .filter(function (v) { return v[0] !== '_'; });
    }
});

function test() {
    for (var k in proxy) {
        print(k);  // prints 'foo' and 'bar'
    }
}
test();

print(Object.keys(proxy));                 // prints 'foo,bar'
print(Object.getOwnPropertyNames(proxy));  // prints 'foo,bar'
