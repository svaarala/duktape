/*
 *  Object.prototype.__lookupSetter__ polyfill
 */

(function () {
    if (typeof Object.prototype.__lookupSetter__ === 'undefined') {
        var DP = Object.defineProperty;
        var GPO = Object.getPrototypeOf;
        var GOPD = Object.getOwnPropertyDescriptor;
        DP(Object.prototype, '__lookupSetter__', {
            value: function (n, f) {
                for (var o = this; o; o = GPO(o)) {
                    var p = GOPD(o, n);
                    if (p) {
                        return p.set;
                    }
                }
            }, writable: true, enumerable: false, configurable: true
        });
    }
})();
