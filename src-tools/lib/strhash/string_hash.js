'use strict';

const { assert } = require('../util/assert');
const { bstrToUint8Array } = require('../util/bstr');

// Compute a byte hash identical to duk_util_hashbytes().
const DUK__MAGIC_M = 0x5bd1e995;
const DUK__MAGIC_R = 24;

function hashBytes(x, off, nbytes, str_seed, big_endian) {
    assert(x instanceof Uint8Array);
    var h = (str_seed ^ nbytes) & 0xffffffff;

    while (nbytes >= 4) {
        // 4-byte fetch byte order:
        //  - native (endian dependent) if unaligned accesses allowed
        //  - little endian if unaligned accesses not allowed

        let k;
        if (big_endian) {
            k = x[off + 3] + (x[off + 2] << 8) + (x[off + 1] << 16) + (x[off + 0] << 24);
        } else {
            k = x[off + 0] + (x[off + 1] << 8) + (x[off + 2] << 16) + (x[off + 3] << 24);
        }

        k = (k * DUK__MAGIC_M) & 0xffffffff;
        k = (k ^ (k >>> DUK__MAGIC_R)) & 0xffffffff;
        k = (k * DUK__MAGIC_M) & 0xffffffff;
        h = (h * DUK__MAGIC_M) & 0xffffffff;
        h = (h ^ k) & 0xffffffff;

        off += 4;
        nbytes -= 4;
    }

    if (nbytes >= 3) {
        h = (h ^ (x[off + 2] << 16)) & 0xffffffff;
    }
    if (nbytes >= 2) {
        h = (h ^ (x[off + 1] << 8)) & 0xffffffff;
    }
    if (nbytes >= 1) {
        h = (h ^ x[off]) & 0xffffffff;
        h = (h * DUK__MAGIC_M) & 0xffffffff;
    }

    h = (h ^ (h >>> 13)) & 0xffffffff;
    h = (h * DUK__MAGIC_M) & 0xffffffff;
    h = (h ^ (h >>> 15)) & 0xffffffff;

    return h >>> 0;
}
exports.hashBytes = hashBytes;

// Compute a string hash identical to duk_heap_hashstring() when dense
// hashing is enabled.
const DUK__STRHASH_SHORTSTRING = 4096;
const DUK__STRHASH_MEDIUMSTRING = 256 * 1024;
const DUK__STRHASH_BLOCKSIZE = 256;
function hashStringDense(x, hash_seed, big_endian, strhash16) {
    if (typeof x === 'string') {
        x = bstrToUint8Array(x);
    }
    assert(x instanceof Uint8Array);
    var str_seed = (hash_seed ^ x.length) & 0xffffffff;
    var res;

    if (x.length <= DUK__STRHASH_SHORTSTRING) {
        res = hashBytes(x, 0, x.length, str_seed, big_endian);
    } else {
        let skip, off;

        if (x.length <= DUK__STRHASH_MEDIUMSTRING) {
            skip = 16 * DUK__STRHASH_BLOCKSIZE + DUK__STRHASH_BLOCKSIZE;
        } else {
            skip = 256 * DUK__STRHASH_BLOCKSIZE + DUK__STRHASH_BLOCKSIZE;
        }

        res = hashBytes(x, 0, DUK__STRHASH_SHORTSTRING, str_seed, big_endian);
        off = DUK__STRHASH_SHORTSTRING + Math.floor((skip * (res % 256)) / 256);

        while (off < x.length) {
            let left = x.length - off;
            let now = Math.min(left, DUK__STRHASH_BLOCKSIZE);
            res = (res ^ hashBytes(x, off, now, str_seed, big_endian)) & 0xffffffff;
            off += skip;
        }
    }

    if (strhash16) {
        res &= 0xffff;
    }

    return res >>> 0;
}
exports.hashStringDense = hashStringDense;

// Compute a string hash identical to duk_heap_hashstring() when sparse
// hashing is enabled.
const DUK__STRHASH_SKIP_SHIFT = 5;  // assumes default value

function hashStringSparse(x, hash_seed, strhash16) {
    if (typeof x === 'string') {
        x = bstrToUint8Array(x);
    }
    assert(x instanceof Uint8Array);
    var res = (hash_seed ^ x.length) & 0xffffffff;

    var step = (x.length >>> DUK__STRHASH_SKIP_SHIFT) + 1;
    var off = x.length;
    while (off >= step) {
        assert(off >= 1);
        res = ((res * 33) + x[off - 1]) & 0xffffffff;
        off -= step;
    }

    if (strhash16) {
        res &= 0xffff;
    }

    return res >>> 0;
}
exports.hashStringSparse = hashStringSparse;

function test() {
    // TBD
}
exports.test = test;
