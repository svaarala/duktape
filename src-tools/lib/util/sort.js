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
