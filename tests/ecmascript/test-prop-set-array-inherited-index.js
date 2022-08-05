/*===
1 foo
1 foo
aiee
===*/

Object.defineProperty(Array.prototype, '1000', { value: 'aiee', writable: false });
var arr = [];
arr[0] = 'foo';
print(arr.length, String(arr));
arr[1000] = 'bar';  // Inherited '1000' key is not writable.
print(arr.length, String(arr));
print(arr[1000]);
