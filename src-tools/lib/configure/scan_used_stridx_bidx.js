'use strict';

const { readFileUtf8 } = require('../util/fs');

// Scan source files for:
// - strings which need a stridx (to thr->strs[])
// - objects which need a bidx (to thr->builtins[])
// - DUK_USE_xxx config options referenced
function scanUsedStridxBidx(filenameList) {
    var strDefs = {};
    var objDefs = {};
    var optDefs = {};

    function matchAll(data, re, map) {
        var m;
        while ((m = re.exec(data)) !== null) {
            map[m[1]] = true;
        }
    }

    for (let fn of filenameList) {
        const re_str_stridx = /DUK_STRIDX_(\w+)/gm;
        const re_str_heap = /DUK_HEAP_STRING_(\w+)/gm;
        const re_str_hthread = /DUK_HTHREAD_STRING_(\w+)/gm;
        const re_obj_bidx = /DUK_BIDX_(\w+)/gm;
        const re_duk_use = /DUK_USE_(\w+)/gm;

        let data = readFileUtf8(fn);
        matchAll(data, re_str_stridx, strDefs);
        matchAll(data, re_str_heap, strDefs);
        matchAll(data, re_str_hthread, strDefs);
        matchAll(data, re_obj_bidx, objDefs);
        matchAll(data, re_duk_use, optDefs);
    }

    var strUsed = Object.keys(strDefs).sort().map((v) => 'DUK_STRIDX_' + v);
    var objUsed = Object.keys(objDefs).sort().map((v) => 'DUK_BIDX_' + v);
    var optUsed = Object.keys(optDefs).sort().map((v) => 'DUK_USE_' + v);

    return {
        usedStridxDefines: strUsed,
        usedBidxDefines: objUsed,
        usedDukUseOptions: optUsed,
    };
}
exports.scanUsedStridxBidx = scanUsedStridxBidx;
