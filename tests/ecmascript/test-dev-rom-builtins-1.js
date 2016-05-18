/*
 *  Some tests for using ROM strings and builtins, but a writable global
 *  object.
 *
 *  Run manually.  The test case assumes DUK_USE_ROM_GLOBAL_INHERIT.
 */

/*---
{
    "custom": true,
    "skip": true
}
---*/

var global = new Function('return this;')();

/*===
--- extensibility
Math extensible: false
global extensible: true
===*/

function romObjectExtensibleTest() {
    /* ROM objects are marked non-extensible so that attempts to create new
     * properties on them will fail.
     */
    print('Math extensible: ' + Object.isExtensible(Math));

    /* When global object is made writable (DUK_USE_ROM_GLOBAL_CLONE or
     * DUK_USE_ROM_GLOBAL_INHERIT) it is extensible to allow new properties
     * to be added.
     */
    print('global extensible: ' + Object.isExtensible(global));
}

/*===
--- global object
property descriptor of global.RegExp: undefined
RegExp in global: true
===*/

function romObjectGlobalObjectTest() {
    var pd;

    /* When DUK_USE_ROM_GLOBAL_INHERIT is used, ordinary global object
     * properties are inherited from a (non-standard) ROM global object
     * so that properties won't appear as "own" properties of the global
     * object.
     */
    pd = Object.getOwnPropertyDescriptor(global, 'RegExp');
    print('property descriptor of global.RegExp:', Duktape.enc('jx', pd));
    print('RegExp in global:', 'RegExp' in global);
}

/*===
--- property attributes
property descriptor of RegExp.prototype.exec: {value:{_func:true},writable:true,enumerable:false,configurable:false}
123
in non-strict mode the failed write is ignored silently
nonwritable property
in non-strict mode the failed write is ignored silently
function
number
{value:0,writable:true,enumerable:false,configurable:false}
in non-strict mode the failed write is ignored silently
Array.prototype[0]: undefined
in non-strict mode the failed write is ignored silently
Array.prototype.length: 0
===*/

function romObjectPropertyAttributeTest() {
    var pd;

    /* Property attributes for properties of ROM objects:
     *
     *   - Configurable: always false, property cannot be edited.
     *   - Enumerable: depends on property.
     *   - Writable: depends on property.
     *
     * The "writable" attribute is set to "true" to allow an inheriting
     * object to gain overriding properties, but actual writes to the
     * ROM object itself will fail.
     */
    pd = Object.getOwnPropertyDescriptor(RegExp.prototype, 'exec');
    print('property descriptor of RegExp.prototype.exec:', Duktape.enc('jx', pd));

    /* Example of how a non-writable ancestor property would prevent
     * creating a new property on an object.
     */
    var parent = {};
    Object.defineProperty(parent, 'prop', {
        value: 'nonwritable property',
        writable: false,
        enumerable: false,
        configurable: false
    });
    var child = {};
    Object.setPrototypeOf(child, parent);
    try {
        /* Creating an own property "prop" on the child is not allowed
         * because parent.prop is not writable.  This is why some ROM
         * properties are presented as writable from property attributes
         * point of view.
         */
        print(child.prop = 123);
        print('in non-strict mode the failed write is ignored silently');
    } catch (e) {
        print(e.name);
    }
    print(child.prop);

    /* Even though ROM properties (like Date.prototype.getMonth) appear
     * writable, they can't be overwritten directly.
     */
    try {
        Date.prototype.getMonth = function fake() {};
        print('in non-strict mode the failed write is ignored silently');
    } catch (e) {
        print(e.name);
    }

    /* But it's possible to inherit and overwrite in the child. */
    var child = {};
    Object.setPrototypeOf(child, RegExp.prototype);
    print(typeof child.exec);
    child.exec = 123;
    print(typeof child.exec);

    /* Array.prototype.length is technically writable too. */
    print(Duktape.enc('jx', Object.getOwnPropertyDescriptor(Array.prototype, 'length')));

    /* Can't assign to Array.prototype indices nor its length. */
    try {
        Array.prototype[0] = 1;
        print('in non-strict mode the failed write is ignored silently');
    } catch (e) {
        print(e.name);
    }
    print('Array.prototype[0]:', Array.prototype[0]);
    try {
        Array.prototype.length = 0;
        print('in non-strict mode the failed write is ignored silently');
    } catch (e) {
        print(e.name);
    }
    print('Array.prototype.length:', Array.prototype.length);
}

/*===
--- property write
function
123
function
number
boolean
===*/

function romObjectPropertyWriteTest() {
    // duk_hobject_putprop()

    /* Trying to overwrite a plain property fails silently. */
    try {
        print(typeof Date.prototype.getYear);
        print(Date.prototype.getYear = 123);
    } catch (e) {
        print(e.name);
    }
    print(typeof Date.prototype.getYear);

    /* Trying to write to a setter is allowed and captured by the setter.
     * The built-in objects don't have many setters, but Error.prototype
     * has a few which we can use to test.
     */
    try {
        var err = new Error('aiee');
        print(typeof err.lineNumber);
        err.lineNumber = true;  // setter captures and operates on 'err'
    } catch (e) {
        print(e.name);
    }
    print(typeof err.lineNumber);
}

/*===
--- property delete
true
still here
false
false
still here
true
===*/

function romObjectPropertyDeleteTest() {
    // duk_hobject_delprop_raw()

    // Attempt to delete a non-existent property succeeds as normal.
    try {
        print(delete RegExp.prototype.nonExistent);
        print('still here');
    } catch (e) {
        print(e.name);
    }
    print('nonExistent' in RegExp.prototype);

    // Attempt to delete a ROM object property fails.
    try {
        print(delete Date.prototype.getSeconds);
        print('still here');
    } catch (e) {
        print(e.name);
    }
    print('getSeconds' in Date.prototype);
}

/*===
--- defineProperty
{value:{_func:true},writable:true,enumerable:false,configurable:false}
still here
still here
TypeError
TypeError
TypeError
TypeError
TypeError
===*/

function romObjectDefinePropertyTest() {
    var old;
    var pd;

    pd = Object.getOwnPropertyDescriptor(Date.prototype, 'getMinutes');
    print(Duktape.enc('jx', pd));
    old = pd.value;

    // Trying to set current property values works and causes no errors.
    try {
        Object.defineProperty(Date.prototype, 'getMinutes', pd);
        print('still here');
    } catch (e) {
        print(e.name);
    }

    // Trying to set current property values works and causes no errors,
    // here with partial descriptor.
    try {
        Object.defineProperty(Date.prototype, 'getMinutes', { value: old });
        print('still here');
    } catch (e) {
        print(e.name);
    }

    // Trying to set a different value fails.
    try {
        Object.defineProperty(Date.prototype, 'getMinutes', { value: 123 });
        print('still here');
    } catch (e) {
        print(e.name);
    }
    try {
        Object.defineProperty(Date.prototype, 'getMinutes', { writable: false });
        print('still here');
    } catch (e) {
        print(e.name);
    }
    try {
        Object.defineProperty(Date.prototype, 'getMinutes', { enumerable: true });
        print('still here');
    } catch (e) {
        print(e.name);
    }
    try {
        Object.defineProperty(Date.prototype, 'getMinutes', { configurable: true });
        print('still here');
    } catch (e) {
        print(e.name);
    }

    // Attempt to change descriptor type fails.
    try {
        Object.defineProperty(Date.prototype, 'getMinutes', { set: function() {}, get: function() {} });
        print('still here');
    } catch (e) {
        print(e.name);
    }
}

/*===
--- setPrototypeOf
Math extensible: false
TypeError
===*/

function romObjectSetPrototypeOfTest() {
    // Object.setPrototypeOf() for a read-only object causes a type error.
    // There's no explicit check, but the built-ins are not extensible which
    // prevents setPrototypeOf.
    try {
        print('Math extensible: ' + Object.isExtensible(Math));
        Object.setPrototypeOf(Math, {});
    } catch (e) {
        print(e.name);
    }
}

/*===
--- seal/freeze
TypeError
TypeError
===*/

function romObjectSealFreezeTest() {
    // duk_hobject_seal_freeze_helper()

    // Object.seal() is ignored with no memory violation.
    try {
        print(Object.prototype.toString.call(Object.seal(Math)));
    } catch (e) {
        print(e.name);
    }

    // Object.freeze() is ignored with no memory violation.
    try {
        print(Object.prototype.toString.call(Object.freeze(Math)));
    } catch (e) {
        print(e.name);
    }
}

/*===
--- compact
[object Math]
true
===*/

function romObjectCompactTest() {
    // duk_hobject_compact_props(): Duktape.compact()

    // Compaction of a read-only object is ignored with no memory violation.
    // Returned value must still be the argument.
    print(Object.prototype.toString.call(Duktape.compact(Math)));
    print(Math === Duktape.compact(Math));
}

/*===
--- Date toGMTString/toUTCString
true
===*/

function romObjectDateGmtUtcStringTest() {
    // These two references must point to the same built-in function
    // object (not just logically same).
    print(Date.prototype.toGMTString === Date.prototype.toUTCString);
}

/*===
--- Boolean prototype _Value
false
===*/

function romObjectBooleanInternalValueTest() {
    var pfx = Duktape.dec('hex', 'ff');
    var key = pfx + 'Value';

    print(Duktape.enc('jx', Boolean.prototype[key]));
}

/*===
--- RegExp prototype matches empty string
"\x00\x02\x0b\x00\x0b\x01\x01"
true
[""]
===*/

function romObjectRegExpTest() {
    var pfx = Duktape.dec('hex', 'ff');
    var key = pfx + 'Bytecode';

    print(Duktape.enc('jx', RegExp.prototype[key]));
    print(Duktape.enc('jx', RegExp.prototype.test('')));
    print(Duktape.enc('jx', RegExp.prototype.exec('')));
}

/*===
--- Error.prototype setter/getter
fileName
{get:{_func:true},set:{_func:true},enumerable:false,configurable:false}
get length: 0, set length: 0
lineNumber
{get:{_func:true},set:{_func:true},enumerable:false,configurable:false}
get length: 0, set length: 0
stack
{get:{_func:true},set:{_func:true},enumerable:false,configurable:false}
get length: 0, set length: 0
===*/

function romObjectErrorPrototypeAccessorTest() {
    function test(key) {
        print(key);
        try {
            var pd = Object.getOwnPropertyDescriptor(Error.prototype, key);
            print(Duktape.enc('jx', pd));
            print('get length: ' + pd.get.length + ', set length: ' + pd.set.length);
        } catch (e) {
            print(typeof e, typeof e.message, typeof e.name, e.message, e.name);
            print(e instanceof Error);
            print(e.stack || e);
        }
    }

    test('fileName');
    test('lineNumber');
    test('stack');
}

/*===
--- Accessor test
true
===*/

function romObjectAccessorTest() {
    // Development time issue: accessors were mapped incorrectly.
    // Test that __proto__ result matches Object.getPrototypeOf().
    var global = new Function('return this;')();
    print(global.__proto__ == Object.getPrototypeOf(global));
}

/*===
--- Array.prototype test
[object Array]
[object Array]
0
===*/

function romObjectArrayPrototypeTest() {
    // Array.prototype is also an Array instance; this affects the
    // ROM init data and is thus useful to check.
    print(Object.prototype.toString.call([]));
    print(Object.prototype.toString.call(Array.prototype));
    print(Array.prototype.length);
}

// Read-only code paths related to object properties which aren't covered:
//
// duk_hobject_props.c:duk_realloc_props(): assert, can't be exercised directly.
// duk_hobject_props.c:duk__putprop_shallow_fastpath_array_tval(): assert, can't be exercised directly.
// duk_hobject_props.c:duk__putprop_fastpath_bufobj_tval(): assert, can't be exercised directly.

try {
    print('--- extensibility');
    romObjectExtensibleTest();

    print('--- global object');
    romObjectGlobalObjectTest();

    print('--- property attributes');
    romObjectPropertyAttributeTest();

    print('--- property write');
    romObjectPropertyWriteTest();

    print('--- property delete');
    romObjectPropertyDeleteTest();

    print('--- defineProperty');
    romObjectDefinePropertyTest();

    print('--- setPrototypeOf');
    romObjectSetPrototypeOfTest();

    print('--- seal/freeze');
    romObjectSealFreezeTest();

    print('--- compact');
    romObjectCompactTest();

    print('--- Date toGMTString/toUTCString');
    romObjectDateGmtUtcStringTest();

    print('--- Boolean prototype _Value');
    romObjectBooleanInternalValueTest();

    print('--- RegExp prototype matches empty string');
    romObjectRegExpTest();

    print('--- Error.prototype setter/getter');
    romObjectErrorPrototypeAccessorTest();

    print('--- Accessor test')
    romObjectAccessorTest();

    print('--- Array.prototype test');
    romObjectArrayPrototypeTest();
} catch (e) {
    print(e.stack || e);
}
