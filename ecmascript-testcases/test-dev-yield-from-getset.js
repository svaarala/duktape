/*
 *  Yielding from a getter/setter is currently not allowed and should
 *  cause a clean TypeError.
 */

/*===
TypeError
TypeError
===*/

function getter() {
    Duktape.Thread.yield('foo')
}

function setter() {
    Duktape.Thread.yield('bar')
}

var obj = {
    get a() { getter(); },
    set a() { setter(); }
}

function test_get() {
    var t = new Duktape.Thread(function() {
        print(obj.a);
    });
    Duktape.Thread.resume(t)
}

function test_set() {
    var t = new Duktape.Thread(function() {
        obj.a = 1;
        print('setter ok');
    });
    Duktape.Thread.resume(t)
}

try {
    test_get();
} catch (e) {
    print(e.name);
}

try {
    test_set();
} catch (e) {
    print(e.name);
}
