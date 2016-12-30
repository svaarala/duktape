/*
 *  ES2015 method definition in object literal.
 */

/*===
{}
5
{}
5
{"dummy":1}
5
{"dummy":2}
5
{"dummy":2}
6
{"myName":"objectName"}
this: object objectName
5
foo() sees foo as: undefined
function
string foo false false true
number 2 false false true
undefined
TypeError
object
undefined
["123","3735928559","foo","bar","234.123"]
123
234
345
456
567
===*/

(new Function('return this'))().myName = 'globalName';  // ensure set to global even with 'nodejs' command line tool

function test() {
    var myName = 'localName';
    var bar = 234;
    var obj;

    // Some basic positions in the literal.

    obj = {
        foo(a,b) { return a+b; }
    };
    print(JSON.stringify(obj));
    print(obj.foo(2,3));

    obj = {
        foo(a,b) { return a+b; },
    };
    print(JSON.stringify(obj));
    print(obj.foo(2,3));

    obj = {
        dummy: 1,
        foo(a,b) { return a+b; }
    };
    print(JSON.stringify(obj));
    print(obj.foo(2,3));

    obj = {
        dummy: 1,
        foo(a,b) { return a+b; },
        dummy: 2
    };
    print(JSON.stringify(obj));
    print(obj.foo(2,3));

    obj = {
        dummy: 1,
        foo(a,b) { return a+b; },
        dummy: 2,
        foo(a,b) { return a*b; }
    };
    print(JSON.stringify(obj));
    print(obj.foo(2,3));

    // The 'this' binding seen is handled normally; here global object.
    obj = {
        foo(a,b) { print('this:', typeof this, this.myName); return a+b; },
        myName: 'objectName'
    };
    print(JSON.stringify(obj));
    print(obj.foo(2,3));

    // Method definitions have a .name property (ES2016) but they don't get a
    // name binding, i.e. they don't see their own name when called.  They
    // also don't get a .prototype object and are not constructable.

    obj = {
        foo(a,b) { print('foo() sees foo as:', typeof foo); }
    };
    obj.foo();
    print(typeof obj.foo);
    pd = Object.getOwnPropertyDescriptor(obj.foo, 'name');
    print(typeof pd.value, pd.value, pd.writable, pd.enumerable, pd.configurable);
    pd = Object.getOwnPropertyDescriptor(obj.foo, 'length');
    print(typeof pd.value, pd.value, pd.writable, pd.enumerable, pd.configurable);
    pd = Object.getOwnPropertyDescriptor(obj.foo, 'prototype');
    print(typeof pd);
    try {
        new obj.foo();
    } catch (e) {
        print(e.name);
    }

    // Strictness is inherited; the caller here is non-strict so the result
    // function is also non-strict.
    obj = {
        foo(a,b) { print(typeof this); }
    };
    obj.foo.call(void 0);  // non-strict -> promotes to global

    // Here the caller is strict.
    obj = eval('"use strict"; ({ foo(a,b) { print(typeof this); } });');
    obj.foo.call(void 0);  // strict -> keep as is

    // Method name may be a string, an identifier, or a number.
    obj = {
        foo(a,b) { return 123; },
        "bar"(a,b) { return 234; },
        123(a,b) { return 345; },
        234.123(a,b) { return 456; },
        0xdeadbeef(a,b) { return 567; }
    };
    print(JSON.stringify(Object.keys(obj)));
    print(obj.foo());
    print(obj.bar());
    print(obj['123']());
    print(obj['234.123']());
    print(obj[0xdeadbeef]());
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
