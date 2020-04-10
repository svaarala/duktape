'use strict';

const { pathJoin } = require('../util/fs');
const { stripLastNewline } = require('../util/string_util');
const { Snippet } = require('./snippet');

/* Helper for building a text file from individual lines, injected files, etc.
 * Inserted values are converted to Snippets so that their provides/requires
 * information can be tracked.  When non-C outputs are created, these will be
 * bogus but ignored.
 */
function FileBuilder(baseDirectory, opts) {
    this.vals = [];  // snippets
    this.baseDirectory = baseDirectory;
    this.useCppWarning = !!opts.useCppWarning;
}
FileBuilder.prototype.line = function line(x) {
    this.vals.push(new Snippet([ x ]));
};
FileBuilder.prototype.lines = function lines(x) {
    x = stripLastNewline(x);  // Strip last newline to avoid empty line
    this.vals.push(new Snippet(x.split('\n')));
};
FileBuilder.prototype.empty = function empty() {
    this.vals.push(new Snippet([ '' ]));
};
FileBuilder.prototype.rstHeading = function rstHeading(title, ch, doubled) {
    var tmp = [];
    if (doubled) {
        tmp.push(ch.repeat(title.length));
    }
    tmp.push(title);
    tmp.push(ch.repeat(title.length));
    this.vals.push(new Snippet(tmp));
};
FileBuilder.prototype.snippetRelative = function snippetRelative(fn) {
    this.vals.push(new Snippet(pathJoin(this.baseDirectory, fn)));
};
FileBuilder.prototype.snippetAbsolute = function snippetAbsolute(fn) {
    this.vals.push(new Snippet(fn));
};
FileBuilder.prototype.cppError = function cppError(msg) {
    // Assumes no newlines etc.
    this.vals.push(new Snippet([ '#error ' + msg ]));
};
FileBuilder.prototype.cppWarning = function cppWarning(msg) {
    // Assumes no newlines etc.
    if (this.useCppWarning) {
        // C preprocessor '#warning' is often supported.
        // XXX: Support compiler specific warning mechanisms.
        this.vals.push(new Snippet([ '#warning ' + msg ]));
    } else {
        this.vals.push(new Snippet([ '/* WARNING: ' + msg + ' */' ]));
    }
};
FileBuilder.prototype.cppWarningOrError = function cppWarningOrError(msg, isError) {
    if (isError) {
        this.cppError(msg);
    } else {
        this.cppWarning(msg);
    }
};
FileBuilder.prototype.chdrCommentLine = function chdrCommentLine(msg) {
    this.vals.push(new Snippet([ '/* ' + msg + ' */' ]));
};
FileBuilder.prototype.chdrBlockHeading = function chdrBlockHeading(msg) {
    var tmp = [];
    tmp.push('');
    tmp.push('/*');
    tmp.push(' *  ' + msg);
    tmp.push(' */');
    tmp.push('');
    this.vals.push(new Snippet(tmp));
};
FileBuilder.prototype.join = function join() {
    var tmp = [];
    for (var s of this.vals) {
        if (!(s instanceof Snippet)) {
            throw new TypeError('this.vals must be all snippets');
        }
        for (var line of s.lines) {
            tmp.push(line);
        }
    }
    return tmp.join('\n') + '\n';
};

exports.FileBuilder = FileBuilder;
