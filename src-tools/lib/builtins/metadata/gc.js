/*
 *  Garbage collect built-in objects and strings based on reachability roots.
 */

'use strict';

const {
    walkObjects,
    walkStrings,
    walkObjectProperties,
    walkObjectsAndProperties,
    propDefault
} = require('./util');
const { createBareObject } = require('../../util/bare');

// Mark objects with 'bidx' forcibly reachable.
function markBidxObjectsReachable(meta) {
    walkObjects(meta, (o) => {
        if (propDefault(o, 'bidx_used', false)) {
            o._force_reachable = 'bidx_used';
        }
    });
}
exports.markBidxObjectsReachable = markBidxObjectsReachable;

// Mark strings with actively referenced 'stridx' forcibly reachable.
function markStridxStringsReachable(meta) {
    walkStrings(meta, (s) => {
        if (s.stridx_used) {
            s._force_reachable = 'stridx';
        }
    });
}
exports.markStridxStringsReachable = markStridxStringsReachable;

// Delete objects and strings not reachable from reachability roots or
// forced to be reachable.  Such objects can't be reached at runtime
// so they're useless in RAM or ROM init data.
function removeUnreachableObjectsAndStrings(meta) {
    var reachable = createBareObject({});

    // First prune objects: keep only reachable and forced objects.

    walkObjects(meta, (o) => {
        if (propDefault(o, '_force_reachable', false)) {
            reachable[o.id] = true;
        }
    });

    function markId(objId) {
        if (objId) {
            reachable[objId] = true;
        }
    }

    // Keep marking until steady state.
    console.debug('original object count: ' + meta.objects.length);
    for (;;) {
        let reachableCount = Object.keys(reachable).length;

        walkObjects(meta, (o) => {
            if (!reachable[o.id]) {
                return;
            }
            markId(o.internal_prototype);
            walkObjectProperties(o, (p) => {
                // Shorthand has been normalized so no need
                // to support it here.
                let v = p.value;
                if (typeof v === 'object' && v !== null) {
                    switch (v.type) {
                    case 'object':
                        markId(v.id);
                        break;
                    case 'accessor':
                        markId(v.getter_id);
                        markId(v.setter_id);
                        break;
                    }
                }
            });
        });

        let newReachableCount = Object.keys(reachable).length;
        console.debug('mark reachable, reachable count ' + reachableCount + ' -> ' + newReachableCount);

        if (reachableCount === newReachableCount) {
            break;
        }
    }

    let numDeleted = 0;
    meta.objects = meta.objects.filter((o) => {
        if (reachable[o.id]) {
            return true;
        } else {
            console.debug('object ' + o.id + ' not reachable, dropping: ' + JSON.stringify(o));
            numDeleted++;
            return false;
        }
    });

    // Then prune strings: keep only reachable and forced strings.
    // (This is not very relevant for RAM initdata because only
    // strings with stridx are ultimately kept.)

    var reachableStrings = {};
    var numDeletedStrings = 0;
    walkObjectsAndProperties(meta, null, (p, o) => {
        void o;
        reachableStrings[p.key] = true;
        if (typeof p.value === 'string') {
            reachableStrings[p.value] = true;
        }
    });
    meta.strings = meta.strings.filter((s) => {
        if (reachableStrings[s.str]) {
            return true;
        } else if (s._force_reachable) {
            console.debug('string not reachable but forced, keep:', s.str);
            return true;
        } else {
            console.debug('string not reachable, drop:', s.str);
            numDeletedStrings++;
            return false;
        }
    });

    if (numDeleted > 0 || numDeletedStrings > 0) {
        console.log('deleted ' + numDeleted + ' unreachable objects, ' +
                    numDeletedStrings + ' unreachable strings');
    }
}
exports.removeUnreachableObjectsAndStrings = removeUnreachableObjectsAndStrings;
