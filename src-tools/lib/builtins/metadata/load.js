/*
 *  Load, merge, and normalize strings and builtins metadata.
 *
 *  Final metadata object contains merged and normalized objects and strings.
 *  These can then be used to generate RAM/ROM built-in initialization data.
 *
 *  Various useful indices and helper maps are also added to the metadata
 *  object.
 */

'use strict';

const { readFileUtf8 } = require('../../extbindings/fileio');
const { parse: parseYaml } = require('../../util/yaml');
const { assert } = require('../../util/assert');
const { createBareObject } = require('../../util/bare');
const { resolveMagic } = require('../magic');
const {
    propDefault,
    walkObjects,
    walkObjectsAndProperties,
    walkObjectProperties,
    walkStrings,
    findPropertyByKey,
    findPropertyIndexByKey,
    findPropertyAndIndexByKey,
    findObjectById,
    findObjectAndIndexById,
} = require('./util');
const { normalizeMetadata } = require('./normalize');
const { validateFinalMetadata } = require('./validate');
const { markStridxStringsReachable, markBidxObjectsReachable, removeUnreachableObjectsAndStrings } = require('./gc');
const { jsonDeepClone } = require('../../util/clone');
const { validateStringsAreBstrRecursive } = require('../../util/bstr');
const { isWholeFinite } = require('../../util/double');

// Delete dangling references to removed/missing objects.
function deleteDanglingReferencesToObject(meta, objId) {
    walkObjects(meta, (o) => {
        let newProps = [];
        o.properties.forEach((p) => {
            let v = p.value;
            let ptype = void 0;
            let delProp = false;
            if (typeof v === 'object' && v !== null) {
                ptype = p.value.type;
            }
            if (ptype === 'object' && v.id === objId) {
                delProp = true;
            }
            if (ptype === 'accessor' && v.getter_id === objId) {
                console.log('delete getter, points to deleted object ' + objId);
                delete v.getter_id;
            }
            if (ptype === 'accessor' && v.setter_id === objId) {
                console.log('delete setter, points to deleted object ' + objId);
                delete v.setter_id;
            }
            if (delProp) {
                console.log('deleted property ' + p.key + ' of object ' + o.id +
                            ', points to deleted object ' + objId);
            } else {
                newProps.push(p);
            }
        });
        o.properties = newProps;
    });
}

// Merge a user YAML file into current metadata.
function mergeMetadata(meta, newMeta) {
    // Merge objects and their properties.

    propDefault(newMeta, 'objects', []).forEach((o) => {
        assert(o.disable !== true);
        var [ targ, targIdx ] = findObjectAndIndexById(meta, o.id);
        var action;

        // Action to be taken for object.  Default is 'add', which is applied
        // to automatically created shorthand objects.  If the shorthand
        // objects are not actually needed, they are removed as orphans.
        if (typeof o.action === 'string') {
            action = o.action;
        } else if (propDefault(o, 'delete', false)) {
            action = 'delete';
        } else if (propDefault(o, 'replace', false)) {
            action = 'replace';
        } else if (propDefault(o, 'add', false)) {
            action = 'add';
        } else if (propDefault(o, 'modify', false)) {
            action = 'modify';
        } else {
            action = 'add';
        }

        if (action === 'delete') {
            console.debug('delete object ' + o.id);
            if (!targ) {
                throw new TypeError('cannot delete non-existent object ' + o.id);
            }
            void meta.objects.splice(targIdx, 1);
            deleteDanglingReferencesToObject(meta, targ.id);
            return;
        }

        if (action === 'replace') {
            console.debug('replace object ' + o.id);
            if (!targ) {
                console.log('object to be replaced does not exist, append new object');
                meta.objects.push(o);
            } else {
                meta.objects[targIdx] = o;
            }
            return;
        }

        if (action === 'add') {
            console.debug('add object ' + o.id);
            if (targ) {
                throw new TypeError('cannot add object ' + o.id + ' which already exists');
            }
            meta.objects.push(o);
            return;
        }

        assert(action === 'modify');

        if (!targ) {
            throw new TypeError('cannot modify non-existent object ' + o.id);
        }

        // Merge top level keys by copying over, except for 'properties'.
        Object.keys(o).sort().forEach((k) => {
            if (k === 'properties') {
                return;
            } else {
                targ[k] = o[k];
            }
        });

        // Handle properties.
        propDefault(o, 'properties', []).forEach((p) => {
            assert(p.disable !== true);
            var [ prop, propIdx ] = findPropertyAndIndexByKey(targ, p.key);
            if (prop) {
                if (propDefault(p, 'delete', false)) {
                    console.debug('delete property ' + p.key + ' of ' + o.id);
                    void targ.properties.splice(propIdx, 1);
                } else {
                    console.debug('replace property ' + p.key + ' of ' + o.id);
                    targ.properties[propIdx] = p;
                }
            } else {
                if (propDefault(p, 'delete', false)) {
                    console.debug('deleting property ' + p.key + ' of ' + o.id +
                                  ': does not exist, nop');
                } else {
                    console.debug('add property ' + p.key + ' of ' + o.id);
                    targ.properties.push(p);
                }
            }
        });
    });

    // Merge strings.

    var strsHave = createBareObject({});
    walkStrings(meta, (s) => {
        assert(!strsHave[s.str]);
        strsHave[s.str] = true;
    });
    (newMeta.strings || []).forEach((s) => {
        if (strsHave[s.str]) {
            /* nop */
        } else {
            strsHave[s.str] = true;
            meta.strings.push(s);
        }
    });

    (newMeta.add_forced_strings || []).forEach((s) => {
        if (strsHave[s.str]) {
            s._force_reachable = 'add_forced_strings';
        } else {
            strsHave[s.str] = true;
            s._force_reachable = 'add_forced_strings';
            s._auto_add_user = true;
            meta.strings.push(s);
        }
    });
}

// Remove objects and properties disabled in active configuration.
function removeInactive(meta, activeOpts) {
    var objList = [];
    var countUnneededObject = 0;
    var countDroppedProperty = 0;
    var countUnneededProperty = 0;

    function presentIfCheck(v) {
        let pi = v.present_if;
        if (typeof pi === 'undefined') {
            return true;
        } else if (typeof pi === 'string') {
            pi = [ pi ];
        }
        if (!Array.isArray(pi)) {
            throw new TypeError('invalid present_if syntax');
        }
        // Present if all listed options are true or unknown.
        // Absent if any option is known to be false.
        for (let opt of pi) {
            if (activeOpts[opt] === false) {
                return false;
            }
        }
        return true;
    }

    for (let o of meta.objects) {
        assert(o.disable !== true);
        let objDropped = false;
        if (!presentIfCheck(o)) {
            console.debug('removed object not needed in active configuration: ' + o.id);
            countUnneededObject++;
            objDropped = true;
        } else {
            objList.push(o);
        }

        let propList = [];
        for (let p of o.properties) {
            assert(p.disable !== true);
            if (objDropped) {
                console.debug('removed dropped property (owning object dropped): ' + p.key + ', object: ' + o.id);
                countDroppedProperty++;
            } else if (!presentIfCheck(p)) {
                console.debug('removed property not needed in active configuration: ' + p.key + ', object: ' + o.id);
                countUnneededProperty++;
            } else {
                propList.push(p);
            }
        }
        o.properties = propList;
    }

    meta.objects = objList;

    let totalCount = countUnneededObject + countDroppedProperty + countUnneededProperty;
    if (totalCount > 0) {
        console.debug('removed ' + countUnneededObject + ' unneeded objects, ' +
                      countDroppedProperty + ' dropped properties (owning object dropped), ' +
                      countUnneededProperty + ' unneeded properties');
    }
}

// Handle property value .type: 'share_property_value', useful for at least
// Date.prototype.toGMTString.
function handleSharedPropertyValues(meta) {
    walkObjects(meta, (obj) => {
        obj.properties.forEach((prop, idx) => {
            if (typeof prop.value === 'object' && prop.value !== null && prop.value.type === 'share_property_value') {
                console.debug('share property value:', obj.id, prop.key, '->', prop.value.key);
                assert(typeof prop.value.key === 'string');

                let otherProp = findPropertyByKey(obj, prop.value.key);
                assert(otherProp);

                var newProp = jsonDeepClone(otherProp);
                newProp.key = prop.key;
                obj.properties[idx] = newProp;
            }
        });
    });
}

// Date.prototype.toGMTString must point to the same Function object
// as Date.prototype.toUTCString, so special case hack it.  (Replaced
// by 'share_property_value' syntax.)
function dateToGMTStringReplacement(meta) {
    var obj = findObjectById(meta, 'bi_date_prototype');
    if (!obj) {
        return;
    }

    var gmtIndex = findPropertyIndexByKey(obj, 'toGMTString');
    var utcIndex = findPropertyIndexByKey(obj, 'toUTCString');
    //console.debug(gmtIndex, utcIndex);
    if (!(typeof gmtIndex === 'number' && typeof utcIndex === 'number')) {
        return;
    }

    console.log('clone Date.prototype.toUTCString to Date.prototype.toGMTString');
    var newProp = jsonDeepClone(obj.properties[utcIndex]);
    newProp.key = 'toGMTString';
    obj.properties[gmtIndex] = newProp;
}

// Add .name properties to functions, expected by current RAM format.
function addRamFunctionNames(meta) {
    let numAdded = 0;
    walkObjects(meta, (o) => {
        if (!o.callable) {
            return;
        }
        let nameProp = findPropertyByKey(o, 'name');

        if (typeof nameProp === 'undefined') {
            console.debug('add missing "name" property for object ' + o.id);
            numAdded++;
            o.properties.push({ key: 'name', value: '', attributes: 'c' });
        }
    });
    if (numAdded > 0) {
        console.debug('added missing "name" property for ' + numAdded + ' functions');
    }
}

// ROM properties must not be configurable (runtime code depends on this).
// Writability is kept so that instance objects can override parent properties.
function handleRomPropertyAttributes(meta) {
    walkObjectsAndProperties(meta, null, (p, o) => {
        void o;
        p.attributes = p.attributes.replace(/c/g, '');
    });
}

// Convert eligible functions to lightfuncs.
function convertLightfuncs(meta) {
    var numConverted = 0;
    var numSkipped = 0;

    walkObjectsAndProperties(meta, null, (p, o) => {
        let v = p.value;
        if (!(typeof v === 'object' && v !== null && v.type === 'object')) {
            return;
        }
        let targ = findObjectById(meta, v.id);
        if (!targ) {
            console.log('target object ' + v.id + ' not found in lightfunc conversion check, ignoring');
            return;
        }
        let reasons = [];
        if (!targ.callable) {
            reasons.push('not-callable');
        }
        //if (targ.constructable) {
        //    reasons.push('constructable');
        //}
        if (typeof targ.native !== 'string') {
            reasons.push('no-native');
        }

        // Don't convert if function has properties that we're not willing
        // to sacrifice.
        let lfLen = 0;
        let allowedKeys = createBareObject({ length: true, name: true });
        walkObjectProperties(targ, (p2) => {
            if (p2.key === 'length' && typeof p2.value === 'number') {
                // Requires length to use number shorthand at present.
                if (isWholeFinite(p2.value)) {
                    lfLen = p2.value;
                } else {
                    reasons.push('unexpected-length');
                }
            }
            if (!allowedKeys[p2.key]) {
                reasons.push('nonallowed-property');
            }
        });

        if (!propDefault(p, 'auto_lightfunc', true)) {
            reasons.push('no-auto-lightfunc');
        }

        let lfMagic = 0;
        if (typeof targ.magic !== 'undefined') {
            try {
                // Magic values which resolve to 'bidx' indices cannot
                // be resolved here yet, because the bidx map is not
                // yet ready.  If so, reject the lightfunc conversion
                // for now.  In practice this doesn't matter.
                lfMagic = resolveMagic(targ.magic, {});  // {} = fake bidx map
            } catch (e) {
                console.debug('failed to resolve magic for ' + p.key + ', skipping lightfunc conversion: ' + e);
                reasons.push('magic-resolve-failed');
                lfMagic = 0xffffffff;  // dummy out of bounds value
            }
        }

        let lfNargs, lfVarargs;
        if (propDefault(targ, 'varargs', true)) {
            lfNargs = null;
            lfVarargs = true;
        } else {
            lfNargs = propDefault(targ, 'nargs', 0);
            lfVarargs = false;
        }

        if (lfLen < 0 || lfLen > 15) {
            reasons.push('len-bounds');
        }
        if (lfMagic < -0x80 || lfMagic > 0x7f) {
            reasons.push('magic-bounds');
        }
        if (!lfVarargs && (lfNargs < 0 || lfNargs > 14)) {
            reasons.push('nargs-bounds');
        }

        if (reasons.length > 0) {
            console.debug('do not convert lightfunc ' + o.id + ' ' + p.key + ' ' + p.value.id + ': ' + reasons.join(','));
            numSkipped++;
        } else {
            // Replace value in place.  This typically leaves orphan objects
            // which are collected later.
            let oldId = p.value.id;
            p.value = {
                type: 'lightfunc',
                native: targ.native,
                length: lfLen,
                magic: lfMagic,
                nargs: lfNargs,
                varargs: lfVarargs
            }
            console.debug('convert to lightfunc ' + o.id + ' ' + p.key + ' ' + oldId);
            numConverted++;
        }
    });

    console.log('converted ' + numConverted + ' built-in function properties to lightfuncs, ' + numSkipped + ' skipped as non-eligible');
}

// Prepare a list of built-in objects which need a runtime 'bidx'.
function prepareObjectsBidx(meta) {
    var objList = meta.objects;
    meta.objects = [];
    meta.objects_bidx = [];

    // Objects have a 'bidx: true' if they need a DUK_BIDX_xxx constant
    // and need to be present in thr->builtins[].  The list is already
    // stripped of built-in objects which are not needed based on config.
    // Ideally we'd scan the actually needed indices from the source
    // but since some usage is inside #if defined()s that's not trivial.
    // We could, however, warn about bidx objects which are not referenced
    // anywhere in the source.

    for (let o of objList) {
        if (propDefault(o, 'bidx', false)) {
            o.bidx_used = true;
            meta.objects.push(o);
            meta.objects_bidx.push(o);
        }
    }

    // Append remaining objects.
    for (let o of objList) {
        if (!o.bidx_used) {
            meta.objects.push(o);
        }
    }
}

// Add C define names for builtin strings.  These defines are added to all
// strings, even when they won't get a stridx because the define names are
// used to autodetect referenced strings.
function addStringDefineNames(meta) {
    var specialDefs = meta.special_define_names;

    walkStrings(meta, (s) => {
        let v = s.str;
        if (Object.prototype.hasOwnProperty.call(specialDefs, v) && typeof specialDefs[v] !== 'undefined') {
            s.define = 'DUK_STRIDX_' + specialDefs[v];
            return;
        }
        let pfx = 'DUK_STRIDX_';
        if (v.length >= 1 && v[0] === '\x82') {
            pfx = 'DUK_STRIDX_INT_';
            v = v.substring(1);
        } else if (v.length >= 1 && v[0] === '\x81' && v[v.length - 1] === '\xff') {
            pfx = 'DUK_STRIDX_WELLKNOWN_';
            v = v.substring(1, v.length - 1);
        }
        // No support for other forms of Symbols above.
        let t = v.replace(/([a-z0-9])([A-Z])/g, (match, cap1, cap2) => cap1 + '_' + cap2);  // add underscores: aB -> a_B
        t = t.replace('.', '_');  // replace . with _, e.g. Symbol.iterator
        s.define = pfx + t.toUpperCase();
    });
}

// Order builtin strings (strings with a stridx) into an order satisfying
// multiple constraints.
function orderBuiltinStrings(meta) {
    var inputStrlist = meta.strings;
    var keywordList = meta.reserved_word_token_order;

    // Strings are ordered in the result as follows:
    //   1. Non-reserved words requiring 8-bit indices
    //   2. Non-reserved words not requiring 8-bit indices
    //   3. Reserved words in non-strict mode only
    //   4. Reserved words in strict mode
    //
    // Reserved words must follow an exact order because they are
    // translated to/from token numbers by addition/subtraction.
    // Some strings require an 8-bit index and must be in the
    // beginning.

    function deepCopy(v) {
        return JSON.parse(JSON.stringify(v));
    }

    // Strings needing stridx.

    var tmpStrs = deepCopy(inputStrlist).filter((s) => s.stridx_used);

    // The reserved word list must match token order in duk_lexer.h
    // exactly, so pluck them out first.  We'll then have two lists:
    // keywords[], strs[].

    var strIndex = createBareObject({});
    var kwIndex = createBareObject({});
    var keywords = [];
    var strs = [];
    for (let s of tmpStrs) {
        strIndex[s.str] = s;
    }
    for (let s of keywordList) {
        let v = strIndex[s];
        assert(v, 'keyword in strIndex');
        keywords.push(v);
        kwIndex[s] = true;
    }
    for (let s of tmpStrs) {
        if (!kwIndex[s.str]) {
            strs.push(s);
        }
    }

    // Sort the strings by category number; within category keep
    // previous order.

    strs.forEach((s, idx) => { s._idx = idx; });  // for ensuring stable sort

    function req8Bit(s) {
        return propDefault(s, 'class_name', false);  // currently just class names
    }
    function getCat(s) {
        var req8 = req8Bit(s);
        if (propDefault(s, 'reserved_word', false)) {
            assert(!req8);
            if (propDefault(s, 'future_reserved_word_strict', false)) {
                return 4;
            } else {
                return 3;
            }
        } else if (req8) {
            return 1;
        } else {
            return 2;
        }
    }

    function sortCmp(a, b) {
        var aCat = getCat(a);
        var bCat = getCat(b);
        if (aCat > bCat) { return 1; }
        if (aCat < bCat) { return -1; }
        if (a._idx > b._idx) { return 1; }  // These guarantee stable sort.
        if (a._idx < b._idx) { return -1; }
        return 0;
    }

    strs.sort(sortCmp);

    for (let s of strs) {
        // Remove temporary _idx properties.
        delete s._idx;
    }

    strs.forEach((s, idx) => {
        if (req8Bit(s) && idx >= 256) {
            throw new TypeError('8-bit string index not satisfied: ' + s.str);
        }
    });

    var res = strs.concat(keywords);
    meta.strings_stridx = res;
}

// Add .stridx_used for strings which need a STRIDX define and must
// be present in a runtime strings[] array.
function addStringUsedStridx(meta, usedStridxEtcMeta) {
    var defsNeeded = createBareObject({});
    var defsFound = createBareObject({});

    usedStridxEtcMeta.usedStridxDefines.forEach((s) => {
        defsNeeded[s] = true;
    });

    // strings whose define is referenced
    meta.strings.forEach((s) => {
        if (s.define !== 'undefined' && defsNeeded[s.define]) {
            s.stridx_used = true;
            defsFound[s.define] = true;
            //console.log(s);
        }
    });

    // duk_lexer.h needs all reserved words
    meta.strings.forEach((s) => {
        if (propDefault(s, 'reserved_word', false)) {
            assert(typeof s.define === 'string');
            s.stridx_used = true;
        }
    });

    // ensure all needed defines are provided
    defsFound.DUK_STRIDX_START_RESERVED = true;  // special defines provided automatically
    defsFound.DUK_STRIDX_START_STRICT_RESERVED = true;
    defsFound.DUK_STRIDX_END_RESERVED = true;
    defsFound.DUK_STRIDX_TO_TOK = true;
    for (let k of Object.keys(defsNeeded).sort()) {
        if (!defsFound[k]) {
            throw new TypeError('source code needs define ' + k + ' not provided by strings');
        }
    }
}

// Add final bidx and stridx indices.
function addFinalBidxStridxIndices(meta) {
    let bidx = 0;
    walkObjects(meta, (o) => {
        if (o.bidx_used) {
            o.bidx = bidx++;
        }
    });
    let stridx = 0;
    walkStrings(meta, (s) => {
        if (s.stridx_used) {
            s.stridx = stridx++;
        }
    });
}

// Add a built-in objects list for RAM initialization.
function addRamFilteredObjectList(meta) {
    // For RAM init data to support user objects, we need to prepare a
    // filtered top level object list, containing only those objects which
    // need a value stack index during duk_hthread_builtins.c init process.
    //
    // Objects in meta['objects'] which are covered by inline property
    // notation in the init data (this includes e.g. member functions like
    // Math.cos) must not be present.

    var objList = [];
    walkObjects(meta, (o) => {
        let keep = propDefault(o, 'bidx_used', false);
        if (o.native && typeof o.bidx === 'undefined') {
            // Handled inline by runtime init code.
        } else {
            // Top level object.
            keep = true;
        }
        if (keep) {
            objList.push(o);
        }
    });

    console.debug('filtered RAM object list: ' + meta.objects_bidx.length + ' objects with bidx, ' +
                  objList.length + ' total top level objects');

    meta.objects_ram_toplevel = objList;
}

// Sanity check: object index must match 'bidx' for all objects
// which have a runtime 'bidx'.  This is assumed by e.g. RAM
// thread init.
function bidxSanityCheck(meta) {
    meta.objects.forEach((o, idx) => {
        if (idx < meta.objects_bidx.length) {
            assert(meta.objects_bidx[idx] === meta.objects[idx]);
        }
        if (propDefault(o, 'bidx', false)) {
            assert(o.bidx === idx);
        }
    });
}

// Create some helper index properties for ROM/RAM initdata generation.
// (These should be removed and handled inline in the ROM/RAM code.)
function createHelperProperties(meta) {
    Object.assign(meta, {
        _strings_plain: [],  // ROM
        _plain_to_stridx: {},  // RAM
        _is_plain_reserved_word: {},  // ROM
        _is_plain_strict_reserved_word: {},  // ROM
        _objid_to_bidx: {},  // ROM
        _objid_to_ramidx: {}   // RAM
    });

    meta.strings.forEach((s) => {
        assert(meta._strings_plain.indexOf(s.str) < 0);
        meta._strings_plain.push(s.str);
        if (propDefault(s, 'reserved_word', false)) {
            meta._is_plain_reserved_word[s['str']] = true;  // includes also strict reserved words
        }
        if (propDefault(s, 'future_reserved_word_strict', false)) {
            meta._is_plain_strict_reserved_word[s['str']] = true;
        }
    });

    meta.strings_stridx.forEach((s, idx) => {
        assert(propDefault(s, 'stridx_used', false) === true);
        meta._plain_to_stridx[s.str] = idx;
    });

    meta.objects_bidx.forEach((o, idx) => {
        assert(propDefault(o, 'bidx_used', false) === true);
        meta._objid_to_bidx[o.id] = idx;
    });

    (meta.objects_ram_toplevel || []).forEach((o, idx) => {
        meta._objid_to_ramidx[o.id] = idx;
    });
}

// Add missing strings into strings metadata.  For example, if an object
// property key is not part of the strings list, append it there.  This
// is critical for ROM builtins because all strings referenced by a ROM
// object must also be in ROM.
function addMissingRomStrings(meta) {
    // We just need plain strings here.
    var strsHave = createBareObject({});
    walkStrings(meta, (s) => {
        strsHave[s.str] = true;
    });

    // For ROM builtins all the strings must be in the strings list,
    // so scan objects for any strings not explicitly listed in metadata.
    walkObjectsAndProperties(meta, null, (p, o) => {
        void o;
        var key = p.key;
        if (!strsHave[key]) {
            console.debug('add missing key string: ' + key);
            meta.strings.push({ str: key, _auto_add_ref: true, _force_reachable: 'rom_missing' });
            strsHave[key] = true;
        }
        if (typeof p.value === 'string') {
            if (!strsHave[p.value]) {
                console.debug('add missing value string: ' + p.value);
                meta.strings.push({ str: p.value, _auto_add_ref: true, _force_reachable: 'rom_missing' });
                strsHave[p.value] = true;
            }
        }
    });
}

// Dump some useful stats.
function dumpStats(meta, romBuild) {
    var stats = {
        countAdd: 0,
        countAddRef: 0,
        countAddUser: 0
    };
    meta.strings.forEach((s) => {
        if (propDefault(s, '_auto_add_ref', false)) {
            stats.countAddRef++;
        }
        if (propDefault(s, '_auto_add_user', false)) {
            stats.countAddUser++;
        }
    });
    stats.countAdd = stats.countAddRef + stats.countAddUser;
    console.log('prepared ' + (romBuild ? 'ROM' : 'RAM') + ' metadata: ' +
                meta.objects.length + ' objects, ' +
                meta.objects_bidx.length + ' objects with bidx, ' +
                meta.strings.length + ' strings, ' +
                meta.strings_stridx.length + ' strings with stridx, ' +
                stats.countAdd + ' strings added (' +
                stats.countAddRef + ' property key references, ' +
                stats.countAddUser + ' user strings)');
}

// Resolve magic values into final integer values.
function resolveMagicValues(meta) {
    // Build temporary objIdToBidx index.
    var objIdToBidx = createBareObject({});
    walkObjects(meta, (o) => {
        if (typeof o.bidx === 'number') {
            objIdToBidx[o.id] = o.bidx;
        }
    });
    walkObjects(meta, (o) => {
        if (o.magic !== void 0) {
            o.magic = resolveMagic(o.magic, objIdToBidx);
            assert(typeof o.magic === 'number');
        }
    });
}

// Load built-in object and string metadata, merge in user metadata files,
// and prepare metadata for either RAM or ROM build.
function loadMetadata(args) {
    var romAutoLightfunc = args.romAutoLightfunc;
    var usedStridxEtcMeta = args.usedStridxEtcMeta;
    var romBuild = args.romBuild;
    var dukVersion = args.dukVersion;
    var activeOpts = args.activeOpts;
    var stringsMetadataFilename = args.stringsMetadataFilename;
    var objectsMetadataFilename = args.objectsMetadataFilename;
    var userBuiltinFiles = args.userBuiltinFiles;

    // Load built-in strings and objects.  Merge strings and objects
    // metadata as simple top level key merge.
    var meta = createBareObject({});
    var objectsMetadata = parseYaml(readFileUtf8(objectsMetadataFilename));
    var stringsMetadata = parseYaml(readFileUtf8(stringsMetadataFilename));
    Object.assign(meta, objectsMetadata);
    Object.assign(meta, stringsMetadata);
    normalizeMetadata(meta);

    // Add user objects.
    (userBuiltinFiles || []).forEach((fn) => {
        console.log('merging user built-in metadata file ' + fn);
        let userMeta = parseYaml(readFileUtf8(fn));
        normalizeMetadata(userMeta, activeOpts);
        mergeMetadata(meta, userMeta);
    });

    // Remove objects and properties which are disabled in (known) active
    // duk_config.h.
    removeInactive(meta, activeOpts);

    // Handle shared property values like Date.prototype.toGMTString
    // once properties have been normalized.
    handleSharedPropertyValues(meta);

    // Date.prototype .toGMTString() and .toUTCString() hack.  No longer
    // needed with property value .type: 'share_property_value'.
    //dateToGMTStringReplacement(meta);
    void dateToGMTStringReplacement;

    // RAM top-level functions must have a 'name'.
    if (!romBuild) {
        addRamFunctionNames(meta);
    }

    // Add Duktape.version and Duktape.env for ROM case.
    let dukObj = findObjectById(meta, 'bi_duktape');
    if (dukObj) {
        dukObj.properties.unshift({ key: 'version', value: dukVersion, attributes: '' });
        if (romBuild) {
            // Use a fixed (quite dummy for now) Duktape.env when ROM
            // builtins are in use.  In the RAM case this is added
            // during global object initialization based on config options
            // in use.
            dukObj.properties.unshift({ key: 'env', value: 'ROM', attributes: '' });
        }
    }

    // For ROM objects, mark all properties non-configurable.
    if (romBuild) {
        handleRomPropertyAttributes(meta);
    }

    // Convert built-in function properties automatically into
    // lightfuncs if requested and function is eligible.
    if (romBuild && romAutoLightfunc) {
        convertLightfuncs(meta)
    }

    // Now we're ready to start pruning the built-ins.  First, mark all
    // built-ins with 'bidx' and actually referenced strings with 'stridx'
    // as forcibly reachable.  These are needed in all cases.
    prepareObjectsBidx(meta);
    addStringDefineNames(meta);
    addStringUsedStridx(meta, usedStridxEtcMeta);
    markStridxStringsReachable(meta);
    markBidxObjectsReachable(meta);

    // For the ROM build: add any strings referenced by built-in objects
    // into the string list (not the 'stridx' list though): all strings
    // referenced by ROM objects must also be in ROM.
    if (romBuild) {
        addMissingRomStrings(meta);
    }

    // Check for unreachable objects and strings and remove them.
    removeUnreachableObjectsAndStrings(meta);

    // Reorder built-in strings to match multiple constraints,
    // e.g. 8-bit stridx indices for some strings.
    orderBuiltinStrings(meta);

    // Add final stridx and bidx indices to metadata objects and strings.
    addFinalBidxStridxIndices(meta);

    // Resolve remaining magic values: can only be done once bidx values are
    // known because some magic values are bidx dependent.
    resolveMagicValues(meta);

    // Prepare a filtered RAM top level object list, needed for RAM init.
    if (!romBuild) {
        addRamFilteredObjectList(meta);
    }

    // Sanity check for bidx.
    bidxSanityCheck(meta);

    // Dump stats.
    dumpStats(meta, romBuild);

    // Final validation steps.
    validateStringsAreBstrRecursive(meta);
    validateFinalMetadata(meta);

    return meta;
}
exports.loadMetadata = loadMetadata;

function augmentMetadata(meta) {
    // Create a set of helper lists and maps now that the metadata is
    // in its final form.
    createHelperProperties(meta);
}
exports.augmentMetadata = augmentMetadata;
