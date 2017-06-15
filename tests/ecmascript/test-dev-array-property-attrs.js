/*
 *  Property attributes of array entries.  Also tests the properties with
 *  and without an internal "array part".
 */

/*@include util-object.js@*/

/*===
value=1,writable=true,enumerable=true,configurable=true
value=2,writable=true,enumerable=true,configurable=true
value=3,writable=true,enumerable=true,configurable=true
value=1,writable=true,enumerable=true,configurable=true
value=2,writable=true,enumerable=true,configurable=true
value=3,writable=true,enumerable=true,configurable=true
value=4,writable=true,enumerable=true,configurable=true
===*/

/* array is initially dense (array part exists) */
a = [1,2,3];

printPropDesc(Object.getOwnPropertyDescriptor(a, '0'));
printPropDesc(Object.getOwnPropertyDescriptor(a, '1'));
printPropDesc(Object.getOwnPropertyDescriptor(a, '2'));

/* force array to be sparse (array part is abandoned) */
a[10000] = 4;

printPropDesc(Object.getOwnPropertyDescriptor(a, '0'));
printPropDesc(Object.getOwnPropertyDescriptor(a, '1'));
printPropDesc(Object.getOwnPropertyDescriptor(a, '2'));
printPropDesc(Object.getOwnPropertyDescriptor(a, '10000'));
