/*
 *  Normalize metadata shorthands which are stateless and should always be
 *  equivalent to the canonical form.
 */

'use strict';

const { validateStringsAreBstrRecursive } = require('../../util/bstr');
const {
    walkObjects,
    walkObjectsAndProperties,
    walkStrings,
    findPropertyByKey,
    parseAttributeString,
    propDefault
} = require('./util');
const { formatSymbol } = require('../symbol');
const { createBareObject } = require('../../util/bare');
const { assert } = require('../../util/assert');
const { jsonDeepClone } = require('../../util/clone');

// Counter for creating unique sub-object IDs across multiple metadata files.
var subObjCounter = 0;

// Add missing object, string, and properties lists, just to make it
// easier to work with the files.
function addMissingLists(meta) {
    meta.objects = meta.objects || [];
    meta.strings = meta.strings || [];
    for (let o of meta.objects) {
        o.properties = o.properties || [];
    }
}

// Replace Symbol keys with encoded plain strings in internal representation.
function normalizeSymbolStrings(meta) {
    function fmt(x) {
        if (typeof x === 'object' && x !== null && x.type === 'symbol') {
            return formatSymbol(x);
        } else {
            return x;
        }
    }

    walkStrings(meta, (s) => {
        s.str = fmt(s.str);
    });
    walkObjectsAndProperties(meta, null, (p, o) => {
        void o;
        p.key = fmt(p.key);
        p.value = fmt(p.value);
    });
    (meta.add_forced_strings || []).forEach((s) => {
        s.str = fmt(s.str);
    });
}

// Merge duplicate strings in string metadata.
function mergeStringEntries(meta) {
    // The raw string list contains duplicates so merge entries.
    // The list is processed in reverse because the last entry should
    // "win" and keep its place (this matters for reserved words).

    var strs = [];
    var strMap = createBareObject({});   // plain string -> object in strs[]
    var numDuplicates = 0;

    for (let i = meta.strings.length - 1; i >= 0; i--) {
        let s = meta.strings[i];
        let prev = strMap[s.str];
        if (prev) {
            console.debug('duplicate string found, merging: ' + prev.str);
            // Must compare exactly the same.
            for (let k in Object.keys(s).sort()) {
                if (prev[k] !== s[k]) {
                    throw new TypeError('fail to merge string entry, conflicting keys; string ' + s.str + ', key ' + k);
                }
            }
            for (let k in Object.keys(prev).sort()) {
                if (prev[k] !== s[k]) {
                    throw new TypeError('fail to merge string entry, conflicting keys; string ' + s.str + ', key ' + k);
                }
            }
            numDuplicates++;
        } else {
            strs.push(s);  // reverse order at this point
            strMap[s.str] = s;
        }
    }
    strs.reverse();

    console.debug('removed ' + numDuplicates + ' duplicate strings');

    meta.strings = strs;
}

// Default 'nargs' from 'length' for top level function objects and
// function property shorthand.
function normalizeNargsLength(meta) {
    walkObjects(meta, (o) => {
        if (!(typeof o.nargs === 'undefined' && o.callable)) {
            return;
        }
        let lenProp = findPropertyByKey(o, 'length');
        if (!lenProp) {
            return;
        }
        if (typeof lenProp.value !== 'number') {
            throw new TypeError('length value must be an integer');
        }
        o.nargs = lenProp.value;
    });

    // XXX: We could decode shorthand first so that we wouldn't need to support it here.
    walkObjectsAndProperties(meta, null, (p, o) => {
        void o;
        if (!(typeof p.value === 'object' && p.value !== null && p.value.type === 'function')) {
            return;
        }
        let pval = p.value;
        if (typeof pval.length === 'undefined') {
            pval.length = 0;
        }
        if (typeof pval.nargs === 'undefined') {
            pval.nargs = pval.length;
        }
    });
}

// Normalize property attribute order, default attributes, etc.
function normalizePropertyAttributes(meta) {
    walkObjectsAndProperties(meta, (o) => {
        void o;
        /* nop */
    }, (p, o) => {
        let origAttrs = p.attributes;
        let isAccessor = (typeof p.value === 'object' && p.value !== null && p.value.type === 'accessor');

        // If missing, set default attributes.
        let attrs = origAttrs;
        if (typeof attrs === 'undefined') {
            attrs = (isAccessor ? 'ca' : 'wc');  // accessor: configurable; data: writable+configurable
        }

        // Decode flags to normalize their order in the end.
        let { writable, enumerable, configurable, accessor } = parseAttributeString(attrs);

        // Force 'accessor' attribute for accessors.
        if (isAccessor && !accessor) {
            accessor = true;
        }

        // Normalize order and write back.
        attrs = (writable ? 'w' : '') +
                (enumerable ? 'e' : '') +
                (configurable ? 'c' : '') +
                (accessor ? 'a' : '');
        p.attributes = attrs;

        if (origAttrs !== attrs) {
            console.debug('updated attributes of ' + o.id + '/' + p.key + ' from ' + origAttrs + ' ' + attrs);
        }
    });
}

// Normalize metadata property shorthand.  For example, if a property value
// is a shorthand function, create a function object and change the property
// to point to that function object.
function normalizePropertyShorthand(meta) {
    // New objects created based on property shorthands.
    var subObjs = [];

    // Create a new blank object with empty property list.
    function getSubObject() {
        var obj = {
            id: 'subobj_' + (subObjCounter++),
            properties: [],
            auto_generated: true
        }
        subObjs.push(obj);
        return obj;
    }

    // Convert the built-in function property shorthand into an actual object.
    function decodeFunctionShorthand(funProp) {
        assert(funProp.value.type === 'function');
        let val = funProp.value;
        let obj = getSubObject();
        let props = obj.properties;
        Object.assign(obj, {
            native: val.native,
            nargs: propDefault(val, 'nargs', val.length),
            varargs: propDefault(val, 'varargs', false),
            magic: propDefault(val, 'magic', 0),
            internal_prototype: 'bi_function_prototype',
            class: 'Function',
            callable: propDefault(val, 'callable', true),
            constructable: propDefault(val, 'constructable', false),
            special_call: propDefault(val, 'special_call', false)
        });
        let funName = (typeof val.name !== 'undefined' ? val.name : funProp.key);
        props.push({ key: 'length', value: val.length, attributes: 'c' });  // Configurable in ES2015
        props.push({ key: 'name', value: funName, attributes: 'c' });  // Configurable in ES2015
        return obj;
    }

    // Create a new accessor function object.
    function createAccessor(funProp, magic, nargs, length, name, nativeFunc) {
        assert(funProp.value.type === 'accessor');
        let val = funProp.value;
        let obj = getSubObject();
        let props = obj.properties;
        Object.assign(obj, {
            native: nativeFunc,
            nargs: nargs,
            varargs: false,
            magic: magic,
            internal_prototype: 'bi_function_prototype',
            class: 'Function',
            callable: propDefault(val, 'callable', true),
            constructable: propDefault(val, 'constructable', false)
        });
        assert(propDefault(obj, 'special_call', false) === false);

        // Shorthand accessors are minimal and have no .length or .name
        // right now.  Use longhand if these matter.
        //props.push({ key: 'length', value: length, attributes: 'c' });
        //props.push({ key: 'name', value: name, attributes: 'c' });
        void props;

        return obj;
    }

    // Decode getter shorthand and return the function object.
    function decodeGetterShorthand(key, funProp) {
        assert(funProp.value.type === 'accessor');
        let val = funProp.value;
        if (typeof val.getter === 'undefined') {
            return;
        }
        return createAccessor(funProp, val.getter_magic, val.getter_nargs, propDefault(val, 'getter_length', 0), key, val.getter);
    }

    // Decode setter shorthand and return the function object.
    function decodeSetterShorthand(key, funProp) {
        assert(funProp.value.type === 'accessor');
        let val = funProp.value;
        if (typeof val.setter === 'undefined') {
            return;
        }
        return createAccessor(funProp, val.setter_magic, val.setter_nargs, propDefault(val, 'setter_length', 0), key, val.setter);
    }

    // Decode a plain structured value (JSON-like value) into objects.
    function decodeStructuredValue(val) {
        if (typeof val === 'number') {
            return val;
        } else if (typeof val === 'string') {
            return val;
        } else if (typeof val === 'object' && val !== null && Array.isArray(val)) {
            let obj = decodeStructuredArray(val);
            return { type: 'object', id: obj.id };
        } else if (typeof val === 'object' && val !== null) {
            let obj = decodeStructuredObject(val);
            return { type: 'object', id: obj.id };
        } else {
            throw new TypeError('unsupported value in structured shorthand');
        }
    }

    function decodeStructuredObject(val) {
        // Original Python implementation could not preserve dict order
        // from YAML source, so sorted order was used to make the result
        // deterministic.  Keep this behavior for now.  User can always
        // use longhand for exact property control.

        let obj = getSubObject();
        Object.assign(obj, {
            class: 'Object',
            internal_prototype: 'bi_object_prototype'
        });

        let props = obj.properties;
        for (let k of Object.keys(val).sort()) {
            let prop = { key: k, value: decodeStructuredValue(val[k]), attributes: 'wec' };
            props.push(prop);
        }

        return obj;
    }

    function decodeStructuredArray(val) {
        let obj = getSubObject();
        Object.assign(obj, {
            class: 'Object',
            internal_prototype: 'bi_array_prototype'
        });

        void val;
        throw new TypeError('shorthand arrays not supported');  // XXX: add structured array support
    }

    // Decode structured JSON-like values into objects.
    function decodeStructuredShorthand(structProp) {
        assert(structProp.value.type === 'structured');
        return decodeStructuredValue(structProp.value.value);
    }

    // Create a new property based on an existing one, copying common keys
    // but omitting e.g. value.
    function clonePropShared(prop) {
        let res = jsonDeepClone(prop);
        delete res.value;
        return res;
    }

    // Walk objects and properties, normalizing shorthands.
    walkObjects(meta, (obj) => {
        let replProps = [];

        for (let prop of obj.properties) {
            if (prop.value === void 0) {
                replProps.push(prop);
            } else if (prop.value === null) {
                replProps.push(prop);
            } else if (typeof prop.value === 'boolean') {
                replProps.push(prop);
            } else if (typeof prop.value === 'number') {
                replProps.push(prop);
            } else if (typeof prop.value === 'object' && prop.value !== null && prop.value.type === 'function') {
                let subFun = decodeFunctionShorthand(prop);
                let newProp = clonePropShared(prop);
                newProp.value = { type: 'object', id: subFun['id'] };
                replProps.push(newProp);
            } else if (typeof prop.value === 'object' && prop.value !== null && prop.value.type === 'accessor' &&
               ('getter' in prop.value || 'setter' in prop.value)) {
                // Accessor normal and shorthand forms both use the type 'accessor',
                // but are differentiated by properties.
                let subGetter = decodeGetterShorthand(prop.key, prop);
                let subSetter = decodeSetterShorthand(prop.key, prop);
                let newProp = clonePropShared(prop);
                newProp.value = { 'type': 'accessor' };
                if (subGetter) {
                    newProp.value.getter_id = subGetter['id'];
                }
                if (subSetter) {
                    newProp.value.setter_id = subSetter['id'];
                }
                assert(newProp.attributes.indexOf('a') >= 0);  // If missing, weird things happen runtime.
                replProps.push(newProp);
            } else if (typeof prop.value === 'object' && prop.value !== null && prop.value.type === 'structured') {
                let subVal = decodeStructuredShorthand(prop);
                let newProp = clonePropShared(prop);
                newProp.value = subVal;
                replProps.push(newProp);
            } else if (typeof prop.value === 'object' && prop.value !== null && prop.value.type === 'buffer') {
                throw new TypeError('buffer type not yet supported for builtins');
            } else if (typeof prop.value === 'object' && prop.value !== null && prop.value.type === 'pointer') {
                throw new TypeError('pointer type not yet supported for builtins');
            } else {
                // Assume already in normalized form.
                replProps.push(prop);
            }
        }

        obj.properties = replProps;
    });

    let lenBefore = meta.objects.length;
    meta.objects = meta.objects.concat(subObjs);
    let lenAfter = meta.objects.length;

    console.debug('normalized metadata shorthand, ' + lenBefore + ' -> ' + lenAfter + ' final objects');
}

// Remove objects and properties unconditionally marked 'disable: true'.
function removeDisabled(meta) {
    var objList = [];
    var countDisabledObject = 0;
    var countDroppedProperty = 0;
    var countDisabledProperty = 0;

    for (let o of meta.objects) {
        let objDropped = false;
        if (o.disable === true) {
            console.debug('removed disabled object: ' + o.id);
            countDisabledObject++;
            objDropped = true;
        } else {
            objList.push(o);
        }

        let propList = [];
        for (let p of o.properties) {
            if (objDropped) {
                console.debug('removed dropped property (owning object dropped): ' + p.key + ', object: ' + o.id);
                countDroppedProperty++;
            } else if (p.disable === true) {
                console.debug('removed disabled property: ' + p.key + ', object: ' + o.id);
                countDisabledProperty++;
            } else {
                propList.push(p);
            }
        }
        o.properties = propList;
    }

    meta.objects = objList;

    let totalCount = countDisabledObject + countDroppedProperty + countDisabledProperty;
    if (totalCount > 0) {
        console.debug('removed ' + countDisabledObject + ' disabled objects, ' +
                      countDroppedProperty + ' dropped properties (owning object dropped), ' +
                      countDisabledProperty + ' disabled properties');
    }
}

function normalizeMetadata(meta) {
    addMissingLists(meta);
    validateStringsAreBstrRecursive(meta);
    removeDisabled(meta);
    normalizeSymbolStrings(meta);
    mergeStringEntries(meta);
    normalizeNargsLength(meta);
    normalizePropertyAttributes(meta);
    normalizePropertyShorthand(meta);
    validateStringsAreBstrRecursive(meta);
}
exports.normalizeMetadata = normalizeMetadata;
