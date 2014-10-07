/*
 *  'in' operator (E5 Section 11.8.7).
 */

/*FIXME*/

var obj;
var arr;

/*===
true false
===*/

obj = {foo:1};
print('foo' in obj, 'bar' in obj);

/*===
true true false
===*/

arr = [1,2];
print('0' in arr, '1' in arr, '2' in arr);
