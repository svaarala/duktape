/*===
A
B
done
===*/

print('A');
Object.defineProperty(Array.prototype, 0, { get : Math.asin, set : function f( ) { } });
print('B');
eval('([ 123 ] / 2)');
print('done');
