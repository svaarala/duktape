/*
 *  Demonstrate 'this' binding behavior for "direct calls to eval" made
 *  from Ecmascript code.  The behavior for both strict and non-strict is
 *  the same: the 'this' binding is inherited from the calling context,
 *  see E5.1 Section 10.4.2:
 *
 *    - Step 2 will match (direct call) and establish a 'this' binding
 *      in step 2.a, independent of target code strictness.
 *
 *    - Step 3 will match for strict code only and will update the lexical
 *      environment, but the 'this' binding is not affected.
 *
 *  The C API eval behavior (duk_(p)eval_*) was different prior to Duktape 1.3:
 *  eval code 'this' binding was the global object for non-strict eval code,
 *  but undefined for strict eval code.  This was fixed as part of GH-164:
 *
 *      https://github.com/svaarala/duktape/issues/164
 */

/*===
non-strict eval code
not an error in non-strict code
object
object
strict eval code
ReferenceError
object
object
===*/

function test() {
    eval("print('non-strict eval code');\n" +
         "try { dummy1 = 1; print('not an error in non-strict code'); } catch (e) { print(e.name); }\n" +
         "print(typeof this);\n" +
         "print(typeof (this || {}).Math);\n");

    eval("'use strict';\n" +
         "print('strict eval code');\n" +
         "try { dummy2 = 1; print('error in non-strict code'); } catch (e) { print(e.name); }\n" +
         "print(typeof this);\n" +
         "print(typeof (this || {}).Math);\n");
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
