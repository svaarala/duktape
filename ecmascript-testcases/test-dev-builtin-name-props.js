// marked as custom because 'name' property of some built-ins is not
// specified exactly and implementations differ

/*---
{
    "custom": true
}
---*/

/*===
undefined
Object
undefined
Function

Array
undefined
String
undefined
Boolean
undefined
Number
undefined
undefined
Date
undefined
RegExp
undefined
Error
Error
EvalError
EvalError
RangeError
RangeError
ReferenceError
ReferenceError
SyntaxError
SyntaxError
TypeError
TypeError
URIError
URIError
undefined
===*/

/* Names of standard built-ins. */

print(this.name);

print(Object.name);
print(Object.prototype.name);

// Function is a special case: Function is a constructor (a function),
// but Function.prototype is also a function and should thus have a name.
// Rhino has "", V8 has "Empty"; we use "" too for now.
print(Function.name);
print(Function.prototype.name);

print(Array.name);
print(Array.prototype.name);

print(String.name);
print(String.prototype.name);

print(Boolean.name);
print(Boolean.prototype.name);

print(Number.name);
print(Number.prototype.name);

print(Math.name);

print(Date.name);
print(Date.prototype.name);

print(RegExp.name);
print(RegExp.prototype.name);

// Error objects are not function objects, but they have a 'name'
// property for the error type

print(Error.name);
print(Error.prototype.name);

print(EvalError.name);
print(EvalError.prototype.name);

print(RangeError.name);
print(RangeError.prototype.name);

print(ReferenceError.name);
print(ReferenceError.prototype.name);

print(SyntaxError.name);
print(SyntaxError.prototype.name);

print(TypeError.name);
print(TypeError.prototype.name);

print(URIError.name);
print(URIError.prototype.name);

print(JSON.name);

/*===
ThrowTypeError
===*/

/* Name of the "type error thrower" of E5 Section 13.2.3.  This is not
 * defined in the specification; V8 uses "ThrowTypeError" which we also
 * use.
 */

function func() {
    'use strict';
}

try {
    var desc = Object.getOwnPropertyDescriptor(func, 'caller');
    print(desc.get.name);
} catch (e) {
    print(e.name, e);
}

/*===
undefined
===*/

/* XXX: other non-standard builtins */

try {
    print(Duktape.name);
} catch (e) {
    print(e.name);
}
