'use strict';

const { readFileUtf8, pathJoin, listDir } = require('../util/fs');
const { createBareObject } = require('../util/bare');
const { stripLastNewline } = require('../util/string_util');

const re_line_provides = /^#(?:define|undef)\s+(\w+).*$/g;
const re_line_requires = /(DUK_[A-Z0-9_]+)/g;  // uppercase only, don't match 'DUK_USE_xxx' for example

/* Strip C/C++ comments of any DUK_ prefixed text to avoid incorrect
 * requires/provides detection.  Other comment text is kept.
 */
function stripCommentsFromLines(lines) {
    function censor(x) {
        return x.replace(/DUK_\w+/gm, 'xxx');
    }

    let tmp = lines.join('\n');
    tmp = tmp.replace(/\/\*(?:.|\n)*?\*\//gm, censor);
    tmp = tmp.replace(/\/\/.*?$/gm, censor);
    return tmp.split('\n');
}

function Snippet(arg) {
    var filename;

    if (!new.target) {
        throw new TypeError('must be called as a constructor');
    }
    this.provides = createBareObject({});
    this.requires = createBareObject({});

    if (Array.isArray(arg)) {
        this.lines = arg;
    } else if (typeof arg === 'string') {
        filename = arg;
        let data = stripLastNewline(readFileUtf8(arg));  // Strip last newline to avoid empty line.
        this.lines = data.split('\n');
    } else {
        throw new TypeError('invalid argument');
    }

    if (filename) {
        let snippetDir = pathJoin(filename, '..', '..', 'header-snippets');  // This hardcoding is awkward.
        this.lines = this.lines.flatMap((line) => {
            if (line.startsWith('#snippet')) {
                let m = /#snippet\s+"(.*?)"/.exec(line);
                if (!m) {
                    throw new TypeError('invalid #snippet: ' + line);
                }
                let subFn = pathJoin(snippetDir, m[1]);
                let sn = new Snippet(subFn);
                return sn.lines;
            }
            return line;
        });
    }

    this.strippedLines = stripCommentsFromLines(this.lines);

    for (let line of this.strippedLines) {
        // Careful with order: a snippet may self-reference its own defines
        // in which case there's no outward 'require' dependency.  Current
        // handling is not 100% because the order of require/provide is not
        // checked.
        //
        // Also, some snippets may #undef/#define another define but
        // they don't "provide" the define as such.  Such redefinitions
        // are marked "/* redefine */" in the snippets.  They're best
        // avoided and not currently needed.

        let m;
        while ((m = re_line_provides.exec(line)) !== null) {
            let cap = m[1];
            if (!line.includes('/* redefine */') && cap.length > 0) {
                // Line doesn't include '/* redefine */'.
                this.provides[cap] = true;
            }
        }
        while ((m = re_line_requires.exec(line)) !== null) {
            let cap = m[1];
            if (cap.length > 0 && cap.endsWith('_')) {
                // Don't allow e.g. 'DUK_USE_' which results from matching 'DUK_USE_xxx'.
                continue;
            }
            if (cap.startsWith('DUK_USE_')) {
                // DUK_USE_xxx are internal and should not be required.
                continue;
            }
            if (this.provides[cap]) {
                // Self reference, omit (assume require after provide).
                continue;
            }
            this.requires[cap] = true;
        }
    }
}

exports.Snippet = Snippet;

function scanHelperSnippets(dir) {
    var res = [];
    for (let fn of listDir(dir).sort()) {
        if (fn.startsWith('DUK_F_') && fn.endsWith('.h.in')) {
            res.push(new Snippet(pathJoin(dir, fn)));
        }
    }
    return res;
}
exports.scanHelperSnippets = scanHelperSnippets;
