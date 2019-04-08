/*
 *  Proxy (ES2015) 'getOwnPropertyDescriptor'.
 */

/*@include util-object.js@*/

/*===
value=123,writable=true,enumerable=false,configurable=true
get=function,enumerable=true,configurable=false
set=function,enumerable=false,configurable=false
get=function,set=function,enumerable=true,configurable=true
value=123,writable=true,enumerable=false,configurable=true
get=function,enumerable=true,configurable=false
set=function,enumerable=false,configurable=false
get=function,set=function,enumerable=true,configurable=true
===*/

function passThroughTest() {
    var T = {};
    var P = new Proxy(T, {});
    var pd;

    Object.defineProperty(T, 'foo', { value: 123, writable: true, enumerable: false, configurable: true });
    Object.defineProperty(T, 'bar', { get: function getter() {}, enumerable: true, configurable: false });
    Object.defineProperty(T, 'quux', { set: function setter() {}, enumerable: false, configurable: false });
    Object.defineProperty(T, 'baz', { get: function getter() {}, set: function setter() {}, enumerable: true, configurable: true });

    pd = Object.getOwnPropertyDescriptor(T, 'foo');
    printPropDesc(pd);
    pd = Object.getOwnPropertyDescriptor(T, 'bar');
    printPropDesc(pd);
    pd = Object.getOwnPropertyDescriptor(T, 'quux');
    printPropDesc(pd);
    pd = Object.getOwnPropertyDescriptor(T, 'baz');
    printPropDesc(pd);

    // As of Duktape 2.2 getOwnPropertyDescriptor() properly passes through
    // to the target object.
    pd = Object.getOwnPropertyDescriptor(P, 'foo');
    printPropDesc(pd);
    pd = Object.getOwnPropertyDescriptor(P, 'bar');
    printPropDesc(pd);
    pd = Object.getOwnPropertyDescriptor(P, 'quux');
    printPropDesc(pd);
    pd = Object.getOwnPropertyDescriptor(P, 'baz');
    printPropDesc(pd);
}

try {
    passThroughTest();
} catch (e) {
    print(e.stack || e);
}

/*===
===*/

function miscTest() {
    var O = { foo: 123 };
    var P = new Proxy(O, {});

    printPropDesc(Object.getOwnPropertyDescriptor(O, 'foo'));
    printPropDesc(Object.getOwnPropertyDescriptor(P, 'foo'));

    var O = { foo: 123 };
    var P = new Proxy(O, {
        getOwnPropertyDescriptor: function () {
            return { value: 321, writable: false, enumerable: false, configurable: true };
        }
    });

    printPropDesc(Object.getOwnPropertyDescriptor(O, 'foo'));
    printPropDesc(Object.getOwnPropertyDescriptor(P, 'foo'));
}

try {
    miscTest();
} catch (e) {
    print(e.stack || e);
}
