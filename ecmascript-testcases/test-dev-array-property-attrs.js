/*
 *  Property attributes of array entries.  Also tests the properties with
 *  and without an internal "array part".
 */

function printDesc(desc) {
    print(desc.writable, desc.enumerable, desc.configurable);
}

/*===
true true true
true true true
true true true
true true true
true true true
true true true
true true true
===*/

/* array is initially dense (array part exists) */
a = [1,2,3];

printDesc(Object.getOwnPropertyDescriptor(a, '0'));
printDesc(Object.getOwnPropertyDescriptor(a, '1'));
printDesc(Object.getOwnPropertyDescriptor(a, '2'));

/* force array to be sparse (array part is abandoned) */
a[10000] = 4;

printDesc(Object.getOwnPropertyDescriptor(a, '0'));
printDesc(Object.getOwnPropertyDescriptor(a, '1'));
printDesc(Object.getOwnPropertyDescriptor(a, '2'));
printDesc(Object.getOwnPropertyDescriptor(a, '10000'));
