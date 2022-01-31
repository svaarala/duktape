'use strict';

const { assert } = require('../../util/assert');
const { parse: parseYaml } = require('../../util/yaml');

function parseTestcaseMetadata({ sourceString }) {
    assert(typeof sourceString === 'string');
    var meta = Object.create(null);
    var dummy = sourceString.replace(/\/\*---\n^((.|\r|\n)*?)^---\*\//gm, (m, a) => {
        let doc = parseYaml(a);
        Object.assign(meta, doc);
        meta.foundMetadataBlocks = true;
        return '';
    });
    void dummy;

    // Approximate 'use strict' parsing, replaced with explicit metadata
    // .use_strict flag.
    sourceString.split('\n').forEach((line) => {
        if (line.startsWith('"use strict"') || line.startsWith("'use strict'")) {
            meta.autoDetectedUseStrict = true;
        }
    });

    return meta;
}
exports.parseTestcaseMetadata = parseTestcaseMetadata;
