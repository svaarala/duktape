/*
 *  If Reflect.construct() is used on a bound target, the bound function
 *  arguments are handled normally but the 'this' binding is ignored for
 *  constructor calls.
 */

/*===
object [object Object]
bindThis
number 100
number 200
number 1
number 2
object [object Object]
bindThis
number 100
number 200
number 1
number 2
object [object Object]
protoThis
number 100
number 200
number 1
number 2
object [object Object]
protoThis
number 100
number 200
number 1
number 2
===*/

function MyConstructor(a, b, c, d) {
    print(typeof this, this);
    print(this.name);
    print(typeof a, a);
    print(typeof b, b);
    print(typeof c, c);
    print(typeof d, d);
}

MyConstructor.prototype.name = 'protoThis';

function test() {
    var bound = MyConstructor.bind({ name: 'bindThis' }, 100, 200);

    // For normal calls, the "closest" 'this' binding wins.
    bound(1, 2, 3);
    bound.apply({ name: 'applyThis' }, [ 1, 2, 3 ]);

    // For constructor calls the default instance created is never
    // overwritten during bound function resolution.
    new bound(1, 2, 3);
    Reflect.construct(bound, [ 1, 2, 3 ]);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
