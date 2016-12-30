/*
 *  In ES2016 the 'enumerate' trap is obsolete and is not called in any situation.
 *  For-in enumeration uses the 'ownKeys' trap.
 *
 *  for-in uses EnumerateObjectProperties:
 *      - http://www.ecma-international.org/ecma-262/7.0/#sec-runtime-semantics-forin-div-ofheadevaluation-tdznames-expr-iterationkind
 *
 *  EnumerateObjectProperties is pretty vague on how the keys are obtained:
 *      - http://www.ecma-international.org/ecma-262/7.0/#sec-enumerate-object-properties
 *      - "EnumerateObjectProperties must obtain the own property keys of the
 *        target object by calling its [[OwnPropertyKeys]] internal method."
 *
 *  Proxy [[OwnPropertyKeys]] calls the 'ownKeys' rather than 'enumerate' trap:
 *      - http://www.ecma-international.org/ecma-262/7.0/#sec-proxy-object-internal-methods-and-internal-slots-ownpropertykeys
 *
 *  There's no reference to a "enumerate" trap anywhere in ES2016 specification.
 *  MDN: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Proxy/handler/enumerate.
 */

/*===
ownKeys trap
string bar
===*/

function test() {
    var P;
    var P = new Proxy({ foo: 123, bar: 234 }, {
        enumerate: function () {
            print('enumerate trap'); return [ 'foo' ];
        },
        ownKeys: function () {
            print('ownKeys trap'); return [ 'bar' ];
        }
    });
    for (k in P) {
        print(typeof k, String(k));
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
