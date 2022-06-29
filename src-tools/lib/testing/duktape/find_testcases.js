'use strict';

// Find valid test cases from a list of files and/or directories.
// Directories are not processed recursively.
function findTestCasesSync(argList) {
    var found = Object.create(null);
    var pat = /^([a-zA-Z0-9_-]+).(js|c)$/;
    var testcases = [];

    // XXX: Remove direct Node.js dependency.
    var fs = require('fs');
    var path = require('path');

    argList.forEach((arg) => {
        var st = fs.statSync(arg);
        var m;

        if (st.isFile()) {
            m = pat.exec(path.basename(arg));
            if (!m) { return; }
            if (found[m[1]]) { return; }
            if (m[1].substring(0, 5) === 'util-') { return; } // skip utils
            found[m[1]] = true;
            testcases.push(arg);
        } else if (st.isDirectory()) {
            fs.readdirSync(arg)
                .forEach((fn) => {
                    var m = pat.exec(fn);
                    if (!m) { return; }
                    if (found[m[1]]) { return; }
                    if (m[1].substring(0, 5) === 'util-') { return; } // skip utils
                    found[m[1]] = true;
                    testcases.push(path.join(arg, fn));
                });
        } else {
            throw new Exception('invalid argument: ' + arg);
        }
    });

    return testcases;
}

exports.findTestCasesSync = findTestCasesSync;
