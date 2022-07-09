'use strict';

const { readFile } = require('../util/fs');

function processFile(filename) {
    var data = readFile(filename);
    var line = 1;
    var lineStart = 0;
    for (let i = 0; i < data.length; i++) {
        let t = data[i];
        if (t == 0x0a) {
            line++;
            lineStart = i + 1;
        } else if (t >= 0x80) {
            console.log(filename + ': non-ascii data on line ' + line + ', char index ' + (i - lineStart) + ', value ' + t);
        }
    }
}

function findNonAscii(files) {
    files.forEach((filename) => {
        processFile(filename);
    });
}
exports.findNonAscii = findNonAscii;
