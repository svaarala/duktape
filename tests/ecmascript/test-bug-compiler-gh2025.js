/*===
A
B
done
===*/

var j;
var str;
print('A');
Object.defineProperty(Array.prototype, "0", { set: function () {}, configurable: true });
print('B');
eval( 'for (j in this){\nstr+=j;\n}' );
print('done');
