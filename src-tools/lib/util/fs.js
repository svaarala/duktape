'use strict';

const { readFile, readFileUtf8, writeFile, writeFileUtf8 } = require('../extbindings/fileio');
const { isNodejs, isDuktape } = require('./engine_detect');
const { exec } = require('./exec');

// Forward these to hide the extbindings module.
exports.readFile = readFile;
exports.readFileUtf8 = readFileUtf8;
exports.writeFile = writeFile;
exports.writeFileUtf8 = writeFileUtf8;

function pathJoin() {
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

function listDir(dir) {
    if (isNodejs()) {
        const fs = require('fs');
        return fs.readdirSync(dir);
    } else if (isDuktape()) {
        return exec([ 'ls', dir ]).stdout.split('\n').filter((fn) => fn !== '');
    } else {
        throw new TypeError('no provider for listDir');
    }
}
exports.listDir = listDir;

function getCwd() {
    if (isNodejs()) {
        return process.cwd();
    } else if (isDuktape()) {
        return exec([ 'pwd' ]).stdout.split('\n')[0];
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

function basename(pathArg) {
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

function fileExists(fn) {
    if (isNodejs()) {
        const fs = require('fs');
        return fs.existsSync(fn);
    } else if (isDuktape()) {
        try {
            void readFile(fn);
            return true;
        } catch (e) {
            /* nop */
        }
        return false;
    } else {
        throw new TypeError('no provider for fileExists');
    }
}
exports.fileExists = fileExists;

function writeFileJsonPretty(fn, doc) {
    writeFileUtf8(fn, JSON.stringify(doc, null, 4) + '\n');
}
exports.writeFileJsonPretty = writeFileJsonPretty;
