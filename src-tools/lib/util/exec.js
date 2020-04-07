'use strict';

const { isNodejs, isDuktape } = require('./engine_detect');
const { readFileUtf8 } = require('../extbindings/fileio');  // Avoid circular dependency on util/fs

function exec(cmdline, opts) {
    void opts;

    if (isNodejs()) {
        const child_process = require('child_process');
        var args = cmdline.slice(1);
        var cmd = cmdline[0];

        var res = child_process.spawnSync(cmd, args, {});
        return {
            stdout: res.stdout,
            stderr: res.stderr,
            status: res.status,
            signal: res.signal,
            error: res.error
        };
    } else if (isDuktape()) {
        if (typeof sysExecute === 'function') {
            let tmp = cmdline[0] + ' ' + cmdline.slice(1).map((v) => '"' + v + '"').join(' ') + ' >/tmp/stdout';
            //console.log('EXECUTE: ' + tmp);
            sysExecute(tmp);
            return {
                stdout: readFileUtf8('/tmp/stdout')
            };
        } else {
            throw new TypeError('no provider for exec');
        }
    } else {
        throw new TypeError('no provider for exec');
    }
}
exports.exec = exec;

function execStdoutUtf8(cmdline, opts) {
    return exec(cmdline, opts).stdout.toString('utf-8');
}
exports.execStdoutUtf8 = execStdoutUtf8;
