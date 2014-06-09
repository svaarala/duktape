/*
 *  http://webreflection.blogspot.fi/2014/05/fixing-java-nashorn-proto.html
 */

/*===
true
false
true
[object Object]
false
true
===*/

function test() {
    var obj;

    obj = {};
    print('__proto__' in obj);
    print(Object.getOwnPropertyNames(obj).indexOf('__proto__') >= 0);  // not own property
    print(Object.getOwnPropertyNames(Object.prototype).indexOf('__proto__') >= 0);  // inherited accessor
    print(Object.getOwnPropertyDescriptor(Object.prototype, '__proto__'));

    obj = Object.create(null);
    obj.__proto__ = [];  // no magic __proto__, becomes own property
    print(obj instanceof Array);  // no, not inherited
    print(Object.getOwnPropertyNames(Object.prototype).indexOf('__proto__') >= 0);  // own property

    // XXX: no __proto__ literals (yet)
}

try {
    test();
} catch (e) {
    print(e);
}
