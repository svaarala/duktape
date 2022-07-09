'use strict';

const { readFileUtf8 } = require('./fs');

function tokenizeFiles({ filelist }) {
    var tokens = Object.create(null);
    filelist.forEach((fn) => {
        var data = readFileUtf8(fn);
        var repl1 = data.replace(/[^a-zA-Z0-9_-]/g, ' ');
        var repl2 = repl1.replace(/\s+/g, ' ');
        var repl3 = repl2.trim();
        repl3.split(' ').forEach((token) => {
            tokens[token] = (tokens[token] || 0) + 1;
        });
    });
    return tokens;
}

exports.tokenizeFiles = tokenizeFiles;
