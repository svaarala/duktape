'use strict';

const { readFile, readFileUtf8, writeFile, writeFileUtf8 } = require('../extbindings/fileio');
const { isNodejs, isDuktape } = require('./engine_detect');
const { exec, execStdoutUtf8 } = require('./exec');
const { parse: parseYaml, stringify: stringifyYaml } = require('../util/yaml');
const { assert } = require('../util/assert');

// Forward these to hide the extbindings module.
exports.readFile = readFile;
exports.readFileUtf8 = readFileUtf8;
exports.writeFile = writeFile;
exports.writeFileUtf8 = writeFileUtf8;

function pathJoin() {
    if (isNodejs()) {
        const path = require('path');
        return path.join.apply(path, arguments);
    } else if (isDuktape()) {
        let tmp = Array.prototype.map.call(arguments, String).join('/').replace(/\/+/g, '/').split('/');
        for (let idx = 0; idx < tmp.length;) {
            if (tmp[idx] === '.') {
                void tmp.splice(idx, 1);
            } else if (tmp[idx] === '..') {
                if (idx >= 1) {
                    void tmp.splice(idx - 1, 2);
                    idx--;
                } else {
                    // Keep '..' as is if nothing to rewind.
                    idx++;
                }
            } else if (tmp[idx] === '' && idx === tmp.length - 1) {
                // Keep empty part caused by trailing slash, e.g. 'foo/bar/'.
                idx++;
            } else {
                idx++;
            }
        }
        return tmp.join('/');
    } else {
        throw new TypeError('no provided for pathJoin');
    }
}
exports.pathJoin = pathJoin;

function pathJoinTest() {
    // > require('path').join('foo/bar/quux.c', '..', 'baz')
    // 'foo/bar/baz'
    // > require('path').join('foo/bar/', '..', 'baz')
    // 'foo/baz'
    // > require('path').join('foo/bar', '..', 'baz')
    // 'foo/baz'
    // > require('path').join('foo/bar', '..', '..', 'baz')
    // 'baz'
    // > require('path').join('foo/bar', '..', '..', '..', 'baz')
    // '../baz'
    // > require('path').join('foo/bar', 'baz')
    // 'foo/bar/baz'
    // > require('path').join('foo/bar', 'baz/')
    // 'foo/bar/baz/'
    // > require('path').join('foo/bar/')
    // 'foo/bar/'

    if (!isDuktape()) {
        return;
    }

    assert(pathJoin('foo/bar/quux.c', '..', 'baz') === 'foo/bar/baz');
    assert(pathJoin('foo/bar/', '..', 'baz') === 'foo/baz');
    assert(pathJoin('foo/bar', '..', 'baz') === 'foo/baz');
    assert(pathJoin('foo/bar', '..', '..', 'baz') === 'baz');
    assert(pathJoin('foo/bar', '..', '..', '..', 'baz') === '../baz');
    assert(pathJoin('foo/bar', 'baz') === 'foo/bar/baz');
    assert(pathJoin('foo/bar', 'baz/') === 'foo/bar/baz/');
    assert(pathJoin('foo/bar/') === 'foo/bar/');
}
pathJoinTest();

function listDir(dir) {
    if (isNodejs()) {
        const fs = require('fs');
        return fs.readdirSync(dir);
    } else if (isDuktape()) {
        return execStdoutUtf8([ 'ls', '-a', dir ]).split('\n').filter((fn) => {
            return fn !== '' && fn !== '.' && fn !== '..';
        });
    } else {
        throw new TypeError('no provider for listDir');
    }
}
exports.listDir = listDir;

function getCwd() {
    if (isNodejs()) {
        return process.cwd();
    } else if (isDuktape()) {
        return execStdoutUtf8([ 'pwd' ]).split('\n')[0];
    } else {
        throw new TypeError('no provider for getCwd');
    }
}
exports.getCwd = getCwd;

function copyFile(srcFn, dstFn) {
    writeFile(dstFn, readFile(srcFn));
}
exports.copyFile = copyFile;

function dirname(pathArg) {
    if (isNodejs()) {
        const path = require('path');
        return path.dirname(pathArg);
    } else if (isDuktape()) {
        let parts = pathArg.replace(/\/+/g, '/').split('/');
        if (parts.length >= 1 && parts[parts.length - 1] === '') {
            parts.pop();  // strip trailing slash
        }
        if (parts.length >= 1) {
            parts.pop();
        }
        return parts.join('/') || '.';
    } else {
        throw new TypeError('no provider for dirname');
    }
}
exports.dirname = dirname;

function dirnameTest() {
    // > require('path').dirname('foo/bar')
    // 'foo'
    // > require('path').dirname('foo/bar/')
    // 'foo'
    // > require('path').dirname('foo/bar//')
    // 'foo'
    // > require('path').dirname('foo/bar/quux')
    // 'foo/bar'
    // > require('path').dirname('')
    // '.'
    // > require('path').dirname('foo')
    // '.'

    if (!isDuktape()) {
        return;
    }

    assert(dirname('foo/bar') === 'foo');
    assert(dirname('foo/bar/') === 'foo');
    assert(dirname('foo/bar//') === 'foo');
    assert(dirname('foo/bar/quux') === 'foo/bar');
    assert(dirname('') === '.');
    assert(dirname('foo') === '.');
}
dirnameTest();

function basename(pathArg) {
    if (isNodejs()) {
        const path = require('path');
        return path.basename(pathArg);
    } else if (isDuktape()) {
        let parts = pathArg.replace(/\/+/g, '/').split('/');
        if (parts.length >= 1 && parts[parts.length - 1] === '') {
            parts.pop();  // strip trailing slash
        }
        if (parts.length >= 1) {
            return parts[parts.length - 1];
        } else {
            return '';
        }
    } else {
        throw new TypeError('no provider for basename');
    }
}
exports.basename = basename;

function basenameTest() {
    // > require('path').basename('foo')
    // 'foo'
    // > require('path').basename('foo/')
    // 'foo'
    // > require('path').basename('foo/bar')
    // 'bar'
    // > require('path').basename('foo/bar.c')
    // 'bar.c'
    // > require('path').basename('foo/bar.c/')
    // 'bar.c'
    // > require('path').basename('foo//bar.c//')
    // 'bar.c'
    // > require('path').basename('')
    // ''
    // > require('path').basename('.')
    // '.'
    // > require('path').basename('..')
    // '..'
    // > require('path').basename('foo/..')
    // '..'

    if (!isDuktape()) {
        return;
    }

    assert(basename('foo') === 'foo');
    assert(basename('foo/') === 'foo');
    assert(basename('foo/bar') === 'bar');
    assert(basename('foo/bar.c') === 'bar.c');
    assert(basename('foo/bar.c/') === 'bar.c');
    assert(basename('foo//bar.c//') === 'bar.c');
    assert(basename('') === '');
    assert(basename('.') === '.');
    assert(basename('..') === '..');
    assert(basename('foo/..') === '..');
}
basenameTest();

function pathExists(path) {
    if (isNodejs()) {
        const fs = require('fs');
        return fs.existsSync(path);
    } else if (isDuktape()) {
        let res = exec([ 'test', '-e', path ]);
        return !res.error;
    } else {
        throw new TypeError('no provider for pathExists');
    }
}
exports.pathExists = pathExists;

function fileExists(path) {
    if (isNodejs()) {
        const fs = require('fs');
        return fs.existsSync(path) && fs.lstatSync(path).isFile();
    } else if (isDuktape()) {
        let res = exec([ 'test', '-f', path ]);
        return !res.error;
    } else {
        throw new TypeError('no provider for fileExists');
    }
}
exports.fileExists = fileExists;

function dirExists(path) {
    if (isNodejs()) {
        const fs = require('fs');
        return fs.existsSync(path) && fs.lstatSync(path).isDirectory();
    } else if (isDuktape()) {
        let res = exec([ 'test', '-d', path ]);
        return !res.error;
    } else {
        throw new TypeError('no provider for dirExists');
    }
}
exports.dirExists = dirExists;

function readFileJson(fn) {
    return JSON.parse(readFileUtf8(fn));
}
exports.readFileJson = readFileJson;

function readFileYaml(fn) {
    return parseYaml(readFileUtf8(fn));
}
exports.readFileYaml = readFileYaml;

function writeFileJson(fn, doc) {
    writeFileUtf8(fn, JSON.stringify(doc) + '\n');
}
exports.writeFileJson = writeFileJson;

function writeFileJsonPretty(fn, doc) {
    writeFileUtf8(fn, JSON.stringify(doc, null, 4) + '\n');
}
exports.writeFileJsonPretty = writeFileJsonPretty;

function writeFileYamlPretty(fn, doc) {
    writeFileUtf8(fn, stringifyYaml(doc));
}
exports.writeFileYamlPretty = writeFileYamlPretty;

function mkdir(pathArg) {
    if (isNodejs()) {
        const fs = require('fs');
        fs.mkdirSync(pathArg);
    } else if (isDuktape()) {
        void execStdoutUtf8([ 'mkdir', pathArg ]);
    } else {
        throw new TypeError('no provider for mkdir');
    }
}
exports.mkdir = mkdir;

// Get a time-based path component, should be filename safe and sort nicely.
function getTimeBasedPathComponent() {
    return Date.now().toString(36);
}

function createTempDir(args) {
    var timeComp = getTimeBasedPathComponent();
    void args;

    if (isNodejs()) {
        const os = require('os');
        const fs = require('fs');
        const path = require('path');
        let res = fs.mkdtempSync(path.join(os.tmpdir(), 'tmp.duk-' + timeComp + '-'));
        console.log('created temp directory:', res);
        return res;
    } else if (isDuktape()) {
        let res = execStdoutUtf8([ 'mktemp', '-d', '-q', '/tmp/tmp.duk-' + timeComp + '-XXXXXX' ]).trim();
        console.log('created temp directory:', res);
        return res;
    } else {
        throw new TypeError('no provider for mkdir');
    }
}
exports.createTempDir = createTempDir;
