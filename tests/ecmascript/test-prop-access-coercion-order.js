/*
 *  The order of checks and coercions for a property access are defined
 *  by the property accessor evaluation rules in E5 Section 11.2.1,
 *  GetValue() in E5 Section 8.7.1, and finally [[Get]].
 *
 *  In short:
 *
 *  - CheckObjectCoercible for base reference.  TypeError for null and
 *    undefined.  (E5 Section 11.2.1 step 5)
 *
 *  - ToString for property name.  This may have side effects and thus
 *    order matters.  (E5 Section 11.2.1 step 6).
 *
 *  - ToObject for base reference.  (E5 Section 8.7.1, special [[Get]],
 *    step 1)
 *
 */

var t = {}
t.toString = function() { print("toString() called"); return "foo"; };

/*===
TypeError
===*/

// CheckObjectCoercible fails, toString() doesn't get called
try {
    null[t] = 'bar';
} catch (e) {
    print(e.name);
}

/*===
toString() called
bar
===*/

// CheckObjectCoercible succeeds, toString() gets called

try {
    var o = {};
    o[t] = 'bar';
    print(o.foo);
} catch (e) {
    print(e.name);
}
