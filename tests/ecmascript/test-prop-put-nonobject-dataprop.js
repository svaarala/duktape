/*
 *  If a property put base value is not an object, the PutValue() algorithm
 *  will reject any attempt to create or update a data property on the
 *  temporary object created by coercion.
 *
 *  This only has a visible effect in strict mode.
 *
 *  See E5 Section 8.7.2, step 7.a of the variant [[Put]] algorithm.
 *
 *  Note that the check in E5 Section 8.7.2, step 4 seems impossible to
 *  trigger.  The possible temporary objects created are: Boolean, Number,
 *  String.  Boolean and Number have no "own properties" while String has
 *  only non-writable properties which would trigger the [[CanPut]] check
 *  in step 2 so step 4 would never be reached.
 */

/*===
TypeError
===*/

function f() {
  'use strict';

  "foo".bar = 1;  /* TypeError in strict mode */
};

try {
    f();
    print("not thrown");
} catch (e) {
    print(e.name);
}
