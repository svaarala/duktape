'use strict';

const { logDebug } = require('../../util/logging');

// Find valid test cases from a list of files and/or directories.
// If recursive=true, process directories recursively.
function findTestCasesSync({ argList, recursive=true }) {
    var found = Object.create(null);
    var pat = /^([a-zA-Z0-9_-]+).(js|c)$/;
    var testcases = [];

    // XXX: Remove direct Node.js dependency.
    var fs = require('fs');
    var path = require('path');

    function processFile(arg) {
        let m = pat.exec(path.basename(arg));
        if (!m) { return; }
        if (found[m[1]]) { return; }
        if (m[1].substring(0, 5) === 'util-') { return; } // skip utils
        found[m[1]] = true;
        testcases.push(arg);
    }

    function process(arg, allowDirectory) {
        logDebug('scan argument:', arg);
        var st = fs.statSync(arg);
        var m;

        if (st.isFile()) {
            processFile(arg);
        } else if (st.isDirectory()) {
            if (allowDirectory) {
                // Allow directory recursion (beyond top level) only if recursive=true.
                fs.readdirSync(arg).forEach((fn) => process(path.join(arg, fn), recursive));
            }
        } else {
            throw new Exception('invalid argument: ' + arg);
        }
    }

    // Allow directory on top level argList always.
    argList.forEach((arg) => process(arg, true));

    return testcases;
}

exports.findTestCasesSync = findTestCasesSync;
