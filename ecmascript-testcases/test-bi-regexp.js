/*
 *  RegExp objects (E5 Section 15.10).
 */

/*===
constructor
true
[object Function]
true 2
true
===*/

function regexpConstructorTest() {
    print(Object.getPrototypeOf(RegExp) === Function.prototype);
    print(Object.prototype.toString.call(RegExp));
    print('length' in RegExp, RegExp.length);
    print('prototype' in RegExp);
}

try {
    print('constructor');
    regexpConstructorTest();
} catch (e) {
    print(e.stack || e);
}

/*===
prototype
true true
function
function
function
===*/

function regexpPrototypeTest() {
    // These are tested in much more detail by the general built-in objects
    // and properties test.

    print('constructor' in RegExp.prototype, RegExp.prototype.constructor === RegExp);
    print(typeof RegExp.prototype.exec);
    print(typeof RegExp.prototype.test);
    print(typeof RegExp.prototype.toString);
}

try {
    print('prototype');
    regexpPrototypeTest();
} catch (e) {
    print(e.stack || e);
}

/*===
instance
source true true
global true true
ignoreCase true true
multiline true true
lastIndex true true
===*/

function regexpInstanceTest() {
    var r = /foo/gi;

    function check(name) {
        print(name, name in r, Object.getOwnPropertyDescriptor(r, name) != null);
    }

    // Properties, E5.1 Section 15.10.7
    //
    // The E5.1 specification indicates that the above properties should be
    // own properties (not inherited).  Check for that, although it's quite
    // likely at least the the flag-related properties will be converted to
    // accessors at some point to save memory.

    check('source');
    check('global');
    check('ignoreCase');
    check('multiline');
    check('lastIndex');
}

try {
    print('instance');
    regexpInstanceTest();
} catch (e) {
    print(e.stack || e);
}
