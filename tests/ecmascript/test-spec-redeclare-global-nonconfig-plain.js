/*
 *  Redeclare a global non-configurable plain (data) property, E5.1 change.
 */

/*---
{
    "skip": true
}
---*/

/* Empty output is intentional: Program-wide TypeError. */

/* FIXME */

/*===
===*/

/* NaN is non-configurable.  Attempt to redeclare -> TypeError.
 * Nothing gets printed because the TypeError will run before
 * anything else (as part of binding instantiation).
 */

var desc;
var indirectEval = eval;
var global = indirectEval("this");  // for Node

// Note: since declarations are hoisted, the RegExp declaration below
// will already have taken effect here.

print(typeof NaN);
desc = Object.getOwnPropertyDescriptor(global, 'NaN');
print(desc.writable, desc.enumerable, desc.configurable);

function NaN() {
    return 123;
}

// just checking that order does not matter
print(typeof NaN);
desc = Object.getOwnPropertyDescriptor(global, 'NaN');
print(desc.writable, desc.enumerable, desc.configurable);
