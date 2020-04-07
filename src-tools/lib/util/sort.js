'use strict';

// Numeric sort of object keys (converted from string to number), to sort
// sparse array keys.
function numericSort(arr) {
    Array.prototype.sort.call(arr, (a, b) => {
        let an = Number(a);
        let bn = Number(b);
        if (an > bn) { return 1; }
        if (an < bn) { return -1; }
        return 0;
    });
    return arr;
}
exports.numericSort = numericSort;

// Comparison function for numbers (no coercion).
function numberCompare(a, b) {
    if (a > b) { return 1; }
    if (a < b) { return -1; }
    return 0;
}
exports.numberCompare = numberCompare;

// Sort object (own) keys (flat).
function sortObjectKeysFlat(obj) {
    if (typeof obj === 'object' && obj !== null) {
        if (Array.isArray(obj)) {
            return obj;
        } else {
            let res = {};
            Object.setPrototypeOf(res, Object.getPrototypeOf(obj));
            for (let k of Object.getOwnPropertyNames(obj).sort()) {
                res[k] = obj[k];
            }
            return res;
        }
    }
    return obj;
}
exports.sortObjectKeysFlat = sortObjectKeysFlat;

// Sort object (own) keys recursively.
function sortObjectKeysRecursive(root) {
    function process(obj) {
        if (typeof obj === 'object' && obj !== null) {
            if (Array.isArray(obj)) {
                let res = [];
                Object.setPrototypeOf(res, Object.getPrototypeOf(obj));
                for (let i = 0; i < obj.length; i++) {
                    res[i] = process(obj[i]);
                }
                for (let k of Object.getOwnPropertyNames(obj).sort()) {
                    res[k] = process(obj[k]);  // assigns index keys again, but they're already established (and sorted in ES2015+)
                }
                return res;
            } else {
                let res = {};
                Object.setPrototypeOf(res, Object.getPrototypeOf(obj));
                for (let k of Object.getOwnPropertyNames(obj).sort()) {
                    res[k] = process(obj[k]);
                }
                return res;
            }
        }
        return obj;
    }
    return process(root);
}
exports.sortObjectKeysRecursive = sortObjectKeysRecursive;
