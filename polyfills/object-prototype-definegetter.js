/*
 *  Object.prototype.__defineGetter__ polyfill
 */

if (typeof Object.prototype.__defineGetter__ === 'undefined') {
    Object.defineProperty(Object.prototype, '__defineGetter__', {
        value: function (n, f) {
            Object.defineProperty(this, n, { enumerable: true, configurable: true, get: f });
        }, writable: true, enumerable: false, configurable: true
    });
}
