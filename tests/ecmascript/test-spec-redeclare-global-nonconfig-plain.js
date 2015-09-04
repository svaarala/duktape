/*
 *  Redeclare a global non-configurable plain (data) property, E5.1 change.
 */

/* Empty output is intentional: Program-wide TypeError happens after
 * compilation when function declarations are initialized in the
 * function prologue.  Nothing gets output before that finishes.
 */

/* XXX: how to better test this so that the TypeError could be
 * actually checked for?
 */

print('never here');

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
