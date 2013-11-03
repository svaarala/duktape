/*
 *  Testcases for E5 to E5.1 corrections, Appendix F of the
 *  E5.1 specification.
 */

/*===
foobar
barfoo
===*/

/* 7.8.4: CV definitions added for DoubleStringCharacter :: LineContinuation and SingleStringCharacter ::
 * LineContinuation.
 *
 * (The character value (CV) is defined to be the empty character.)
 */

try {
    print("foo\
bar");
    print('bar\
foo');
} catch (e) {
    print(e);
}

/*===
imm1
function
function
imm2
function
TypeError
should not be modified
===*/

/* 10.2.1.1.3: The argument S is not ignored. It controls whether an exception is thrown when attempting to set
 * an immutable binding.
 *
 * (There are not very many immutable bindings in the spec.  One of them is
 * the name of a named function expression.  Assigning to it should be silently
 * ignored in non-strict mode and a TypeError in strict mode.)
 */

var func = 'should not be modified';
var immutable1 = function func() { print('imm1'); print(typeof func); func = 123; print(typeof func); }
var immutable2 = function func() { 'use strict'; print('imm2'); print(typeof func); func = 123; print(typeof func); }
try { immutable1(); } catch(e) { print(e); }
try { immutable2(); } catch(e) { print(e.name); }
print(func);

/*===
===*/

/* 10.2.1.2.2: In algorithm step 5, true is passed as the last argument to [[DefineOwnProperty]]. */

/* FIXME: impact? property doesn't exist so does the flag matter? */

/*===
===*/

/* 10.5: Former algorithm step 5.e is now 5.f and a new step 5.e was added to restore compatibility with 3rd
 * Edition when redefining global functions.
 */

/* FIXME */

/*===
===*/

/* 11.5.3: In the final bullet item, use of IEEE 754 round-to-nearest mode is specified. */

/* FIXME */

/*===
===*/

/* 12.6.3: Missing ToBoolean restored in step 3.a.ii of both algorithms. */

/* FIXME */

/*===
===*/

/* 12.6.4: Additional final sentences in each of the last two paragraphs clarify certain
 * property enumeration requirements.
 *
 * (Already covered by enumeration tests.)
 */

/*===
undefined
===*/

/* 12.7, 12.8, 12.9: BNF modified to clarify that a continue or break statement without an
 * Identifier or a return statement without an Expression may have a LineTerminator before
 * the semi-colon.
 *
 * (If automatic semicolon insertion kicks in instead, the following semi-colon
 * would be interpreted as an empty statement.  This doesn't usually matter but
 * it does matter e.g. in a naked "if () X else Y".)
 */

try {
    /* FIXME: break, continue */
    print(eval('(function() { if (true) return\n;else return 234 })()'));
} catch (e) {
    print(e);
}

/*===
===*/

/* 12.14: Step 3 of algorithm 1 and step 2.a of algorithm 3 are corrected such
 * that the value field of B is passed as a parameter rather than B itself.
 *
 * (No relevant test.)
 */

/*===
NaN
NaN
===*/

/* 15.1.2.2: In step 2 of algorithm, clarify that S may be the empty string.
 *
 * (Already works, demonstrate with empty string: NaN is provided by step 12.)
 */

try {
    print(parseInt(''));
    print(parseInt('    '));
} catch (e) {
    print(e);
}

/*===
NaN
NaN
===*/

/* 15.1.2.3: In step 2 of algorithm clarify that trimmedString may be the empty string.
 *
 * (Already works, demonstrate with empty string: NaN is provided by step 3,
 * as empty string doesn't match StrDecimalLiteral.)
 */

try {
    print(parseFloat(''));
    print(parseFloat('    '));
} catch (e) {
    print(e);
}

/*===
%23[]!'()*
#%5B%5D!'()*
===*/

/* 15.1.3: Added notes clarifying that ECMAScript‘s URI syntax is based upon RFC 2396
 * and not the newer RFC 3986. In the algorithm for Decode, a step was removed that
 * immediately preceded the current step 4.d.vii.10.a because it tested for a condition
 * that cannot occur.
 *
 * (The URI syntax changes between RFC 2396 and RFC 3986 don't affect URI
 * decoding in Ecmascript, but e.g. reserved character set has changed.  See
 * doc/uri.txt for discussion.)
 */

try {
    print(decodeURI("%23%5B%5D%21%27%28%29%2A"));
    print(encodeURI("#[]!'()*"));
} catch (e) {
    print(e);
}

/*===
===*/

/* 15.2.3.7: Corrected use of variable P in steps 5 and 6 of algorithm.
 *
 * (No test.)
 */

/*===
[object Undefined]
[object Null]
===*/

/* 15.2.4.2: Edition 5 handling of undefined and null as this value caused existing code to fail. Specification
 * modified to maintain compatibility with such code. New steps 1 and 2 added to the algorithm.
 */

try { print(Object.prototype.toString.call(undefined)); } catch (e) { print(e); }
try { print(Object.prototype.toString.call(null)); } catch (e) { print(e); }

/*===
apply: mythis foo bar quux
apply: mythis undefined undefined undefined
apply: mythis undefined undefined undefined
apply: mythis undefined undefined undefined
apply: mythis undefined undefined undefined
apply: mythis foo undefined undefined
===*/

/* 15.3.4.3: Steps 5 and 7 of Edition 5 algorithm have been deleted because they imposed requirements upon
 * the argArray argument that are inconsistent with other uses of generic array-like objects.
 *
 * (This only affects apply() calls where argArray is an object, but not an
 * Array, so it may have an arbitrary or missing 'length' value.  E5.1 states
 * that ToUint32() coercion is blindly done: undefined/null length coerces to
 * 0 instead of causing a TypeError.)
 */

function applyTest(x,y,z) { print('apply:', this, x, y, z); }

try { applyTest.apply('mythis', [ 'foo', 'bar', 'quux' ]) } catch (e) { print(e); }

// no length -> 0
try { applyTest.apply('mythis', { "0": 'foo', "1": 'bar', "2": 'quux' }) } catch (e) { print(e); }

// undefined or null length -> 0
try { applyTest.apply('mythis', { "0": 'foo', "1": 'bar', "2": 'quux', "length": undefined }) } catch (e) { print(e); }
try { applyTest.apply('mythis', { "0": 'foo', "1": 'bar', "2": 'quux', "length": null }) } catch (e) { print(e); }

// already required by E5, false->0, true->1
try { applyTest.apply('mythis', { "0": 'foo', "1": 'bar', "2": 'quux', "length": false }) } catch (e) { print(e); }
try { applyTest.apply('mythis', { "0": 'foo', "1": 'bar', "2": 'quux', "length": true }) } catch (e) { print(e); }

/*===
===*/

/* 15.4.4.12: In step 9.a, incorrect reference to relativeStart was replaced with a reference to actualStart. */

/*===
===*/

/* 15.4.4.15: Clarified that the default value for fromIndex is the length minus 1 of the array. */

/*===
===*/

/* 15.4.4.18: In step 9 of the algorithm, undefined is now the specified return value. */

/*===
===*/

/* 15.4.4.22: In step 9.c.ii the first argument to the [[Call]] internal method has been
 * changed to undefined for consistency with the definition of Array.prototype.reduce.
 */

/*===
===*/

/* 15.4.5.1: In Algorithm steps 3.l.ii and 3.l.iii the variable name was inverted resulting
 * in an incorrectly inverted test.
 */

/*===
===*/

/* 15.5.4.9: Normative requirement concerning canonically equivalent strings deleted from paragraph following
 * algorithm because it is listed as a recommendation in NOTE 2.
 */

/*===
===*/

/* 15.5.4.14: In split algorithm step 11.a and 13.a, the positional order of the arguments to SplitMatch was
 * corrected to match the actual parameter signature of SplitMatch. In step 13.a.iii.7.d, lengthA replaces A.length.
 */

/*===
===*/

/* 15.5.5.2: In first paragraph, removed the implication that the individual character property access had array
 * index semantics. Modified algorithm steps 3 and 5 such that they do not enforce array index requirement.
 */

/*===
===*/

/* 15.9.1.15: Specified legal value ranges for fields that lacked them. Eliminated time-only formats. Specified
 * default values for all optional fields.
 */

/*===
===*/

/* 15.10.2.2: The step numbers of the algorithm for the internal closure produced by step 2 were incorrectly
 * numbered in a manner that implied that they were steps of the outer algorithm.
 */

/*===
===*/

/* 15.10.2.6: In the abstract operation IsWordChar the first character in the list in step 3 is a rather than A.
 */

/*===
===*/

/* 15.10.2.8: In the algorithm for the closure returned by the abstract operation CharacterSetMatcher, the variable
 * defined by step 3 and passed as an argument in step 4 was renamed to ch in order to avoid a name conflict
 * with a formal parameter of the closure.
 */

/*===
===*/

/* 15.10.6.2: Step 9.e was deleted because It performed an extra increment of i. */

/*===
===*/

/* 15.11.1.1: Removed requirement that the message own property is set to the empty String when the message
 * argument is undefined.
 */

/*===
===*/

/* 15.11.1.2: Removed requirement that the message own property is set to the empty String when the message
 * argument is undefined.
 */

/*===
===*/

/* 15.11.4.4: Steps 6-10 modified/added to correctly deal with missing or empty message property value. */

/*===
===*/

/* 15.11.1.2: Removed requirement that the message own property is set to the empty String when the message
 * argument is undefined.
 */

/*===
===*/

/* 15.12.3: In step 10.b.iii of the JA internal operation, the last element of the concatenation is ]. */

/*===
===*/

/* B.2.1: Added to NOTE that the encoding is based upon RFC 1738 rather than the newer RFC 3986. */

/*===
===*/

/* Annex C: An item was added corresponding to 7.6.12 regarding FutureReservedWords in strict mode. */


