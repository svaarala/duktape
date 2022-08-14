/*===
1
===*/

/* Specific test: array length must be updated by defineProperty / defineProperties */

var arr = [];
Object.defineProperties(arr, { '0': { get: function(){return 'get';}, set:function(){}, enumerable: true, configurable: true } })
print(arr.length);
