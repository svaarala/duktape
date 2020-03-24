/*
 *  Validate metadata.
 */

'use strict';

const { createBareObject } = require('../../util/bare');
const { walkObjectsAndProperties, walkStrings } = require('./util');
const { assert } = require('../../util/assert');
const { splitHexDouble } = require('../../util/double');

function validateHexDoubleNaN(hexbytes) {
    var parts = splitHexDouble(hexbytes);
    console.debug(hexbytes, parts);
    if (parts.exponent === 0x7ff && parts.mantissa !== 0) {
        if (parts.sign !== 0 || parts.mantissa !== 0x8000000000000) {
            throw new TypeError('invalid NaN, not in normalized form: ' + hexbytes);
        }
    }
}

function validateFinalMetadata(meta) {
    var strsFound = createBareObject({});
    var objIdsFound = createBareObject({});

    walkObjectsAndProperties(meta, (o) => {
        assert(o.disable !== true);
        assert(typeof o.id === 'string');
        if (objIdsFound[o.id]) {
            throw new TypeError('duplicate object for id ' + o.id);
        }
        objIdsFound[o.id] = true;
        assert(typeof o.magic === 'undefined' || typeof o.magic === 'number');
    }, (p, o) => {
        void o;
        assert(p.disable !== true);
        let v = p.value;
        if (typeof v === 'object' && v !== null && v.type === 'double') {
            validateHexDoubleNaN(v.bytes);
        }
    });

    walkStrings(meta, (s) => {
        assert(s.disable !== true);
        assert(typeof s.str === 'string');
        if (strsFound[s.str]) {
            throw new TypeError('duplicate string: ' + s.str);
        }
        strsFound[s.str] = true;
    });
}
exports.validateFinalMetadata = validateFinalMetadata;
