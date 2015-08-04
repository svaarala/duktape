/*===

1 2
===*/

/* Basic cases */
print.call(null);
print.call(null,1,2);

/*===
TypeError
===*/

/* Contrived case where E5 Section 15.3.4.4 step 1 should cause TypeError */

try {
    ({ "call": Function.prototype.call }).call();
} catch (e) {
    print(e.name);
}

/* XXX: thisArg handling */
