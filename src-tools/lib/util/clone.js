'use strict';

function jsonDeepClone(v) {
    return JSON.parse(JSON.stringify(v));
}
exports.jsonDeepClone = jsonDeepClone;
