'use strict';

function jsonDeepClone(v) {
    return JSON.parse(JSON.stringify(v));
}
exports.jsonDeepClone = jsonDeepClone;

function shallowCloneArray(arr) {
    var res = [];
    for (let i = 0; i < arr.length; i++) {
        res[i] = arr[i];
    }
    return res;
}
exports.shallowCloneArray = shallowCloneArray;
