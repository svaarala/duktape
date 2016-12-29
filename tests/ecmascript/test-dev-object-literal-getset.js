/*
 *  In ES2015 object literal get/set properties have a few special requirements:
 *  they are not constructable and they have no .prototype.
 */

/*===
true myFunc
false undefined
GET
object globalName
123
TypeError
true fooBar
false undefined
SET
object globalName
setter called
undefined
TypeError
===*/

(new Function('return this'))().myName = 'globalName';  // ensure set to global even with 'nodejs' command line tool

function test() {
    var myName = 'localName';

    var tmp = { get myFunc() { print('GET'); print(typeof this, this.myName); return 123; },
                set fooBar(v) { print('SET'); print(typeof this, this.myName); print('setter called') },
                myName: 'objectName' };
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
