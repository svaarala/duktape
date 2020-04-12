// Miscellaneous polyfills needed to run with Duktape and old Node.js.
// Ideally handled by transpiling.

'use strict';

function miscPolyfills() {
    if (typeof Array.prototype[Symbol.iterator] === 'undefined') {
        Array.prototype[Symbol.iterator] = function arrayIterator() {
            var arg = this;
            var index = 0;
            return {
                next: function arrayNext() {
                    if (index >= arg.length) { return { done: true }; }
                    return { value: arg[index++], done: false };
                }
            }
        };
    }

    if (typeof Array.prototype.flatMap === 'undefined') {
        Array.prototype.flatMap = function flatMap(fn) {
            var arg = this;
            var tmp = Array.prototype.map.call(arg, fn);
            var res = [];
            tmp.forEach(function (v) {
                if (Array.isArray(v)) {
                    res = res.concat(v);
                } else {
                    res.push(v);
                }
            });
            return res;
        };
    }

    if (typeof Array.prototype.includes === 'undefined') {
        Array.prototype.includes = function includes(v) {
            var arg = this;
            for (var i = 0; i < arg.length; i++) {
                if (arg[i] === v) {
                    return true;
                }
            }
            return false;
        };
    }

    if (typeof Object.getPrototypeOf(new Uint8Array(0)).map === 'undefined') {
        Object.getPrototypeOf(new Uint8Array(0)).map = Array.prototype.map;
    }
}
exports.miscPolyfills = miscPolyfills;
