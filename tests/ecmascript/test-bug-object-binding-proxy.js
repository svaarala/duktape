/*
 *  Object binding (global object or with statement) can be a Proxy object
 *  but Duktape 1.2.x doesn't handle that correctly.  This test demonstrates
 *  the bug for with-statement, C code is needed to demonstrate the global
 *  object issue.  See: https://github.com/svaarala/duktape/issues/221.
 */

/*===
in with (obj) statement
123
bar
has print
in with (proxy) statement
has print
has dummyGlobal
123
has print
has foo
get foo
bar
===*/

var dummyGlobal = 123;

function test() {
    var obj = { foo: 'bar' };
    var proxy = new Proxy(obj, {
        get: function get(targ, key, recv) {
            print('get ' + key);
            return targ[key];
        },
        has: function has(targ, key) {
            print('has ' + key);
            return key in targ;
        }
    });

    with (obj) {
        print('in with (obj) statement');
        print(dummyGlobal);
        print(foo);
    }

    with (proxy) {
        print('in with (proxy) statement');
        print(dummyGlobal);
        print(foo);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
