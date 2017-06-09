/*
 *  Object.prototype.__lookupGetter__ polyfill
 */

(function () {
    if (typeof Object.prototype.__lookupGetter__ === 'undefined') {
        var DP = Object.defineProperty;
        var GPO = Object.getPrototypeOf;
        var GOPD = Object.getOwnPropertyDescriptor;
        DP(Object.prototype, '__lookupGetter__', {
            value: function (n) {
                for (var o = this; o; o = GPO(o)) {
                    var p = GOPD(o, n);
                    if (p) {
                        return p.get;
                    }
                }
            }, writable: true, enumerable: false, configurable: true
        });
    }
})();
