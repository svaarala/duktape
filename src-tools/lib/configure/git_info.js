'use strict';

const { execStdoutUtf8 } = require('../util/exec');

function getGitInfo() {
    var gitCommit, gitDescribe, gitBranch;

    if (typeof gitCommit === 'undefined') {
        gitCommit = execStdoutUtf8([ 'git', 'rev-parse', 'HEAD' ]).trim();
    }
    if (typeof gitDescribe === 'undefined') {
        gitDescribe = execStdoutUtf8([ 'git', 'describe', '--always', '--dirty' ]).trim();
    }
    if (typeof gitBranch === 'undefined') {
        gitBranch = execStdoutUtf8([ 'git', 'rev-parse', '--abbrev-ref', 'HEAD' ]).trim();
    }
    return { gitCommit, gitDescribe, gitBranch };
}
exports.getGitInfo = getGitInfo;
