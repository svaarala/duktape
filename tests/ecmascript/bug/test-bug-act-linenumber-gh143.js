/*
 *  https://github.com/svaarala/duktape/issues/143
 */

/*---
custom: true
---*/

/*===
-1 0
-2 23
-3 25
-4 29
25
-1 0
-2 23
-3 26
-4 29
26
===*/

(function() {
function cows_eat_kitties() { for (var i = -1; i >= -4; i--) { print(i, Duktape.act(i).lineNumber); }; print(Duktape.act(-3).lineNumber); }
function pigs_eat_cows() { return cows_eat_kitties(); }
pigs_eat_cows();
pigs_eat_cows();


})();

/* Callstack for Duktape.act() in cows_eat_kitties():
 *
 *    - -1: Duktape.act, lineNumber 0
 *    - -2: cows_eat_kitties()
 *    - -3: call site on either line 21 or 22
 *
 *          => these line numbers are wrong in Duktape 1.1.x, 22 and 25
 *             instead of 21 and 22.
 *
 * pigs_eat_cows() is not in the callstack because it tailcalls
 * cows_eat_kitties() and gets replaced by it.
 */
