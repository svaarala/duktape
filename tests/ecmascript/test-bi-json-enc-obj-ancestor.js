/*===
{"foo":1,"bar":2}
{"foo":1,"protoProp":1}
{"foo":{"myProp":"myVal"},"bar":{"myProp":"myVal"},"myProp":"myVal"}
===*/

function ancestorTest1() {
    function F() {
    }
    F.prototype = { protoProp: 1 };

    var obj = new F();
    obj.foo = 1;
    obj.bar = 2;

    // only 'foo' and 'bar' are serialized, because JSON.stringify() only
    // serializes 'own properties', not ancestors; E5.1 Section 15.12.3,
    // JO(), step 6.a.
    print(JSON.stringify(obj));

    // However, when explicit property names are given, also ancestor
    // properties can be serialized (Str() step 1 uses [[Get]])
    print(JSON.stringify(obj, [ 'foo', 'protoProp' ]));

    // Try the same through Object.prototype
    Object.prototype.myProp = 'myVal';
    print(JSON.stringify({ foo: {}, bar: {} }, ['foo', 'bar', 'myProp']));
}

try {
    ancestorTest1();
} catch (e) {
    print(e.name);
}
