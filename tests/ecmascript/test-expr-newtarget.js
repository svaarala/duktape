/*
 *  new.target
 */

/*===
MyFunc called
undefined
false
false
MyFunc called
function
true
false
MyFunc called
undefined
false
false
MyFunc called
function
true
false
foo called
undefined
foo called
target-prop-value
outer function
inner undefined
===*/

var bound;

function MyFunc() {
    print('MyFunc called');
    print(typeof new.target);
    print(new
        .
            target === MyFunc);
    print(new /* comment */ .  // another comment
          target === bound);
}

function test() {
    bound = MyFunc.bind(null, 123);

    MyFunc();
    new MyFunc();

    bound();
    new bound();

    function foo() {
        print('foo called');
        if (new.target) {
            print(new.target.target);
        } else {
            print('undefined');
        }
    }
    foo.target = 'target-prop-value';
    foo();
    new foo();

    // Inner function doesn't see outer function new.target.

    function outer() {
        print('outer', typeof new.target);

        function inner() {
            print('inner', typeof new.target);
        }
        inner();
    }
    new outer();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
