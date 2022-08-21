'use strict';

var enableDebug = false;

function makeLogDocument(args, level) {
    return {
        time: Date.now() / 1000,
        message: Array.prototype.map.call(args, String).join(' '),
        level
    };
}

function writeLogDocument(doc) {
    //console.log(JSON.stringify(doc));
    if (doc.level !== 'DEBUG' || enableDebug) {
        console.log(doc.message);
    }
}

function logDebug() {
    let doc = makeLogDocument(arguments, 'DEBUG');
    writeLogDocument(doc);
}

function logInfo() {
    let doc = makeLogDocument(arguments, 'INFO');
    writeLogDocument(doc);
}

function logWarn() {
    let doc = makeLogDocument(arguments, 'WARN');
    writeLogDocument(doc);
}

exports.logDebug = logDebug;
exports.logInfo = logInfo;
exports.logWarn = logWarn;
