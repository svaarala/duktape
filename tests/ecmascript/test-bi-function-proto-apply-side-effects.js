/*===
get 0
get 1
get 2
103
TypeError
===*/

function test() {
    var notCallable = 123;
    var args = {
        length: 3
    };
    Object.defineProperty(args, '0', {
        get: function () { print('get 0'); return 109; }
    });
    Object.defineProperty(args, '1', {
        get: function () { print('get 1'); return 103; }
    });
    Object.defineProperty(args, '2', {
        get: function () { print('get 2'); return 107; }
    });
    Object.defineProperty(args, '3', {
        get: function () { print('get 3'); return -100; }  // not read because .length == 3
    });

    try {
        // Target is callable, args are read and trigger side effects.
        print(Function.prototype.apply.call(Math.min, null, args));

        // Target is not callable, args are not read and there should be
        // no side effects.
        print(Function.prototype.apply.call(notCallable, null, args));
    } catch (e) {
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
