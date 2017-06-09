/*
 *  ES2017 Annex B __lookupSetter__
 */

/*===
ownSetter
inheritedSetter
undefined
undefined
undefined
undefined
1
__lookupSetter__
===*/

function testLookupSetter() {
    var obj = {};

    // Setter lookup traverses internal prototype chain.
    Object.defineProperty(Object.prototype, 'inherited', {
        set: function inheritedSetter() {}
    });
    Object.defineProperty(obj, 'own', {
        set: function ownSetter() {}
    });
    print(obj.__lookupSetter__('own').name);
    print(obj.__lookupSetter__('inherited').name);

    // An accessor lacking a 'get' masks an inherited setter.
    Object.defineProperty(Object.prototype, 'masking', {
        set: function inheritedSetter() {}
    });
    Object.defineProperty(obj, 'masking', {
        get: function ownGetter() {}
        // no setter
    });
    print(obj.__lookupSetter__('masking'));

    // A data property causes undefined to be returned.
    Object.defineProperty(Object.prototype, 'inheritedData', {
        value: 123
    });
    Object.defineProperty(obj, 'ownData', {
        value: 321
    });
    print(obj.__lookupSetter__('ownData'));
    print(obj.__lookupSetter__('inheritedData'));

    // Same for missing property.
    print(obj.__lookupSetter__('noSuch'));

    // .length and .name
    print(Object.prototype.__lookupSetter__.length);
    print(Object.prototype.__lookupSetter__.name);
}

try {
    testLookupSetter();
} catch (e) {
    print(e.stack || e);
}
