/*
 *  In ES6 object literal get/set properties have a few special requirements:
 *  they are not constructable and they have no .prototype.
 */

/*===
true myFunc
false undefined
123
TypeError
true fooBar
false undefined
setter called
undefined
TypeError
===*/

function test() {
    var tmp = { get myFunc() { return 123; },
                set fooBar(v) { print('setter called') } };
    var fun;

    fun = Object.getOwnPropertyDescriptor(tmp, 'myFunc').get;
    print(fun.hasOwnProperty('name'), fun.name);
    print(fun.hasOwnProperty('prototype'), fun.prototype);
    print(fun());
    try {
        print(new fun());
        print('never here');
    } catch (e) {
        print(e.name);
    }

    fun = Object.getOwnPropertyDescriptor(tmp, 'fooBar').set;
    print(fun.hasOwnProperty('name'), fun.name);
    print(fun.hasOwnProperty('prototype'), fun.prototype);
    print(fun('dummy'));
    try {
        print(new fun());
        print('never here');
    } catch (e) {
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
