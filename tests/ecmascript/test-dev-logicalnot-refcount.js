/*---
{
    "custom": true
}
---*/

/*===
2
3
2
false
===*/

/* Development time test to test refcount handling of bytecode executor
 * logical NOT.
 */

var dummy = {};
var z = 1;

function test() {
    var tmp;

    print(Duktape.info(dummy)[2]);
    tmp = dummy;
    print(Duktape.info(dummy)[2]);
    tmp = !z;
    print(Duktape.info(dummy)[2]);
    print(tmp);
}

try {
    test();
} catch (e) {
    print(e);
}
