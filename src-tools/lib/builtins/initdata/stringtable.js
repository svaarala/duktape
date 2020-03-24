/*
 *  Create ROM string table for strings embedded into code section.
 */

'use strict';

const { assert } = require('../../util/assert');
const { createBareObject } = require('../../util/bare');
const { numberCompare } = require('../../util/sort');
const { isPowerOfTwo } = require('../../util/double');

// Organize ROM strings into a chained ROM string table.  The ROM string
// h_next link pointer is used for chaining just like for RAM strings but
// in a separate string table.
function createRomStringTable(strs, lookupSize, hashFunc) {
    // To avoid dealing with the different possible string hash algorithms,
    // use a much more trivial lookup key for ROM strings for now.

    assert(typeof hashFunc === 'function');
    assert(isPowerOfTwo(lookupSize));

    var romstrHash = [];  // top level table -> chains, plain strings
    while (romstrHash.length < lookupSize) {
        romstrHash.push([]);
    }

    for (let v of strs) {
        let hash = hashFunc(v);
        let idx = hash % lookupSize;
        romstrHash[idx].push(v);
    }

    var romstrNext = createBareObject({});  // plain string -> next link (also plain string)
    for (let lst of romstrHash) {
        let prev = null;
        for (let v of lst) {
            if (prev) {
                romstrNext[prev] = v;
            }
            prev = v;
        }
    }

    var chainLens = createBareObject({});
    for (let lst of romstrHash) {
        let chainLen = lst.length;
        if (typeof chainLens[chainLen] === 'undefined') {
            chainLens[chainLen] = 0;
        }
        chainLens[chainLen]++;
    }
    var chainTmp = Object.keys(chainLens).sort(numberCompare).map((len) => {
        return len + ': ' + chainLens[len];
    });
    console.debug('ROM string table chain lengths: ' + chainTmp.join(', '));

    return { romstrHash, romstrNext, romstrChainLens: chainLens };
}
exports.createRomStringTable = createRomStringTable;
