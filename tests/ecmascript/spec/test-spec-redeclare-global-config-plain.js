/*
 *  Redeclare a global configurable plain (data) property, E5.1 change.
 */

/*===
function
true true false
function
true true false
123
===*/

/* RegExp is configurable.
 *
 * Re-declaring it should change the binding, and make the new property
 * writable, non-configurable, and enumerable.  The property becomes
 * non-configurable because configurableBindings=false in E5.1 Section 10.5,
 * 5.e.iii for Program code.
 */

var desc;
var indirectEval = eval;
var global = indirectEval("this");  // for Node

// Note: since declarations are hoisted, the RegExp declaration below
// will already have taken effect here.  The original RegExp binding
// is writable and configurable, but not enumerable.

print(typeof RegExp);
desc = Object.getOwnPropertyDescriptor(global, 'RegExp');
print(desc.writable, desc.enumerable, desc.configurable);

function RegExp() {
    return 123;
}

// just checking that order does not matter
print(typeof RegExp);
desc = Object.getOwnPropertyDescriptor(global, 'RegExp');
print(desc.writable, desc.enumerable, desc.configurable);

print(RegExp());
