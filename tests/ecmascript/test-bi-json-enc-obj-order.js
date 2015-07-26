/*===
true
false
===*/

/* Unless a PropertyList is given, object properties must match enumeraiton
 * order.
 */

function getEnumKeys(x) {
    var res = [];
    var k;

    for (k in x) {
        res.push(k);
    }

    return res;
}

function testObj(x) {
    var replKeys = [];

    JSON.stringify(x, function repl(k,v) {
        if (k !== '') {
            replKeys.push(k);
        }
        return v;
    });

    //print(getEnumKeys(x));
    //print(replKeys);

    print(getEnumKeys(x).join(' ') === replKeys.join(' '));
}

function testOrder1() {
    testObj({ foo: 1, bar: 2, quux: 3, baz: 4 });
}

function testOrder2() {
    function F() {
    }
    F.prototype = { protoProp: 1 };

    var obj = new F();
    obj.foo = 1;
    obj.bar = 2;

    // This results in false, because JSON.stringify() only serializes
    // 'own properties', not ancestors
    testObj(obj);
}

try {
    testOrder1();
    testOrder2();
} catch (e) {
    print(e.name);
}
