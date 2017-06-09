/*
 *  ES2017 Annex B __lookupGetter__
 */

/*===
ownGetter
inheritedGetter
undefined
undefined
undefined
undefined
1
__lookupGetter__
===*/

function testLookupGetter() {
    var obj = {};

    // Getter lookup traverses internal prototype chain.
    Object.defineProperty(Object.prototype, 'inherited', {
        get: function inheritedGetter() {}
    });
    Object.defineProperty(obj, 'own', {
        get: function ownGetter() {}
    });
    print(obj.__lookupGetter__('own').name);
    print(obj.__lookupGetter__('inherited').name);

    // An accessor lacking a 'get' masks an inherited getter.
    Object.defineProperty(Object.prototype, 'masking', {
        get: function inheritedGetter() {}
    });
    Object.defineProperty(obj, 'masking', {
        set: function ownGetter() {}
        // no getter
    });
    print(obj.__lookupGetter__('masking'));

    // A data property causes undefined to be returned.
    Object.defineProperty(Object.prototype, 'inheritedData', {
        value: 123
    });
    Object.defineProperty(obj, 'ownData', {
        value: 321
    });
    print(obj.__lookupGetter__('ownData'));
    print(obj.__lookupGetter__('inheritedData'));

    // Same for missing property.
    print(obj.__lookupGetter__('noSuch'));

    // .length and .name
    print(Object.prototype.__lookupGetter__.length);
    print(Object.prototype.__lookupGetter__.name);
}

try {
    testLookupGetter();
} catch (e) {
    print(e.stack || e);
}
