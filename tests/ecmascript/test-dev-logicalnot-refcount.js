/*@include util-object.js@*/

/*---
{
    "custom": true
}
---*/

/*===
3
4
3
false
===*/

/* Development time test to test refcount handling of bytecode executor
 * logical NOT.
 */

var dummy = {};
var z = 1;

function test() {
    var tmp;

    print(getObjectRefcount(dummy));
    tmp = dummy;
    print(getObjectRefcount(dummy));
    tmp = !z;
    print(getObjectRefcount(dummy));
    print(tmp);
}

try {
    test();
} catch (e) {
    print(e);
}
