/*
 *  Arithmetic operators (E5 Sections 11.4.6, 11.4.7, 11.5.1, 11.5.2,
 *  11.5.3, 11.6.1, 11.6.2, 11.6.3).
 *
 *  Operations need to be performed on both constants and variables
 *  (and property accesses etc) because some constant operations are
 *  performed at compile-time.
 *
 *  To distinguish zero signs, the idiom '1 / x' is used.  If x is +0,
 *  this will result in Infinity, and if x is -0, it will result in
 *  -Infinity.  See [[Issues/ZeroSign]].
 */

/*---
{
    "skip": true
}
---*/

/*
 *  FIXME: add very small and very large numbers.
 *  FIXME: add IEEE compliance checks.
 *  FIXME: add integer arithmetic checks for at least 2^32 range
 */

/*** UNARY PLUS ***/

/*===
undefined
NaN number
0 number
1 number
0 number
1 number
-1 number
NaN number
Infinity number
1 number
-15 number
1 number
-15 number
1 number
1 number
2000 number
1 number
Infinity number
===*/

/* ToNumber conversions are specified in E5 Section 9.3, with the
 * string grammar given in E5 Section 9.3.1.  The string grammar
 * is tested in detail in test-conv-tonumber.js.
 */

var t;
var o = { k1: 1, k2: "-1.5e1", 3: true };
var a = [ 1, "2e3", true ];

print(typeof undefined);

t = +undefined;
print(t, typeof t);
t = +null;
print(t, typeof t);
t = +true;
print(t, typeof t);
t = +false;
print(t, typeof t);
t = +1;
print(t, typeof t);
t = -1;
print(t, typeof t);
t = +NaN;
print(t, typeof t);
t = +Infinity;
print(t, typeof t);
t = +"1";
print(t, typeof t);
t = +"-1.5e1";
print(t, typeof t);
t = +o.k1;
print(t, typeof t);
t = +o['k2'];
print(t, typeof t);
t = +o[3];
print(t, typeof t);
t = +a[0];
print(t, typeof t);
t = +a[1];
print(t, typeof t);
t = +a[2];
print(t, typeof t);
t = 1 / +0;
print(t, typeof t);

t = undefined;
o = undefined;
a = undefined;


/*** UNARY MINUS ***/

/*===
undefined
NaN number
0 number
-1 number
0 number
-1 number
1 number
NaN number
-Infinity number
-1 number
15 number
-1 number
15 number
-1 number
-1 number
-2000 number
-1 number
-Infinity number
===*/

var t;
var o = { k1: 1, k2: "-1.5e1", 3: true };
var a = [ 1, "2e3", true ];

print(typeof(undefined));

t = -undefined;
print(t, typeof t);
t = -null;
print(t, typeof t);
t = -true;
print(t, typeof t);
t = -false;
print(t, typeof t);
t = -1;
print(t, typeof t);
t = - -1;
print(t, typeof t);
t = -NaN;
print(t, typeof t);
t = -Infinity;
print(t, typeof t);
t = -"1";
print(t, typeof t);
t = -"-1.5e1";
print(t, typeof t);
t = -o.k1;
print(t, typeof t);
t = -o['k2'];
print(t, typeof t);
t = -o[3];
print(t, typeof t);
t = -a[0];
print(t, typeof t);
t = -a[1];
print(t, typeof t);
t = -a[2];
print(t, typeof t);
t = 1 / -0;
print(t, typeof t);

t = undefined;
o = undefined;
a = undefined;


/*** MULTIPLICATION ***/

/* FIXME: perform the same operations from variables and constants.  Need some
 * eval() + code generation here?
 */

function multest(op1, op2) {
    var code = op1 + " * " + op2;
    var res = eval(code);
    if (res === 0) {
        print(res, typeof res, 1 / res);
    } else {
        print(res, typeof res);
    }

    code = "var t1 = " + op1 + "; ";
    code += "var t2 = " + op2 + "; ";
    code += "t1 * t2";
    res = eval(code);
    if (res === 0) {
        print(res, typeof res, 1 / res);
    } else {
        print(res, typeof res);
    }
}


/*===
6 number
-6 number
-6 number
6 number
120 number
120 number
120 number
===*/

t = 2 * 3;
print (t, typeof t);
t = 2 * -3;
print (t, typeof t);
t = -2 * 3;
print (t, typeof t);
t = -2 * -3;
print (t, typeof t);
t = 2 * 3 * 4 * 5;
print (t, typeof t);
t = 2 * (3 * (4 * 5));
print (t, typeof t);
t = ((2 * 3) * 4) * 5;
print (t, typeof t);

/* Special values: +0, -0, +Infinity, -Infinity, NaN, try all combinations
 * (also including +1 and -1).
 *
 * XXX: check NaN equivalence somehow?
 */

/*===
0 number Infinity
0 number -Infinity
NaN number
NaN number
NaN number
0 number Infinity
0 number -Infinity
===*/

t = +0 * +0;
print (t, typeof t, 1 / t);
t = +0 * -0;
print (t, typeof t, 1 / t);
t = +0 * Infinity;
print (t, typeof t);
t = +0 * -Infinity;
print (t, typeof t);
t = +0 * NaN;
print (t, typeof t);
t = +0 * +1;
print (t, typeof t, 1 / t);
t = +0 * -1;
print (t, typeof t, 1 / t);

/*===
0 number -Infinity
0 number Infinity
NaN number
NaN number
NaN number
0 number -Infinity
0 number Infinity
===*/

t = -0 * +0;
print (t, typeof t, 1 / t);
t = -0 * -0;
print (t, typeof t, 1 / t);
t = -0 * Infinity;
print (t, typeof t);
t = -0 * -Infinity;
print (t, typeof t);
t = -0 * NaN;
print (t, typeof t);
t = -0 * +1;
print (t, typeof t, 1 / t);
t = -0 * -1;
print (t, typeof t, 1 / t);

/*===
NaN number
NaN number
Infinity number
-Infinity number
NaN number
Infinity number
-Infinity number
===*/

t = Infinity * +0;
print (t, typeof t);
t = Infinity * -0;
print (t, typeof t);
t = Infinity * Infinity;
print (t, typeof t);
t = Infinity * -Infinity;
print (t, typeof t);
t = Infinity * NaN;
print (t, typeof t);
t = Infinity * +1;
print (t, typeof t);
t = Infinity * -1;
print (t, typeof t);

/*===
NaN number
NaN number
-Infinity number
Infinity number
NaN number
-Infinity number
Infinity number
===*/

t = -Infinity * +0;
print (t, typeof t);
t = -Infinity * -0;
print (t, typeof t);
t = -Infinity * Infinity;
print (t, typeof t);
t = -Infinity * -Infinity;
print (t, typeof t);
t = -Infinity * NaN;
print (t, typeof t);
t = -Infinity * +1;
print (t, typeof t);
t = -Infinity * -1;
print (t, typeof t);

/*===
NaN number
NaN number
NaN number
NaN number
NaN number
NaN number
NaN number
===*/

t = NaN * +0;
print (t, typeof t);
t = NaN * -0;
print (t, typeof t);
t = NaN * Infinity;
print (t, typeof t);
t = NaN * -Infinity;
print (t, typeof t);
t = NaN * NaN;
print (t, typeof t);
t = NaN * +1;
print (t, typeof t);
t = NaN * -1;
print (t, typeof t);

/* rounding to zero and infinity */

/*===
0 number Infinity
Infinity number
===*/

t = 1e200 * 1e-200 * 1e-200 * 1e-200;
print (t, typeof t, 1 / t);

t = 1e200 * 1e200;
print (t, typeof t);


/*** DIVISION ***/

/*===
FIXME
===*/


/*** MODULO ***/

/*===
FIXME
===*/


/*** ADDITION ***/

/*===
FIXME
===*/


/*** SUBTRACTION ***/

/*===
FIXME
===*/


