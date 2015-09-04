/*
 *  Array length value X is checked and coerced in E5 Section 15.4.5.1
 *  in a rather interesting way.
 *
 *  To be a valid length, X must pass the following:
 *    - ToUint32(X) == ToNumber(X)
 *
 *  Depending on the type of X, this has various outcomes:
 *
 *    - Undefined: not allowed; ToUint32(undefined) = 0, ToNumber(undefined) = NAN
 *    - Null: allowed, ToUint32(null) = 0, ToNumber(null) = 0
 *    - Boolean: allowed, ToUint32(true) = 1, ToNumber(true) = 1;
 *                        ToUint32(false) = 0, ToNumber(false) = 0
 *    - Number: X must be a 32-bit unsigned integer
 *    - String: XXX
 *    - Object: XXX; through coercion
 */

var a;

/*===
RangeError
===*/

a = [1,2,3];
try {
    a.length = undefined;
    print(a.length);
} catch (e) {
    print(e.name);
}

/*===
0
===*/

a = [1,2,3];
try {
    a.length = null;
    print(a.length);
} catch (e) {
    print(e.name);
}

/*===
0
===*/

a = [1,2,3];
try {
    a.length = false;
    print(a.length);
} catch (e) {
    print(e.name);
}

/*===
1
===*/

a = [1,2,3];
try {
    a.length = true;
    print(a.length);
} catch (e) {
    print(e.name);
}

/*===
2
===*/

a = [1,2,3];
try {
    a.length = 2.0;
    print(a.length);
} catch (e) {
    print(e.name);
}

/*===
5
===*/

a = [1,2,3];
try {
    a.length = 5.0;
    print(a.length);
} catch (e) {
    print(e.name);
}

/*===
2
===*/

a = [1,2,3];
try {
    a.length = "2";
    print(a.length);
} catch (e) {
    print(e.name);
}

/*===
2
===*/

a = [1,2,3];
try {
    a.length = "2.000";
    print(a.length);
} catch (e) {
    print(e.name);
}

/*===
2
===*/

a = [1,2,3];
try {
    a.length = "0.02e2";
    print(a.length);
} catch (e) {
    print(e.name);
}

/* XXX: strings and objects */
