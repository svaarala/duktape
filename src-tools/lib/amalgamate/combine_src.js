/*
 *  Combine a set of a source files into a single C file.
 *
 *  Overview of the process:
 *
 *    * Parse user supplied C files.  Add automatic #undefs at the end
 *      of each C file to avoid defines bleeding from one file to another.
 *
 *    * Combine the C files in specified order.  If sources have ordering
 *      dependencies (depends on application), order may matter.  Caller
 *      provides the intended order.
 *
 *    * Process #include statements in the combined source, categorizing
 *      them either as "internal" (found in specified include path) or
 *      "external".  Internal includes, unless explicitly excluded, are
 *      inlined into the result while extenal includes are left as is.
 *      Duplicate internal #include statements are replaced with a comment.
 *
 *  At every step, source and header lines are represented with explicit
 *  line objects which keep track of original filename and line.  The
 *  output contains #line directives, if requested, to ensure error
 *  throwing and other diagnostic info will work in a useful with the
 *  original sources.  It's also possible to generate a combined source
 *  with no #line directives which may be more appropriate for deployment.
 *
 *  Making the process deterministic is important, so that if users have
 *  diffs that they apply to the combined source, such diffs would apply
 *  for as long as possible.
 *
 *  Limitations and notes:
 *
 *    * While there are automatic #undef's for #define's introduced in each
 *      C file, it's not possible to "undefine" structs, unions, etc.  If
 *      there are structs/unions/typedefs with conflicting names, these
 *      have to be resolved in the source files first.
 *
 *    * Because duplicate #include statements are suppressed, currently
 *      assumes #include statements are not conditional.
 *
 *    * A system header might be #include'd in multiple source files with
 *      different feature defines (like _BSD_SOURCE).  Because the #include
 *      file will only appear once in the resulting source, the first
 *      occurrence wins.  The result may not work correctly if the feature
 *      defines must actually be different between two or more source files.
 */

'use strict';

const { readFileUtf8, pathJoin, basename, fileExists } = require('../util/fs');
const { createBareObject } = require('../util/bare');
const { stripLastNewline, normalizeNewlines } = require('../util/string_util');
const { cStrEncode } = require('../util/cquote');

function Line(fileName, lineNo, data) {
    this.fileName = basename(fileName);
    this.fileNameFull = fileName;
    this.lineNo = lineNo;
    this.data = data;
}

function File(fileName, lines) {
    this.fileName = basename(fileName);
    this.fileNameFull = fileName;
    this.lines = lines;
}
File.prototype.pushTextLine = function pushTextLine(text) {
    this.lines.push(new Line(this.fileNameFull, this.lines.length + 1, text));
}

function readFile(fileName) {
    var data = stripLastNewline(normalizeNewlines(readFileUtf8(fileName)));
    var lines = data.split('\n').map((line, idx) => new Line(fileName, idx + 1, line));
    return new File(fileName, lines);
}

function CombineSource(includePaths, includeExcluded) {
    // Include path for finding include files which are amalgamated.
    this.includePaths = includePaths;

    // Include files specifically excluded from being inlined.
    this.includeExcluded = includeExcluded;
    this.includeExcludedMap = createBareObject({});
    this.includeExcluded.forEach((inc) => {
        this.includeExcludedMap[inc] = true;
    });
}
CombineSource.prototype.lookupInclude = function lookupInclude(incFileName) {
    var incComponents = incFileName.split(/\/|\\/g);  // Split include path, support forward slash and backslash

    for (let path of this.includePaths) {
        let fn = pathJoin.apply(null, [ path ].concat(incComponents));
        if (fileExists(fn)) {
            return fn;
        }
    }
};
CombineSource.prototype.addAutomaticUndefs = function addAutomaticUndefs(f) {
    var defined = createBareObject({});

    f.lines.forEach((line) => {
        let matchDef = /^#define\s+(\w+).*$/.exec(line.data);
        if (matchDef) {
            //console.debug('defined: ' + matchDef[1]);
            defined[matchDef[1]] = true;
        }
        let matchUndef = /^#undef\s+(\w+).*$/.exec(line.data);
        if (matchUndef) {
            // Could just ignore #undef's here: we'd then emit
            // reliable #undef's (though maybe duplicates) at
            // the end.
            //console.debug('undefined: ' + matchUndef[1]);
            delete defined[matchUndef[1]];
        }
    });

    // Undefine anything that seems to be left defined.  This not a 100%
    // process because some #undef's might be conditional which we don't
    // track at the moment.  Note that it's safe to #undef something that's
    // not defined.

    let keys = Object.getOwnPropertyNames(defined).sort();
    if (keys.length > 0) {
        f.pushTextLine('');
        f.pushTextLine('/* automatic undefs */');
        keys.forEach((k) => {
            console.debug('automatic #undef for ' + k);
            f.pushTextLine('#undef ' + k);
        });
    }
};
CombineSource.prototype.combineFiles = function combineFiles(files, prologueFileName, lineDirectives) {
    var _self = this;
    var res = [];
    var lineMap = [];  // indicate combined source lines where uncombined file/line would change
    var metadata = {
        line_map: lineMap
    }
    var currFileName, currLineNo;
    var included = createBareObject({});  // headers already included

    function emit(line) {
        if (typeof line === 'string') {
            res.push(line);
            currLineNo++;
        } else {
            if (line.fileName !== currFileName || line.lineNo !== currLineNo) {
                if (lineDirectives) {
                    res.push('#line ' + line.lineNo + ' ' + cStrEncode(line.fileName));
                }
                lineMap.push({ original_file: line.fileName,
                               original_line: line.lineNo,
                               combined_line: res.length + 1 });
            }
            res.push(line.data);
            currFileName = line.fileName;
            currLineNo = line.lineNo + 1;
        }
    }

    // Process a file, appending it to the result; the input may be a
    // source or an include file.  #include directives are handled
    // recursively.
    function processFile(f) {
        console.debug('process file: ' + f.fileName);
        f.lines.forEach((line) => {
            if (!line.data.startsWith('#include')) {
                emit(line);
                return;
            }

            let matchInc = /^#include\s+(<|")(.*?)(>|").*$/.exec(line.data);
            if (!matchInc) {
                throw new TypeError('could not match #include line: ' + line.data);
            }
            let incPath = matchInc[2];

            if (_self.includeExcludedMap[incPath]) {
                // Specific include files excluded from the
                // inlining / duplicate suppression process.
                emit(line);
                return;
            }

            if (included[incPath]) {
                // We suppress duplicate includes, both internal and
                // external, based on the assumption that includes are
                // not behind #if defined() checks.  This is the case for
                // Duktape (except for the include files excluded).
                emit('/* #include ' + incPath + ' -> already included */');
                return;
            }
            included[incPath] = true;

            // An include file is considered "internal" and is amalgamated
            // if it is found in the include path provided by the user.

            var incFile = _self.lookupInclude(incPath);
            if (incFile) {
                console.debug('include considered internal: ' + line.data + ' -> ' + incFile);
                emit('/* #include ' + incPath + ' */');
                processFile(readFile(incFile));
            } else {
                console.debug('include considered external: ' + line.data);
                emit(line);  // keep as is
            }
        });
    }

    if (prologueFileName) {
        stripLastNewline(normalizeNewlines(readFileUtf8(prologueFileName))).split('\n').forEach((line) => {
            emit(line);
        });
    }

    files.forEach((f) => {
        processFile(f);
    });

    return {
        result: res.join('\n') + '\n',
        metadata
    };
};

function combineSources(args) {
    var sourceFiles = args.sourceFiles;
    var includePaths = args.includePaths;
    var includeExcluded = args.includeExcluded || [];
    var prologueFileName = args.prologueFileName;
    var lineDirectives = !!args.lineDirectives;

    var comb = new CombineSource(includePaths, includeExcluded);
    includePaths.forEach((inc) => { comb.includePaths.push(inc); });

    // Read input files, add automatic #undefs.
    var files = sourceFiles.map((fn) => {
        let res = readFile(fn);
        comb.addAutomaticUndefs(res);
        return res;
    });

    // Combine and return.
    var combinedSource, metadata;
    ({ result: combinedSource, metadata } = comb.combineFiles(files, prologueFileName, lineDirectives));
    return { combinedSource, metadata };
}
exports.combineSources = combineSources;
