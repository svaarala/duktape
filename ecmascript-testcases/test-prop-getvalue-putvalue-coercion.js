/*
 *  GetValue() and PutValue() coerce the base reference to an object if it
 *  is a primitive value, and then use a variant of [[Get]] and [[Put]] to
 *  access the value.
 *
 *  This variant is identical to the default one except for one small detail:
 *  if a getter/setter is invoked, the 'this' binding for the getter/setter
 *  is the primitive value, not the coerced object.  Presumably this is nicer
 *  from an implementation perspective, because the coerced object is never
 *  exposed to user code (even through a getter/setter).
 *
 *  Note that the getter/setter must be strict to avoid a this binding
 *  object coercion which happens during call setup: E5 Section 10.4.3.
 *
 *  Implementation behavior differs:
 *
 *    - Some implementations provide the coerced (object) value to the
 *      setter/getter (e.g. Rhino and Smjs)
 *
 *    - Some implementations omit the "foo".test assignment test below
 *      entirely (e.g. NodeJS / V8).  This is presumably because the
 *      assignment goes to a temporary object which is never accessible
 *      to user code.  So except for the setter call side effect and
 *      potential error throwing side effects, the assignment has no
 *      observable effect.
 */

/*===
string
object
string
object
===*/

// add test getter
Object.defineProperty(String.prototype, 'test', {
  get: function() { 'use strict'; print(typeof this); },
  set: function(x) { 'use strict'; print(typeof this); },
});

var s = new String("foo");

// Should print 'string'.  Rhino and Smjs print 'object'.
"foo".test;

// Should print 'object'.
s.test;

// Should print 'string'.  Rhino and Smjs print 'object, while
// V8 omits the statement entirely and prints nothing.
"foo".test = "bar";

// Should print 'object'.
s.test = "bar";
