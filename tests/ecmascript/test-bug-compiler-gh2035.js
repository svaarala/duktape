/*===
A
B
done
===*/

print('A');
Object.defineProperty(Array.prototype, 2, { get : Math.random, set : function f ( ) { } } ) ;
print('B');
eval ( "function\u0009\u2029w(\u000C)\u00A0{\u000D};" ) ;
print('done');
