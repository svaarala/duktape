/*
 *  A lighweight function can be used as a setter/getter but it will be
 *  coerced to an ordinary function by Object.defineProperty() and
 *  Object.defineProperties().
 *
 *  For the most part this is transparent, but when the property descriptor
 *  is read back, the 'set' and 'get' properties will be ordinary functions
 *  and will not match the original values.
 */

/*---
{
    "custom": true
}
---*/

function isLightFunc(x) {
    return Duktape.info(x)[0] == 9;  // tag
}

/*===
Object.defineProperty
pd_in
typeof get: function
isLightFunc get: true
typeof set: function
isLightFunc set: true
get(=Math.min)(9,-3,11,4): -3
set(=Math.max)(9,-3,11,4): 11
pd_out
typeof get: function
isLightFunc get: false
typeof set: function
isLightFunc set: false
get(=Math.min)(9,-3,11,4): -3
set(=Math.max)(9,-3,11,4): 11
pd_in.get == pd_out.get: false
pd_in.get === pd_out.get: false
pd_in.set == pd_out.set: false
pd_in.set === pd_out.set: false
Object.defineProperties
pd_in
typeof get: function
isLightFunc get: true
typeof set: function
isLightFunc set: true
get(=Math.min)(9,-3,11,4): -3
set(=Math.max)(9,-3,11,4): 11
pd_out
typeof get: function
isLightFunc get: false
typeof set: function
isLightFunc set: false
get(=Math.min)(9,-3,11,4): -3
set(=Math.max)(9,-3,11,4): 11
pd_in.get == pd_out.get: false
pd_in.get === pd_out.get: false
pd_in.set == pd_out.set: false
pd_in.set === pd_out.set: false
===*/

function lightfuncAsAccessorTest() {
    /*
     *  Accessor (setter/getter) properties are stored in the internal
     *  property table as a pair of duk_hobject pointers.  On 32-bit
     *  platforms the property value slot is 8 bytes which fits either
     *  one duk_tval or two duk_hobject pointers.  There is no space for
     *  lightfunc flags in the property slot, and increasing the slot
     *  size for accessors would be a bad trade-off.
     *
     *  The current solution is to coerce a lightfunc into a full function
     *  when a user tries to use the lightfunc as a getter/setter.  This
     *  works transparently for the most part.  However, when the property
     *  descriptor is read back, the setter/getter is not a lightfunc and
     *  doesn't match the original argument.
     */

    var obj;
    var pd_in, pd_out;

    obj = {};
    pd_in = {
        get: Math.min,
        set: Math.max,
        enumerable: true,
        configurable: true
    };

    print('Object.defineProperty');

    print('pd_in');
    print('typeof get:', typeof pd_in.get);
    print('isLightFunc get:', isLightFunc(pd_in.get));
    print('typeof set:', typeof pd_in.set);
    print('isLightFunc set:', isLightFunc(pd_in.set));
    print('get(=Math.min)(9,-3,11,4):', pd_in.get(9, -3, 11, 4));
    print('set(=Math.max)(9,-3,11,4):', pd_in.set(9, -3, 11, 4));

    Object.defineProperty(obj, 'prop', pd_in);
    pd_out = Object.getOwnPropertyDescriptor(obj, 'prop');

    print('pd_out');
    print('typeof get:', typeof pd_out.get);
    print('isLightFunc get:', isLightFunc(pd_out.get));
    print('typeof set:', typeof pd_out.set);
    print('isLightFunc set:', isLightFunc(pd_out.set));
    print('get(=Math.min)(9,-3,11,4):', pd_out.get(9, -3, 11, 4));
    print('set(=Math.max)(9,-3,11,4):', pd_out.set(9, -3, 11, 4));

    // Never compares true: lightweight and normal functions never compare
    // as equal.
    print('pd_in.get == pd_out.get:', pd_in.get == pd_out.get);
    print('pd_in.get === pd_out.get:', pd_in.get === pd_out.get);
    print('pd_in.set == pd_out.set:', pd_in.set == pd_out.set);
    print('pd_in.set === pd_out.set:', pd_in.set === pd_out.set);

    /*
     *  Same test for Object.defineProperties() which has a different
     *  internal code path.
     */

    obj = {};
    pd_in = {
        get: Math.min,
        set: Math.max,
        enumerable: true,
        configurable: true
    };

    print('Object.defineProperties');

    print('pd_in');
    print('typeof get:', typeof pd_in.get);
    print('isLightFunc get:', isLightFunc(pd_in.get));
    print('typeof set:', typeof pd_in.set);
    print('isLightFunc set:', isLightFunc(pd_in.set));
    print('get(=Math.min)(9,-3,11,4):', pd_in.get(9, -3, 11, 4));
    print('set(=Math.max)(9,-3,11,4):', pd_in.set(9, -3, 11, 4));

    Object.defineProperties(obj, { prop: pd_in });
    pd_out = Object.getOwnPropertyDescriptor(obj, 'prop');

    print('pd_out');
    print('typeof get:', typeof pd_out.get);
    print('isLightFunc get:', isLightFunc(pd_out.get));
    print('typeof set:', typeof pd_out.set);
    print('isLightFunc set:', isLightFunc(pd_out.set));
    print('get(=Math.min)(9,-3,11,4):', pd_out.get(9, -3, 11, 4));
    print('set(=Math.max)(9,-3,11,4):', pd_out.set(9, -3, 11, 4));

    print('pd_in.get == pd_out.get:', pd_in.get == pd_out.get);
    print('pd_in.get === pd_out.get:', pd_in.get === pd_out.get);
    print('pd_in.set == pd_out.set:', pd_in.set == pd_out.set);
    print('pd_in.set === pd_out.set:', pd_in.set === pd_out.set);
}

try {
    lightfuncAsAccessorTest();
} catch (e) {
    print(e.stack || e);
}
