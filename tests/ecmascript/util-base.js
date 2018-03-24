/*
 *  Basic test utilities in a global 'Test' binding.
 */

(function initTest() {
    var GLOBAL = new Function('return this')();
    var Test = GLOBAL.Test;
    if (typeof Test !== 'object') {
        Test = {};
        Object.defineProperty(GLOBAL, 'Test', {
            value: Test, writable: false, enumerable: false, configurable: false
        });
    }

    // Summarize any value to a printable string, avoiding side effects where
    // possible, preserving details like zero sign.
    function valueToString(x) {
        if (x === void 0) {
            return 'undefined';
        } else if (x === null) {
            return 'null';
        } else if (typeof x === 'boolean') {
            return String(x);
        } else if (typeof x === 'number') {
            if (x !== 0) {
                return String(x);
            } else {
                return (1 / x > 0) ? '0' : '-0';
            }
        } else if (typeof x === 'string') {
            return JSON.stringify(x);
        } else if (typeof x === 'symbol') {
            return String(x);
        } else if (typeof x === 'object') {
            return Object.prototype.toString.call(x);
        } else if (typeof x === 'function') {
            var pd = Object.getOwnPropertyDescriptor(x, 'name');
            if (pd && typeof pd.value === 'string') {
                return 'function:' + pd.value;
            } else {
                return 'function';
            }
        } else if (typeof x === 'pointer') {
            if (String(x) === 'null') {
                return 'pointer:null';
            } else {
                return 'pointer:non-null';  // specific pointer is very rarely interesting for tests
            }
        } else if (typeof x === 'buffer') {
            // No longer used in Duktape 2.x.
            return 'buffer:' + x.length;
        } else {
            return 'unknown:' + String(x);
        }
    }

    // Get property descriptor string for arbitrary object and key.
    function getPropDescString(obj, key) {
        var strkey = String(key);  // handle symbols
        if (obj === void 0) {
            return 'propdesc ' + strkey + ': undefined object';
        } else if (obj === null) {
            return 'propdesc ' + strkey + ': null object';
        } else {
            try {
                var pd = Object.getOwnPropertyDescriptor(obj, key);
                var parts = [];
                if (pd) {
                    if (typeof pd.value !== 'undefined') {
                        parts.push('value=' + valueToString(pd.value));
                    }
                    if (typeof pd.writable !== 'undefined') {
                        parts.push('writable=' + valueToString(pd.writable));
                    }
                    if (typeof pd.enumerable !== 'undefined') {
                        parts.push('enumerable=' + valueToString(pd.enumerable));
                    }
                    if (typeof pd.configurable !== 'undefined') {
                        parts.push('configurable=' + valueToString(pd.configurable));
                    }
                    if (typeof pd.get !== 'undefined') {
                        parts.push('get=' + valueToString(pd.get));
                    }
                    if (typeof pd.set !== 'undefined') {
                        parts.push('set=' + valueToString(pd.set));
                    }
                    return 'propdesc ' + strkey + ': ' + parts.join(', ');
                } else {
                    return 'propdesc ' + strkey + ': no such property';
                }
            } catch (e) {
                return 'propdesc ' + strkey + ': ' + String(e);
            }
        }
    }

    // Permute array.
    function permuteArray(arr) {
        var res = [];
        var len = arr.length;
        for (var i = 0; i < len; i++) {
            var idx = Math.floor(Math.random() * arr.length);
            res.push(arr.splice(idx, 1)[0]);
        }
        return res;
    }

    Test.valueToString = valueToString;
    Test.getPropDescString = getPropDescString;
    Test.permuteArray = permuteArray;
}());
