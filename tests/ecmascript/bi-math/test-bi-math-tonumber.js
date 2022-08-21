/*
 *  Math object ToNumber side effects (ES5 15.8.2)
 *
 *  ECMAScript 5.1 requires that all Math object methods coerce all arguments
 *  with ToNumber from left to right.  This ensures that all side effects
 *  caused by ToNumber coercion have a chance to be evaluated.
 */

var pig = { valueOf: function() { print("pig"); return 812; } };
var cow = { valueOf: function() { print("cow"); return 1208; } };
var ape = { valueOf: function() { print("ape"); return -128; } };
var goku = { valueOf: function() { print("goku"); return 9001.5; } };
var badger = { valueOf: function() { print("badger"); return 8; } };
var munch = { valueOf: function() { print("*munch*"); return NaN; } };

/*===
pig
===*/
Math.abs(pig);

/*===
pig
cow
ape
pig
cow
ape
goku
badger
===*/
Math.cos(pig);
Math.sin(cow);
Math.tan(ape);
Math.acos(pig);
Math.asin(cow);
Math.atan(ape);
Math.atan2(goku, badger);

/*===
badger
*munch*
pig
goku
ape
badger
*munch*
===*/
Math.exp(badger);
Math.exp(munch);
Math.log(pig);
Math.pow(goku, ape);
Math.sqrt(badger);
Math.sqrt(munch);

/*===
pig
cow
ape
===*/
Math.floor(pig);
Math.ceil(cow);
Math.round(ape);

/*===
pig
cow
ape
goku
goku
ape
cow
pig
*munch*
pig
*munch*
cow
===*/
Math.min(pig, cow, ape, goku);
Math.max(goku, ape, cow, pig);
Math.min(munch, pig);
Math.max(munch, cow);
