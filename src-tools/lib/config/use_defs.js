'use strict';

const { readFileYaml, pathJoin, listDir } = require('../util/fs');

const requiredUseMetaKeys = [
    'define',
    'introduced',
    'default',
    'tags',
    'description'
];

const allowedUseMetaKeys = [
    'define',
    'introduced',
    'deprecated',
    'removed',
    'unused',
    'requires',
    'conflicts',
    'related',
    'default',
    'tags',
    'description',
    'warn_if_missing'
];

function scanUseDefs(dir) {
    let useDefs = {};

    for (let fn of listDir(dir).sort()) {
        if (!fn.startsWith('DUK_USE_') || !fn.endsWith('.yaml')) {
            continue;
        }
        var doc = readFileYaml(pathJoin(dir, fn));
        if (doc.example) {
            console.debug('example config option, skip: ' + fn);
            continue;
        }
        if (doc.unimplemented) {
            console.debug('unimplemented config option, skip: ' + fn);
            continue;
        }
        for (let k of Object.keys(doc).sort()) {
            if (!allowedUseMetaKeys.includes(k)) {
                console.log('unknown key ' + k + ' in metadata file, ignoring: ' + fn);
            }
        }
        for (let k of requiredUseMetaKeys) {
            if (!(k in doc)) {
                console.log('missing key ' + k + ' in metadata file, ignoring: ' + fn);
            }
        }
        //console.log(doc);
        useDefs[doc.define] = doc;
    }

    return useDefs;
}
exports.scanUseDefs = scanUseDefs;
