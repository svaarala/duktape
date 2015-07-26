/* Date.prototype.toJSON() is a generic function, check that it behaves
 * that way.
 *
 * E5.1 Section 15.9.5.44.
 */

var obj;

/*===
2012-01-02T03:04:05.006Z
===*/

try {
    // this ends up in toISOString(), so output is exact
    print(new Date('2012-01-02T03:04:05.006Z').toJSON());
} catch (e) {
    print(e.name);
}

/*===
TypeError
TypeError
===*/

try {
    // Step 1: ToObject(undefined) -> TypeError
    Date.prototype.toJSON.call(undefined);
} catch (e) {
    print(e.name);
}

try {
    // Step 1: ToObject(null) -> TypeError
    Date.prototype.toJSON.call(undefined);
} catch (e) {
    print(e.name);
}

/*===
valueOf()
toString()
toISOString()
ISOString
===*/

try {
    // Step 2: ToPrimitive(hint Number) -> valueOf first, toString second
    obj = {
        valueOf: function() {
            // non-primitive value -> toString() will get called
            print('valueOf()');
            return [];
        },
        toString: function() {
            print('toString()');
            return 'foo';
        },
        toISOString: function() {
            print('toISOString()');
            return 'ISOString';
        }
    };
    print(Date.prototype.toJSON.call(obj));
} catch (e) {
    print(e.name);
}

/*===
valueOf()
TypeError
===*/

try {
    // toISOString does not exist -> TypeError
    obj = {
        valueOf: function() {
            // primitive value -> toString() won't be called
            print('valueOf()');
            return 123;
        },
        toString: function() {
            print('toString()');
            return 'foo';
        }
    };
    print(Date.prototype.toJSON.call(obj));
} catch (e) {
    print(e.name);
}

/*===
valueOf()
null
valueOf()
null
valueOf()
null
valueOf()
toISOString()
ISOString
valueOf()
toISOString()
ISOString
===*/

// ToPrimitive returns a non-finite number -> returns 'null'

try {
    obj = {
        valueOf: function() {
            print('valueOf()');
            return Number.POSITIVE_INFINITY;
        }
    };
    print(Date.prototype.toJSON.call(obj));
} catch (e) {
    print(e.name);
}

try {
    obj = {
        valueOf: function() {
            print('valueOf()');
            return Number.NEGATIVE_INFINITY;
        }
    };
    print(Date.prototype.toJSON.call(obj));
} catch (e) {
    print(e.name);
}

try {
    obj = {
        valueOf: function() {
            print('valueOf()');
            return Number.NaN;
        }
    };
    print(Date.prototype.toJSON.call(obj));
} catch (e) {
    print(e.name);
}

try {
    obj = {
        valueOf: function() {
            // Note: returning a finite number which is still outside
            // Ecmascript range should NOT cause a 'null' return!
            print('valueOf()');
            return (100e6 * 86400e3) + 1e10;
        },
        toISOString: function() {
            print('toISOString()');
            return 'ISOString';
        }
    };
    print(Date.prototype.toJSON.call(obj));
} catch (e) {
    print(e.name);
}

try {
    obj = {
        valueOf: function() {
            print('valueOf()');
            return -(100e6 * 86400e3) - 1e10;
        },
        toISOString: function() {
            print('toISOString()');
            return 'ISOString';
        }
    };
    print(Date.prototype.toJSON.call(obj));
} catch (e) {
    print(e.name);
}

/*===
valueOf()
TypeError
===*/

try {
    // toISOString exists but is not callable -> TypeError
    obj = {
        valueOf: function() {
            // primitive value -> toString() won't be called
            print('valueOf()');
            return 123;
        },
        toString: function() {
            print('toString()');
            return 'foo';
        },
        toISOString: 1
    };
    print(Date.prototype.toJSON.call(obj));
} catch (e) {
    print(e.name);
}

/*===
Number.prototype.toISOString()
this binding type: object
ISOString
===*/

try {
    // toISOString gets the ToObject() coercion of the base value as
    // its 'this' binding

    Number.prototype.toISOString = function() {
        print('Number.prototype.toISOString()');
        print('this binding type: ' + typeof this);
        return 'ISOString';
    }

    print(Date.prototype.toJSON.call(123));
} catch (e) {
    print(e.name);
}

/*===
toISOString()
this matches: true
ISOString
===*/

try {
    // another 'this' binding test with an object value

    obj = {
        toISOString: function() {
            print('toISOString()');
            print('this matches: ' + (this === obj));
            return 'ISOString';
        }
    }

    print(Date.prototype.toJSON.call(obj));
} catch (e) {
    print(e.name);
}
