/*
 *  When a finalizer check is made for an object, the internal _finalizer
 *  property is looked up from the prototype chain.  This allows a finalizer
 *  to be inherited, reducing the property count of instance objects.
 */

/*---
{
    "custom": true
}
---*/

/*===
name: example1
name: example2
name: example3
finalizer for name: example1
ex3 finalizer
finalizer for name: example2
finalizer for prototype
===*/

function Example(name) {
    this.name = name;
}
Duktape.fin(Example.prototype, function (obj) {
    // Note: this also gets called for the prototype object when it is
    // collected.
    if (obj === Example.prototype) {
        print('finalizer for prototype');
    } else {
        print('finalizer for name: ' + obj.name);
    }
});
Example.prototype.printName = function () {
    print('name: ' + this.name);
}

function test() {
    var ex1, ex2, ex3;

    ex1 = new Example('example1');
    ex2 = new Example('example2');
    ex3 = new Example('example3');

    // A finalizer found earlier in the prototype chain overrides another
    // higher in the chain (as usual).
    Duktape.fin(ex3, function (obj) {
        print('ex3 finalizer');
    });

    ex1.printName();
    ex2.printName();
    ex3.printName();

    ex1 = null;
    ex3 = null;
    ex2 = null;
}

try {
    test();
} catch (e) {
    print(e);
}
