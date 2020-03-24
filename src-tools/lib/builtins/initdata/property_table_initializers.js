/*
 *  Type declarations and initializers for object property tables.
 */

'use strict';

const { getValueInitializerType, getValueInitializerLiteral } = require('./object_initializers');
const { propAttrLookup } = require('./property_attributes.js');
const { assert } = require('../../util/assert');

function emitPropertyTableStructs(genc, meta, objs, biStrMap, biObjMap) {
    // Emit property table structs.  These are very complex because
    // property count *and* individual property type affect the fields
    // in the initializer, properties can be data properties or accessor
    // properties or different duk_tval types.  There are also several
    // property table memory layouts, each with a different ordering of
    // keys, values, etc.
    //
    // The easy solution is to use a separate initializer type for each
    // property type.  Could also cache and reuse identical initializers
    // but there'd be very few of them so it's more straightforward to
    // not reuse the structs.

    genc.emitLine('#if defined(DUK_USE_HOBJECT_LAYOUT_1)');

    objs.forEach((o, idx) => {
        var numProps = o.properties.length;
        var parts = [];
        if (numProps === 0) {
            return;
        }

        parts.push('typedef struct duk_romprops_' + idx + ' duk_romprops_' + idx + '; ');
        parts.push('struct duk_romprops_' + idx + ' { ');

        o.properties.forEach((p, propIdx) => {
            parts.push('const duk_hstring *key' + propIdx + '; ');
        });
        o.properties.forEach((p, propIdx) => {
            parts.push(getValueInitializerType(meta, p, biStrMap, biObjMap) + ' val' + propIdx + '; ');
        });
        o.properties.forEach((p, propIdx) => {
            parts.push('duk_uint8_t flags' + propIdx + '; ');
        });
        parts.push('};');
        genc.emitLine(parts.join(''));
    });

    genc.emitLine('#elif defined(DUK_USE_HOBJECT_LAYOUT_2)');

    objs.forEach((o, idx) => {
        var numProps = o.properties.length;
        var parts = [];
        if (numProps === 0) {
            return;
        }

        parts.push('typedef struct duk_romprops_' + idx + ' duk_romprops_' + idx + '; ');
        parts.push('struct duk_romprops_' + idx + ' { ');

        o.properties.forEach((p, propIdx) => {
            parts.push(getValueInitializerType(meta, p, biStrMap, biObjMap) + ' val' + propIdx + '; ');
        });
        o.properties.forEach((p, propIdx) => {
            parts.push('const duk_hstring *key' + propIdx + '; ');
        });
        o.properties.forEach((p, propIdx) => {
            parts.push('duk_uint8_t flags' + propIdx + '; ');
        });
        // Padding follows for flags, but we don't need to emit it
        // (at the moment there is never an array or hash part).
        parts.push('};');
        genc.emitLine(parts.join(''));
    });

    genc.emitLine('#elif defined(DUK_USE_HOBJECT_LAYOUT_3)');

    objs.forEach((o, idx) => {
        var numProps = o.properties.length;
        var parts = [];
        if (numProps === 0) {
            return;
        }

        parts.push('typedef struct duk_romprops_' + idx + ' duk_romprops_' + idx + '; ');
        parts.push('struct duk_romprops_' + idx + ' { ');

        o.properties.forEach((p, propIdx) => {
            parts.push(getValueInitializerType(meta, p, biStrMap, biObjMap) + ' val' + propIdx + '; ');
        });
        // No array values
        o.properties.forEach((p, propIdx) => {
            parts.push('const duk_hstring *key' + propIdx + '; ');
        });
        // No hash index
        o.properties.forEach((p, propIdx) => {
            parts.push('duk_uint8_t flags' + propIdx + '; ');
        });
        parts.push('};');
        genc.emitLine(parts.join(''));
    });

    genc.emitLine('#else');
    genc.emitLine('#error invalid object layout');
    genc.emitLine('#endif');
}
exports.emitPropertyTableStructs = emitPropertyTableStructs;

function emitPropertyTableForwardDeclarations(genc, meta, objs, compressRomPtr) {
    // Forward declare all property tables so that objects can reference them.
    // Also pointer compress them.

    objs.forEach((o, idx) => {
        let numProps = o.properties.length;
        if (numProps === 0) {
            return;
        }

        // We would like to use DUK_INTERNAL_DECL here, but that maps
        // to "static const" in a single file build which has C++
        // portability issues: you can't forward declare a static const.
        // We can't reorder the property tables to avoid this because
        // there are cyclic references.  So, as the current workaround,
        // declare as external.
        genc.emitLine('DUK_EXTERNAL_DECL const duk_romprops_' + idx + ' duk_prop_' + idx + ';');

        // Add property tables to ROM compressed pointers too.
        compressRomPtr('&duk_prop_' + idx);
    });
}
exports.emitPropertyTableForwardDeclarations = emitPropertyTableForwardDeclarations;

function emitPropertyTableDefinitions(genc, meta, objs, biStrMap, biObjMap) {
    // Property tables.  Can reference arbitrary strings and objects as
    // they're defined before them.

    // Properties will be non-configurable, but must be writable so that
    // standard property semantics allow shadowing properties to be
    // established in inherited objects (e.g. "var obj={}; obj.toString
    // = myToString").  Enumerable can also be kept.

    function prepAttrs(p) {
        var attrs = p.attributes;
        assert(attrs.indexOf('c') < 0);
        var res = propAttrLookup[attrs];
        assert(typeof res === 'string');
        return res;
    }

    function emitInitializer(idx, o, layout) {
        var initVals = [];
        var initKeys = [];
        var initFlags = [];
        var initList;

        for (let p of o.properties) {
            initKeys.push('(const duk_hstring *)&' + biStrMap[p.key]);
        }
        for (let p of o.properties) {
            initVals.push(getValueInitializerLiteral(meta, p, biStrMap, biObjMap));
        }
        for (let p of o.properties) {
            initFlags.push(prepAttrs(p));
        }
        switch (layout) {
        case 1:
            initList = initKeys.concat(initVals).concat(initFlags);
            break;
        case 2:
            initList = initVals.concat(initKeys).concat(initFlags);
            break;
        case 3:
            // Same as layout 2 now, no hash/array.
            initList = initVals.concat(initKeys).concat(initFlags);
            break;
        default:
            throw new TypeError('internal error, invalid layout: ' + layout);
        }

        if (initList.length > 0) {
            genc.emitLine('DUK_EXTERNAL const duk_romprops_' + idx + ' duk_prop_' + idx +
                          ' = {' + initList.join(',') + '};');
        }
    }

    genc.emitLine('#if defined(DUK_USE_HOBJECT_LAYOUT_1)');
    objs.forEach((o, idx) => {
        emitInitializer(idx, o, 1);
    });
    genc.emitLine('#elif defined(DUK_USE_HOBJECT_LAYOUT_2)');
    objs.forEach((o, idx) => {
        emitInitializer(idx, o, 2);
    });
    genc.emitLine('#elif defined(DUK_USE_HOBJECT_LAYOUT_3)');
    objs.forEach((o, idx) => {
        emitInitializer(idx, o, 3);
    });
    genc.emitLine('#else');
    genc.emitLine('#error invalid object layout');
    genc.emitLine('#endif');
}
exports.emitPropertyTableDefinitions = emitPropertyTableDefinitions;
