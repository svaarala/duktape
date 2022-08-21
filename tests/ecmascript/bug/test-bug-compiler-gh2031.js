/*===
A
B
ReferenceError
===*/

try {
    print('A');
    Object.defineProperty(Array.prototype, 0, { set : function ( ) { } } ) ;
    print('B');
    eval('var x; for (x++ in [0,1]) {}');
    print('done');
} catch (e) {
    print(e.name);
}
