'use strict';

const global = new Function('return this')();
const { assert } = require('../util/assert');
const { bufferToUint8Array } = require('../util/buffer');
const { readFile } = require('./fileio');
const { utf8Uint8ArrayToString } = require('./utf8');

function bufConvert(x) {
    if (x instanceof Buffer) {
        return bufferToUint8Array(x);
    } else if (x instanceof Uint8Array) {
        return x;
    }
    assert(x === void 0 || x === null);
    return x;
}

function execEscape(x) {
    // Approximate, but good enough for now.
    if (/[\u0000-\u001f\u007e-\uffff]"/.test(x)) {
        throw new TypeError('reject shell argument: ' + x);
    }
    return '"' + x + '"';
}

// Execute commandline.  Return an object with .stdout, .stderr, and a possible .error.
function exec(cmdArg, opts) {
    void opts;

    assert(Array.isArray(cmdArg));

    if (typeof global.sysExecute === 'function') {
        // Duktape
        sysExecute('rm -f /tmp/duk-stdout /tmp/duk-stderr');
        let cmdline = cmdArg[0] + ' ' + cmdArg.slice(1).map(execEscape).join(' ') + ' >/tmp/duk-stdout 2>/tmp/duk-stderr';
        //console.log('EXECUTE: ' + cmdline);
        let error;
        try {
             sysExecute(cmdline);
        } catch (e) {
            error = e;
        }
        let stdout = bufConvert(readFile('/tmp/duk-stdout'));
        let stderr = bufConvert(readFile('/tmp/duk-stderr'));
        sysExecute('rm -f /tmp/duk-stdout /tmp/duk-stderr');
        return { stdout, stderr, error };
    } else {
        // Node.js
        const child_process = require('child_process');
        let args = cmdArg.slice(1);
        let cmd = cmdArg[0];
        let res = child_process.spawnSync(cmd, args, {});
        return {
            stdout: bufConvert(res.stdout),
            stderr: bufConvert(res.stderr),
            status: res.status,
            signal: res.signal,
            error: res.error
        };
    }
}
exports.exec = exec;

// Execute and get stdout interpreted as UTF-8.  If command fails, throw.
function execStdoutUtf8(cmdArg, opts) {
    var res = exec(cmdArg, opts);
    if (res.error) {
        throw res.error;
    }
    var u8 = res.stdout;
    assert(u8 instanceof Uint8Array);

    return utf8Uint8ArrayToString(u8);
}
exports.execStdoutUtf8 = execStdoutUtf8;
