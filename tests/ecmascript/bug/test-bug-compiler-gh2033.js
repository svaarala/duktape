/*===
A
B
===*/

print('A');
Object.defineProperty(Array.prototype, 0, { set : function( ) {} });
print('B');
eval('if (true) {} /foo/.test("foo")');
