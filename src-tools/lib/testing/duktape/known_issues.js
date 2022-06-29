'use strict';

const { listDir, pathJoin, readFileUtf8 } = require('../../util/fs');
const { parse: parseYaml } = require('../../util/yaml');
const { hexEncode } = require('../../util/hex');
const { md5 } = require('../../util/md5');

// Parse a known issue: either a plain YAML file, or a YAML file separated by
// a '---' line, followed by expected output for the known issue.
function parseKnownIssue(fn) {
    let data = readFileUtf8(fn);
    let idx = data.indexOf('\n---\n');
    let doc;
    if (idx >= 0) {
        doc = parseYaml(data.substring(0, idx + 1));
        doc.output = data.substring(idx + 5);
    } else {
        doc = parseYaml(data);
    }
    doc.filename = fn;

    return doc;
}

// Parse known issue files from a specified directory.
function parseKnownIssues({ knownIssuesDirectory }) {
    let result = [];

    if (typeof knownIssuesDirectory !== 'string') {
        return result;
    }

    for (let fn of listDir(knownIssuesDirectory)) {
        if (require('fs').statSync(pathJoin(knownIssuesDirectory, fn)).isFile()) {
            result.push(parseKnownIssue(pathJoin(knownIssuesDirectory, fn)));
        }
    }
    return result;
}

// Find a known issue matching actual test stdout.
function findKnownIssue({ knownIssues, stdoutActual }) {
    let stdoutMd5 = hexEncode(md5(new TextEncoder().encode(stdoutActual).buffer));
    for (let o of knownIssues) {
        if (typeof o.output === 'string' && o.output === stdoutActual) {
            return o;
        } else if (typeof o.md5 === 'string' && o.md5.toLowerCase() === stdoutMd5.toLowerCase()) {
            return o;
        }
    }
}

exports.parseKnownIssues = parseKnownIssues;
exports.findKnownIssue = findKnownIssue;
