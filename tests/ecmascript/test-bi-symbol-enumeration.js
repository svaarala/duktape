/*
 *  Symbols in enumeration.
 */

/*@include util-symbol.js@*/
/*@include util-buffer.js@*/

/*===
symbol enumeration
for-in
  - ownEnumStr
  - inhEnumStr
Object.keys()
  - ownEnumStr
Object.getOwnPropertyNames()
  - ownEnumStr
  - ownNonEnumStr
Reflect.ownKeys()
  - ownEnumStr
  - ownNonEnumStr
  - Symbol(ownEnumSymGlobal)
  - Symbol(ownNonEnumSymGlobal)
  - Symbol(ownEnumSymLocal)
  - Symbol(ownNonEnumSymLocal)
Object.getOwnPropertySymbols()
  - Symbol(ownEnumSymGlobal)
  - Symbol(ownNonEnumSymGlobal)
  - Symbol(ownEnumSymLocal)
  - Symbol(ownNonEnumSymLocal)
===*/

function symbolEnumerationTest() {
    var obj = {};
    var ancestor = {};
    if (typeof Duktape === 'object') {
        var internalKey1 = bufferToStringRaw(Duktape.dec('hex', 'ff696e68456e756d53796d48696464656e'));  // _InhEnumSymHidden
        var internalKey2 = bufferToStringRaw(Duktape.dec('hex', 'ff696e684e6f6e456e756d53796d48696464656e'));  // _InhNonEnumSymHidden
        var internalKey3 = bufferToStringRaw(Duktape.dec('hex', 'ff4f776e456e756d53796d48696464656e'));  // _OwnEnumSymHidden
        var internalKey4 = bufferToStringRaw(Duktape.dec('hex', 'ff4f776e4e6f6e456e756d53796d48696464656e'));  // _OwnNonEnumSymHidden
    } else {
        // For manual testing with e.g. Node.js; output will naturally differ for these.
        var internalKey1 = 'fake1';
        var internalKey2 = 'fake2';
        var internalKey3 = 'fake3';
        var internalKey4 = 'fake4';
    }
    var k;

    // Test object: own and inherited properties, enumerable and
    // non-enumerable strings and symbols.

    Object.setPrototypeOf(obj, ancestor);

    Object.defineProperty(ancestor, 'inhEnumStr', {
        value: 'inhValue1',
        enumerable: true
    });
    Object.defineProperty(ancestor, 'inhNonEnumStr', {
        value: 'inhValue2',
        enumerable: false
    });
    Object.defineProperty(ancestor, Symbol.for('inhEnumSymGlobal'), {
        value: 'inhValue3',
        enumerable: true
    });
    Object.defineProperty(ancestor, Symbol.for('inhNonEnumSymGlobal'), {
        value: 'inhValue4',
        enumerable: false
    });
    Object.defineProperty(ancestor, Symbol('inhEnumSymLocal'), {
        value: 'inhValue5',
        enumerable: true
    });
    Object.defineProperty(ancestor, Symbol('inhNonEnumSymLocal'), {
        value: 'inhValue6',
        enumerable: false
    });
    Object.defineProperty(ancestor, internalKey1, {
        value: 'inhValue7',
        enumerable: true
    });
    Object.defineProperty(ancestor, internalKey2, {
        value: 'inhValue8',
        enumerable: false
    });

    Object.defineProperty(obj, 'ownEnumStr', {
        value: 'ownValue1',
        enumerable: true
    });
    Object.defineProperty(obj, 'ownNonEnumStr', {
        value: 'ownValue2',
        enumerable: false
    });
    Object.defineProperty(obj, Symbol.for('ownEnumSymGlobal'), {
        value: 'ownValue3',
        enumerable: true
    });
    Object.defineProperty(obj, Symbol.for('ownNonEnumSymGlobal'), {
        value: 'ownValue4',
        enumerable: false
    });
    Object.defineProperty(obj, Symbol('ownEnumSymLocal'), {
        value: 'ownValue5',
        enumerable: true
    });
    Object.defineProperty(obj, Symbol('ownNonEnumSymLocal'), {
        value: 'ownValue6',
        enumerable: false
    });
    Object.defineProperty(obj, internalKey3, {
        value: 'ownValue7',
        enumerable: true
    });
    Object.defineProperty(obj, internalKey4, {
        value: 'ownValue8',
        enumerable: false
    });

    print('for-in');
    for (k in obj) {
        print('  -', k);
    }

    print('Object.keys()');
    Object.keys(obj).forEach(function (k) {
        print('  -', k);
    });

    print('Object.getOwnPropertyNames()');
    Object.getOwnPropertyNames(obj).forEach(function (k) {
        print('  -', k);
    });

    // Duktape hidden symbols won't enumerate even with
    // Object.getOwnPropertySymbols() or Reflect.ownKeys():
    // they're intended to be hidden from script access.

    print('Reflect.ownKeys()');
    Reflect.ownKeys(obj).forEach(function (k) {
        print('  -', String(k));
    });

    print('Object.getOwnPropertySymbols()');
    Object.getOwnPropertySymbols(obj).forEach(function (k) {
        print('  -', String(k));
    });
}

try {
    print('symbol enumeration');
    symbolEnumerationTest();
} catch (e) {
    print(e.stack || e);
}
