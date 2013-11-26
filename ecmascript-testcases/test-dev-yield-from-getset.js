/*
 *  Yielding from a getter/setter is currently not allowed and should
 *  cause a clean TypeError.
 */

/*===
TypeError
TypeError
===*/

function getter() {
    __duk__.Thread.yield('foo')
}

function setter() {
    __duk__.Thread.yield('bar')
}

var obj = {
    get a() { getter(); },
    set a() { setter(); }
}

function test_get() {
    var t = new __duk__.Thread(function() {
        print(obj.a);
    });
    __duk__.Thread.resume(t)
}

function test_set() {
    var t = new __duk__.Thread(function() {
        obj.a = 1;
        print('setter ok');
    });
    __duk__.Thread.resume(t)
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

