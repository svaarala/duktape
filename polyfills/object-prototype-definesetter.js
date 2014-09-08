/*
 *  Object.prototype.__defineSetter__ polyfill
 */

if (typeof Object.prototype.__defineSetter__ === 'undefined') {
    Object.defineProperty(Object.prototype, '__defineSetter__', {
        value: function (n, f) {
            Object.defineProperty(this, n, { enumerable: true, configurable: true, set: f });
        }, writable: true, enumerable: false, configurable: true
    });
}
