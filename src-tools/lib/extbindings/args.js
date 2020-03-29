'use strict';

function getArgs() {
    if (typeof process === 'object' && process !== null && process.argv) {
        // $ nodejs --zero-fill-buffers test.js --bar
        // '/usr/bin/node', '/tmp/test.js', '--bar' ]
        //
        // Command and script are included, Node.js options are stripped, and
        // anything following the script is included.  For above we'd return
        // [ '--bar' ].
        return JSON.parse(JSON.stringify(process.argv.slice(2)));
    } else if (typeof sysArgv === 'object' && sysArgv !== null) {
        // Duktape, use raw argv, scan for '--', and include anything after
        // that as arguments.
        for (let i = 0; i < sysArgv.length; i++) {
            if (sysArgv[i] === '--') {
                return JSON.parse(JSON.stringify(sysArgv.slice(i + 1)));
            }
        }
        return [];
    } else {
        throw new TypeError('no provider for getArgs');
    }
}
exports.getArgs = getArgs;
