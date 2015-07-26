/*
 *  Some tests on removing a "magic binding" from the arguments object
 *  parameter map.
 */

/*===
delete binding
1 2
10 2
10 2
write protect binding
1 2
10 2
10 2
change binding into accessor
1 2
10 2
setter 20
10 2
===*/

function f1(x,y) {
    print('delete binding');
    print(x,y);           // -> "1 2"

    arguments[0] = 10;    // magically bound to 'x'
    print(x,y);           // -> "10 2"

    delete arguments[0];  // magic binding is lost (removed from parameter map)
    arguments[0] = 20;    // reintroduced but no magic binding
    print(x,y);           // -> "10 2"
}

function f2(x,y) {
    print('write protect binding');
    print(x,y);

    arguments[0] = 10;
    print(x,y);

    Object.defineProperty(arguments, "0", { writable: false });
    arguments[0] = 20;
    print(x,y);
}

function f3(x,y) {
    print('change binding into accessor');
    print(x,y);

    arguments[0] = 10;
    print(x,y);

    Object.defineProperty(arguments, "0", {
        get: function() { return 'getter'; },
        set: function(v) { print('setter', v); },
        enumerable: true,
        configurable: true
    });
    arguments[0] = 20;
    print(x,y);
}

try {
    f1(1,2);
    f2(1,2);
    f3(1,2);
} catch (e) {
    print(e);
}
