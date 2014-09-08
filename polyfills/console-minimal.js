/*
 *  Minimal console.log() polyfill
 */

if (typeof console === 'undefined') {
    Object.defineProperty(this, 'console', {
        value: {}, writable: true, enumerable: false, configurable: true
    });
}
if (typeof console.log === 'undefined') {
    Object.defineProperty(this.console, 'log', {
        value: function () {
            print(Array.prototype.join.call(arguments, ' '));
        }, writable: true, enumerable: false, configurable: true
    });
}
