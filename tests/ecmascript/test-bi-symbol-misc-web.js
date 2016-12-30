/*
 *  Miscellaneous Symbol tests, based on examples on the web.
 *
 *  https://developer.mozilla.org/en/docs/Web/JavaScript/Reference/Global_Objects/Symbol
 *  https://www.keithcirkel.co.uk/metaprogramming-in-es6-symbols/
 */

/*===
bar
quux
undefined
foo
true
===*/

function miscSymbolTest() {
    // Symbols can appear in object literals using ES2015 computed property names.
    var obj = {
        foo: 'bar',
        [ Symbol.for('bar') ]: 'quux'
    };
    print(obj.foo);
    print(obj[Symbol.for('bar')]);

    // Symbol.keyFor() maps a global symbol back to its requested name.
    var localFoo = Symbol('foo');
    var globalFoo = Symbol.for('foo');
    print(Symbol.keyFor(localFoo));
    print(Symbol.keyFor(globalFoo));
    print(Symbol.for(Symbol.keyFor(globalFoo)) === Symbol.for('foo'));
}

try {
    miscSymbolTest();
} catch (e) {
    print(e.stack || e);
}
