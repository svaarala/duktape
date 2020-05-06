/*
 *  Helpers for working with metadata.
 */

'use strict';

function propDefault(obj, key, defaultValue) {
    return (typeof obj[key] !== 'undefined' ? obj[key] : defaultValue);
}
exports.propDefault = propDefault;

// Parse attributes from string format such as 'wec'.
function parseAttributeString(attrs) {
    let writable = (attrs.indexOf('w') >= 0);
    let enumerable = (attrs.indexOf('e') >= 0);
    let configurable = (attrs.indexOf('c') >= 0);
    let accessor = (attrs.indexOf('a') >= 0);

    // Count matched flags (+true == 1, +false == 0).
    let count = (+writable) + (+enumerable) + (+configurable) + (+accessor);
    if (count !== attrs.length) {
        throw new TypeError('invalid attribute string: ' + attrs);
    }

    return { writable, enumerable, configurable, accessor };
}
exports.parseAttributeString = parseAttributeString;

function walkObjects(meta, cb) {
    for (let o of meta.objects) {
        let ret = cb(o);
        if (typeof ret !== 'undefined') {
            return ret;
        }
    }
}
exports.walkObjects = walkObjects;

function walkObjectsAndProperties(meta, cbObj, cbProp) {
    for (let o of meta.objects) {
        if (cbObj) {
            let ret = cbObj(o);
            if (typeof ret !== 'undefined') {
                return ret;
            }
        }

        for (let p of o.properties) {
            if (cbProp) {
                let ret = cbProp(p, o);
                if (typeof ret !== 'undefined') {
                    return ret;
                }
            }
        }
    }
}
exports.walkObjectsAndProperties = walkObjectsAndProperties;

function walkObjectProperties(o, cb) {
    for (let p of o.properties) {
        if (cb) {
            let ret = cb(p, o);
            if (typeof ret !== 'undefined') {
                return ret;
            }
        }
    }
}
exports.walkObjectProperties = walkObjectProperties;

function walkStrings(meta, cb) {
    for (let s of meta.strings) {
        if (cb) {
            let ret = cb(s);
            if (typeof ret !== 'undefined') {
                return ret;
            }
        }
    }
}
exports.walkStrings = walkStrings;

function findObjectAndIndexById(meta, objId) {
    for (let i = 0; i < meta.objects.length; i++) {
        let o = meta.objects[i];
        if (o.id === objId) {
            return [ o, i ];
        }
    }
    return [ void 0, void 0 ];
}
exports.findObjectAndIndexById = findObjectAndIndexById;

function findObjectById(meta, objId) {
    return findObjectAndIndexById(meta, objId)[0];
}
exports.findObjectById = findObjectById;

function findObjectIndexById(meta, objId) {
    return findObjectAndIndexById(meta, objId)[1];
}
exports.findObjectIndexById = findObjectIndexById;

function findPropertyAndIndexByKey(o, key) {
    for (let i = 0; i < o.properties.length; i++) {
        let p = o.properties[i];
        if (p.key === key) {
            return [ p, i ];
        }
    }
    return [ void 0, void 0 ];
}
exports.findPropertyAndIndexByKey = findPropertyAndIndexByKey;

function findPropertyByKey(o, key) {
    return findPropertyAndIndexByKey(o, key)[0];
}
exports.findPropertyByKey = findPropertyByKey;

function findPropertyIndexByKey(o, key) {
    return findPropertyAndIndexByKey(o, key)[1];
}
exports.findPropertyIndexByKey = findPropertyIndexByKey;
