/*
 *  Test case runner.  Supports both ECMAScript tests and Duktape API
 *  C tests.  Duktape API C tests are compiled on-the-fly against a
 *  dynamic or static library.
 *
 *  Error handling is currently not correct throughout.
 */

var fs = require('fs'),
    path = require('path'),
    tmp = require('tmp'),
    child_process = require('child_process'),
    async = require('async'),
    xml2js = require('xml2js'),
    md5 = require('MD5'),
    yaml = require('yamljs');

var TIMEOUT_SLOW_VALGRIND = 4 * 3600 * 1000;
var TIMEOUT_SLOW = 3600 * 1000;
var TIMEOUT_NORMAL_VALGRIND = 3600 * 1000;
var TIMEOUT_NORMAL = 600 * 1000;

// Global options from command line
var optPythonCommand;
var optPrepTestPath;
var optMinifyClosure;
var optMinifyUglifyJS;
var optMinifyUglifyJS2;
var optUtilIncludePath;
var optEmdukTrailingLineHack;
var knownIssues;

/*
 *  Utils.
 */

// Generate temporary filename, file will be autodeleted on exit unless
// deleted explicitly.  See: https://www.npmjs.com/package/tmp
function mkTempName(ext) {
    var fn = tmp.tmpNameSync({ keep: false, prefix: 'tmp-runtests-', postfix: (typeof ext === 'undefined' ? '' : '' + ext) });
    //console.log('mkTempName -> ' + fn);
    return fn;
}

function safeUnlinkSync(filePath) {
    try {
        if (filePath) {
            fs.unlinkSync(filePath);
        }
    } catch (e) {
        console.log('Failed to unlink ' + filePath + ' (ignoring): ' + e);
    }
}

function safeReadFileSync(filePath, encoding) {
    try {
        if (!filePath) {
            return;
        }
        return fs.readFileSync(filePath, encoding);
    } catch (e) {
        console.log('Failed to read ' + filePath + ' (ignoring): ' + e);
    }
}

function diffText(text1, text2, callback) {
    var tmp1 = mkTempName();
    var tmp2 = mkTempName();
    var cmd;

    fs.writeFileSync(tmp1, text1);
    fs.writeFileSync(tmp2, text2);
    cmd = [ 'diff', '-u', tmp1, tmp2 ];
    child = child_process.exec(cmd.join(' '), function diffDone(error, stdout, stderr) {
        safeUnlinkSync(tmp1);
        safeUnlinkSync(tmp2);
        callback(null, stdout);
    });
}

/*
 *  Parse a testcase file.
 */

function parseTestCaseSync(filePath) {
    var text = fs.readFileSync(filePath, 'utf-8');
    var pos, i1, i2;
    var meta = {};
    var tmp;
    var expect = '';

    i1 = text.indexOf('/*---'); i2 = text.indexOf('---*/');
    if (i1 >= 0 && i2 >= 0 && i2 >= i1) {
        meta = JSON.parse(text.substring(i1 + 5, i2));
    }

    pos = 0;
    for (;;) {
        i1 = text.indexOf('/*===', pos); i2 = text.indexOf('===*/', pos);
        if (i1 >= 0 && i2 >= 0 && i2 >= i1) {
            pos = i2 + 5;
            tmp = text.substring(i1 + 5, i2).split('\n').slice(1, -1);  // ignore first and last line
            expect += tmp.map(function (x) { return x + '\n'; }).join('');
        } else {
            break;
        }
    }

    return {
        filePath: filePath,
        name: path.basename(filePath, '.js'),
        fileName: path.basename(filePath),
        meta: meta,
        expect: expect,
        expect_md5: md5(expect)
    };
}

function addKnownIssueMetadata(testcase) {
    if (!knownIssues) { return; }
    knownIssues.forEach(function (v) {
        if (v.test !== testcase.fileName) { return; }
        testcase.meta = testcase.meta || {};
        if (v.knownissue) {
            testcase.meta.knownissue = v.knownissue;  // XXX: merge multiple
        }
        if (v.specialoptions) {
            testcase.meta.specialoptions = v.specialoptions;  // XXX: merge multiple
        }
    });
}

/*
 *  Execute a testcase with a certain engine, with optional valgrinding.
 */

function executeTest(options, callback) {
    var child;
    var cmd, cmdline;
    var execopts;
    var tempPrologue, tempInput, tempVgxml, tempVgout;
    var tempSource, tempExe;
    var timeout;

    // testcase execution done
    function execDone(error, stdout, stderr) {
        var res;

        // Emduk outputs an extra '\x20\0a' to end of stdout, strip it
        if (optEmdukTrailingLineHack &&
            typeof stdout === 'string' &&
            stdout.length >= 2 &&
            stdout.substring(stdout.length - 2) === ' \n') {
            stdout = stdout.substring(0, stdout.length - 2);
        }

        res = {
            testcase: options.testcase,
            engine: options.engine,
            error: error,
            stdout: stdout || '',
            stderr: stderr || '',
            cmdline: cmdline,
            execopts: execopts
        };

        res.valgrind_xml = safeReadFileSync(tempVgxml, 'utf-8');
        res.valgrind_out = safeReadFileSync(tempVgout, 'utf-8');

        safeUnlinkSync(tempPrologue);
        safeUnlinkSync(tempInput);
        safeUnlinkSync(tempVgxml);
        safeUnlinkSync(tempVgout);
        safeUnlinkSync(tempSource);
        safeUnlinkSync(tempExe);

        if (res.valgrind_xml &&
            res.valgrind_xml.substring(0, 5) === '<?xml' &&
            res.valgrind_xml.indexOf('</valgrindoutput>') > 0) {
            /* FIXME: Xml2js seems to not throw an error nor call the callback
             * in some cases (e.g. when a child is killed and xml output is
             * incomplete).  So, use a simple pre-check to guard against parsing
             * trivially broken XML.
             */
            try {
               xml2js.parseString(res.valgrind_xml, function (err, result) {
                    if (err) {
                        console.log(err);
                    } else {
                        res.valgrind_root = result;
                        res.valgring_json = JSON.stringify(result);
                    }
                    callback(null, res);
                });
            } catch (e) {
                console.log('xml2js parsing failed, should not happen: ' + e);
                callback(null, res);
            }
        } else {
            callback(null, res);
        }
    }

    // testcase compilation done (only relevant for API tests), ready to execute
    function compileDone(error, stdout, stderr) {
        /* FIXME: use child_process.spawn(); we don't currently escape command
         * line parameters which is risky.
          */

        if (error) {
            console.log(error);
            execDone(error);
            return;
        }

        cmd = [];
        if (options.valgrind) {
            tempVgxml = mkTempName();
            tempVgout = mkTempName();
            cmd = cmd.concat([ 'valgrind', '--tool=memcheck', '--xml=yes',
                               '--xml-file=' + tempVgxml,
                               '--log-file=' + tempVgout,
                               '--child-silent-after-fork=yes', '-q' ]);
        }
        if (tempExe) {
            cmd.push(tempExe);
        } else {
            cmd.push(options.engine.fullPath);
            if (!options.valgrind && options.engine.name === 'duk') {
                // cmd.push('--restrict-memory');  // restricted memory
            }
            // cmd.push('--alloc-logging');
            // cmd.push('--alloc-torture');
            // cmd.push('--alloc-hybrid');
            cmd.push(tempInput || options.testPath);
        }
        cmdline = cmd.join(' ');

        if (options.notimeout) {
            timeout = undefined;
        } else if (options.testcase.meta.slow) {
            timeout = options.valgrind ? TIMEOUT_SLOW_VALGRIND : TIMEOUT_SLOW;
        } else {
            timeout = options.valgrind ? TIMEOUT_NORMAL_VALGRIND : TIMEOUT_NORMAL;
        }
        execopts = {
            maxBuffer: 128 * 1024 * 1024,
            timeout: timeout,
            stdio: 'pipe',
            killSignal: 'SIGKILL'
        };

        //console.log(cmdline);
        child = child_process.exec(cmdline, execopts, execDone);
    }

    function compileApiTest() {
        tempSource = mkTempName('.c');
        try {
            fs.writeFileSync(tempSource, options.engine.cPrefix + fs.readFileSync(options.testPath));
        } catch (e) {
            console.log(e);
            callback(e);
            return;
        }
        tempExe = mkTempName();

        // FIXME: listing specific options here is awkward, must match Makefile
        cmd = [ 'gcc', '-o', tempExe,
                '-L.',
                '-Iprep/nondebug',  // this particularly is awkward
                '-Wl,-rpath,.',
                '-pedantic', '-ansi', '-std=c99', '-Wall', '-Wdeclaration-after-statement', '-fstrict-aliasing', '-D_POSIX_C_SOURCE=200809L', '-D_GNU_SOURCE', '-D_XOPEN_SOURCE', '-Os', '-fomit-frame-pointer',
                '-g', '-ggdb',
                //'-Werror',  // Would be nice but GCC differences break tests too easily
                //'-m32',
                'runtests/api_testcase_main.c',
                tempSource,
                '-lduktape',
                //'-lduktaped',
                '-lm'
        ];
        if (options.testcase.meta.pthread) {
            cmd.push('-lpthread');
        }

        cmdline = cmd.join(' ');
        execopts = {
            maxBuffer: 128 * 1024 * 1024,
            timeout: timeout,
            stdio: 'pipe',
            killSignal: 'SIGKILL'
        };

        console.log(options.testPath, cmdline);
        child = child_process.exec(cmdline, execopts, compileDone);
    }

    function prepareEcmaTest() {
        tempPrologue = mkTempName();
        tempInput = mkTempName();

        try {
            // The prefix is written to a temp file each time in case it needs
            // to be dynamic later (e.g. include test case or execution context info).
            fs.writeFileSync(tempPrologue, options.engine.jsPrefix || '/* no prefix */');
        } catch (e) {
            console.log(e);
            callback(e);
            return;
        }

        var args = [];
        args.push(optPrepTestPath)
        if (optMinifyClosure) {
            args.push('--minify-closure', optMinifyClosure);
        }
        if (optMinifyUglifyJS) {
            args.push('--minify-uglifyjs', optMinifyUglifyJS);
        }
        if (optMinifyUglifyJS2) {
            args.push('--minify-uglifyjs2', optMinifyUglifyJS2)
        }
        args.push('--util-include-path', optUtilIncludePath)
        args.push('--input', options.testPath)
        args.push('--output', tempInput)
        args.push('--prologue', tempPrologue)

        child_process.execFile(optPythonCommand, args, {}, compileDone)
    }

    if (options.engine.name === 'api') {
        compileApiTest();
    } else {
        prepareEcmaTest();
    }
}

/*
 *  Main
 */

/* The engine specific headers should not have a newline, to avoid
 * affecting line numbers in errors / tracebacks.
 */

var DUK_HEADER =
    "this.__engine__ = 'duk'; ";

// Note: Array.prototype.map() is required to support 'this' binding
// other than an array (arguments object here).
var NODEJS_HEADER =
    "this.__engine__ = 'v8'; " +
    "function print() {" +
    " var tmp = Array.prototype.map.call(arguments, function (x) { return String(x); });" +
    " var msg = tmp.join(' ') + '\\n';" +
    " process.stdout.write(msg);" +
    " } ";

var RHINO_HEADER =
    "this.__engine__ = 'rhino'; ";

var SMJS_HEADER =
    "this.__engine__ = 'smjs'; ";

var API_TEST_HEADER =
    "#include <stdio.h>\n" +
    "#include <stdlib.h>\n" +
    "#include <string.h>\n" +
    "#include <math.h>\n" +
    "#include <limits.h>  /* INT_MIN, INT_MAX */\n" +
    "#include \"duktape.h\"\n" +
    "\n" +
    "#define  TEST_SAFE_CALL(func)  do { \\\n" +
    "\t\tduk_ret_t _rc; \\\n" +
    "\t\tprintf(\"*** %s (duk_safe_call)\\n\", #func); \\\n" +
    "\t\tfflush(stdout); \\\n" +
    "\t\t_rc = duk_safe_call(ctx, (func), NULL, 0 /*nargs*/, 1 /*nrets*/); \\\n" +
    "\t\tprintf(\"==> rc=%d, result='%s'\\n\", (int) _rc, duk_safe_to_string(ctx, -1)); \\\n" +
    "\t\tfflush(stdout); \\\n" +
    "\t\tduk_pop(ctx); \\\n" +
    "\t} while (0)\n" +
    "\n" +
    "#define  TEST_PCALL(func)  do { \\\n" +
    "\t\tduk_ret_t _rc; \\\n" +
    "\t\tprintf(\"*** %s (duk_pcall)\\n\", #func); \\\n" +
    "\t\tfflush(stdout); \\\n" +
    "\t\tduk_push_c_function(ctx, (func), 0); \\\n" +
    "\t\t_rc = duk_pcall(ctx, 0); \\\n" +
    "\t\tprintf(\"==> rc=%d, result='%s'\\n\", (int) _rc, duk_safe_to_string(ctx, -1)); \\\n" +
    "\t\tfflush(stdout); \\\n" +
    "\t\tduk_pop(ctx); \\\n" +
    "\t} while (0)\n" +
    "\n" +
    "#line 1\n";

function findTestCasesSync(argList) {
    var found = {};
    var pat = /^([a-zA-Z0-9_-]+).(js|c)$/;
    var testcases = [];

    argList.forEach(function checkArg(arg) {
        var st = fs.statSync(arg);
        var m;

        if (st.isFile()) {
            m = pat.exec(path.basename(arg));
            if (!m) { return; }
            if (found[m[1]]) { return; }
            if (m[1].substring(0, 5) === 'util-') { return; }  // skip utils
            found[m[1]] = true;
            testcases.push(arg);
        } else if (st.isDirectory()) {
            fs.readdirSync(arg)
              .forEach(function check(fn) {
                  var m = pat.exec(fn);
                  if (!m) { return; }
                  if (found[m[1]]) { return; }
                  found[m[1]] = true;
                  testcases.push(path.join(arg, fn));
              });
        } else {
            throw new Exception('invalid argument: ' + arg);
        }
    });

    return testcases;
}

function adornString(x) {
    var stars = '********************************************************************************';
    return stars.substring(0, x.length + 8) + '\n' +
           '*** ' + x + ' ***' + '\n' +
           stars.substring(0, x.length + 8);
}

function prettyJson(x) {
    return JSON.stringify(x, null, 2);
}

function prettySnippet(x, label) {
    x = (x != null ? x : '');
    if (x.length > 0 && x[x.length - 1] != '\n') {
        x += '\n';
    }
    return '=== begin: ' + label + ' ===\n' +
           x +
           '=== end: ' + label + ' ===';
}

function getValgrindErrorSummary(root) {
    var res;
    var errors;

    if (!root || !root.valgrindoutput || !root.valgrindoutput.error) {
        return;
    }

    root.valgrindoutput.error.forEach(function vgError(e) {
        var k = e.kind[0];
        if (!res) {
            res = {};
        }
        if (!res[k]) {
            res[k] = 1;
        } else {
            res[k]++;
        }
    });

    return res;
}

function testRunnerMain() {
    var argv = require('optimist')
        .usage('Execute one or multiple test cases; dirname to execute all tests in a directory.')
        .default('num-threads', 4)
        .default('test-sleep', 0)
        .default('python-command', 'python2')
        .boolean('run-duk')
        .boolean('run-nodejs')
        .boolean('run-rhino')
        .boolean('run-smjs')
        .boolean('verbose')
        .boolean('report-diff-to-other')
        .boolean('valgrind')
        .boolean('emduk-trailing-line-hack')
        .describe('num-threads', 'number of threads to use for testcase execution')
        .describe('test-sleep', 'sleep time (milliseconds) between testcases, avoid overheating :)')
        .describe('python-command', 'python2 executable to use')
        .describe('run-duk', 'run testcase with Duktape')
        .describe('cmd-duk', 'path for "duk" command')
        .describe('run-nodejs', 'run testcase with Node.js (V8)')
        .describe('cmd-nodejs', 'path for Node.js command')
        .describe('run-rhino', 'run testcase with Rhino')
        .describe('cmd-rhino', 'path for Rhino command')
        .describe('run-smjs', 'run testcase with smjs')
        .describe('cmd-smjs', 'path for Spidermonkey executable')
        .describe('verbose', 'verbose test output')
        .describe('report-diff-to-other', 'report diff to other engines')
        .describe('valgrind', 'run duktape testcase with valgrind (no effect on other engines)')
        .describe('prep-test-path', 'path for test_prep.py')
        .describe('util-include-path', 'path for util-*.js files (tests/ecmascript usually)')
        .describe('minify-closure', 'path for closure compiler.jar')
        .describe('minify-uglifyjs', 'path for UglifyJS executable')
        .describe('minify-uglifyjs2', 'path for UglifyJS2 executable')
        .describe('known-issues', 'known issues yaml file')
        .describe('emduk-trailing-line-hack', 'strip bogus newline from end of emduk stdout')
        .demand('prep-test-path')
        .demand('util-include-path')
        .demand(1)   // at least 1 non-arg
        .argv;
    var testcases;
    var engines;
    var queue1, queue2;
    var results = {};  // testcase -> engine -> result
    var execStartTime, execStartQueue;

    function iterateResults(callback, filter_engname) {
        var testname, engname;

        for (testname in results) {
            for (engname in results[testname]) {
                if (filter_engname && engname !== filter_engname) {
                    continue;
                }
                res = results[testname][engname];
                callback(testname, engname, results[testname][engname]);
            }
        }
    }

    function queueExecTasks() {
        var tasks = [];

        testcases.forEach(function test(fullPath) {
            var filename = path.basename(fullPath);
            var testcase = parseTestCaseSync(fullPath);

            if (testcase.meta.skip) {
                // console.log('skip testcase: ' + testcase.name);
                return;
            }

            addKnownIssueMetadata(testcase);

            results[testcase.name] = {};  // create in test case order

            if (path.extname(fullPath) === '.c') {
                tasks.push({
                    engine: engine_api,
                    filename: filename,
                    testPath: fullPath,
                    testcase: testcase,
                    valgrind: argv.valgrind,
                    notimeout: argv['no-timeout']
                });
            } else {
                engines.forEach(function testWithEngine(engine) {
                    tasks.push({
                        engine: engine,
                        filename: filename,
                        testPath: fullPath,
                        testcase: testcase,
                        valgrind: argv.valgrind && (engine.name === 'duk'),
                        notimeout: argv['no-timeout']
                    });
                });
            }
        });

        if (tasks.length === 0) {
            console.log('No tasks to execute');
            process.exit(1);
        }

        console.log('Executing ' + testcases.length + ' testcase(s) with ' +
                    engines.length + ' engine(s) using ' + argv['num-threads'] + ' thread(s)' +
                    ', total ' + tasks.length + ' task(s)' +
                    (argv.valgrind ? ', valgrind enabled (for duk)' : ''));

        queue1.push(tasks);
    }

    function queueDiffTasks() {
        var tn, en, res;

        console.log('Testcase execution done, running diffs');

        iterateResults(function queueDiff(tn, en, res) {
            if (res.stdout !== res.testcase.expect) {
                queue2.push({
                    src: res.testcase.expect,
                    dst: res.stdout,
                    resultObject: res,
                    resultKey: 'diff_expect'
                });
            }
            if (en !== 'duk' || en === 'api') {
                return;
            }

            // duk-specific diffs
            engines.forEach(function diffToEngine(other) {
                if (other.name === 'duk') {
                    return;
                }
                if (results[tn][other.name].stdout === res.stdout) {
                    return;
                }
                if (!res.diff_other) {
                    res.diff_other = {}
                }
                queue2.push({
                    src: results[tn][other.name].stdout,
                    dst: res.stdout,
                    resultObject: res.diff_other,
                    resultKey: other.name
                });
            });
        }, null);
    }

    function analyzeResults() {
        var summary = { exitCode: 0 };
        iterateResults(function analyze(tn, en, res) {
            res.stdout_md5 = md5(res.stdout);
            res.stderr_md5 = md5(res.stderr);

            if (res.testcase.meta.skip) {
                res.status = 'skip';
            } else if (res.diff_expect) {
                if (!res.testcase.meta.knownissue && !res.testcase.meta.specialoptions) {
                    summary.exitCode = 1;
                }
                res.status = 'fail';
            } else {
                res.status = 'pass';
            }
        });
        return summary;
    }

    function printSummary() {
        var countPass = 0, countFail = 0, countSkip = 0;
        var lines = [];

        iterateResults(function summary(tn, en, res) {
            var parts = [];
            var diffs;
            var vgerrors;
            var need = false;

            if (en !== 'duk' && en !== 'api') {
                return;
            }

            vgerrors = getValgrindErrorSummary(res.valgrind_root);

            parts.push(res.testcase.name);
            parts.push(res.status);

            if (res.status === 'skip') {
                countSkip++;
            } else if (res.status === 'fail') {
                countFail++;
                parts.push(res.diff_expect.split('\n').length + ' diff lines');
                if (res.testcase.meta.knownissue) {
                    if (typeof res.testcase.meta.knownissue === 'string') {
                        parts.push('known issue: ' + res.testcase.meta.knownissue);
                    } else {
                        parts.push('known issue');
                    }
                }
                if (res.testcase.meta.specialoptions) {
                    if (typeof res.testcase.meta.specialoptions === 'string') {
                        parts.push('requires special feature options: ' + res.testcase.meta.specialoptions);
                    } else {
                        parts.push('requires special feature options');
                    }
                }
                if (res.testcase.meta.comment) {
                    parts.push('comment: ' + res.testcase.meta.comment);
                }
                need = true;
            } else {
                countPass++;

                diffs = [];

                engines.forEach(function checkDiffToOther(other) {
                    if (other.name === 'duk' ||
                        !res.diff_other || !res.diff_other[other.name]) {
                        return;
                    }
                    parts.push(other.name + ' diff ' + res.diff_other[other.name].split('\n').length + ' lines');
                    if (argv['report-diff-to-other']) {
                        need = true;
                    }
                });
           }
           if (vgerrors) {
               parts.push('valgrind ' + JSON.stringify(vgerrors));
               need = true;
           }
           if (need) {
               lines.push(parts);
           }
        }, null);

        lines.forEach(function printLine(line) {
            var tmp = ('                                                  ' + line[0]);
            tmp = tmp.substring(tmp.length - 50);
            console.log(tmp + ': ' + line.slice(1).join('; '));
        });

        console.log('');
        console.log('SUMMARY: ' + countPass + ' pass, ' + countFail +
                    ' fail');  // countSkip is no longer correct
    }

    function createLogFile(logFile) {
        var lines = [];

        iterateResults(function logResult(tn, en, res) {
            var desc = tn + '/' + en;
            lines.push(adornString(tn + ' ' + en));
            lines.push('');
            lines.push(prettyJson(res));
            lines.push('');
            lines.push(prettySnippet(res.stdout, 'stdout of ' + desc));
            lines.push('');
            lines.push(prettySnippet(res.stderr, 'stderr of ' + desc));
            lines.push('');
            lines.push(prettySnippet(res.testcase.expect, 'expect of ' + desc));
            lines.push('');
            if (res.diff_expect) {
                lines.push(prettySnippet(res.diff_expect, 'diff_expect of ' + desc));
                lines.push('');
            }
            if (res.diff_other) {
                for (other_name in res.diff_other) {
                    lines.push(prettySnippet(res.diff_other[other_name], 'diff_other ' + other_name + ' of ' + desc));
                    lines.push('');
                }
            }
        });

        fs.writeFileSync(logFile, lines.join('\n') + '\n');
    }

    optPythonCommand = argv['python-command'];

    if (argv['prep-test-path']) {
        optPrepTestPath = argv['prep-test-path'];
    } else {
        throw new Error('missing --prep-test-path');
    }

    if (argv['minify-closure']) {
        optMinifyClosure = argv['minify-closure'];
    }

    if (argv['minify-uglifyjs']) {
        optMinifyUglifyJS = argv['minify-uglifyjs'];
    }

    if (argv['minify-uglifyjs2']) {
        optMinifyUglifyJS2 = argv['minify-uglifyjs2'];
    }

    // Don't require a minifier here because we may be executing API testcases

    if (argv['util-include-path']) {
        optUtilIncludePath = argv['util-include-path'];
    } else {
        throw new Error('missing --util-include-path');
    }

    if (argv['known-issues']) {
        knownIssues = yaml.load(argv['known-issues']);
    }

    if (argv['emduk-trailing-line-hack']) {
        optEmdukTrailingLineHack = true;
    }

    engines = [];
    if (argv['run-duk']) {
        engines.push({ name: 'duk',
                       fullPath: argv['cmd-duk'] || 'duk' ,
                       jsPrefix: DUK_HEADER });
    }
    if (argv['run-nodejs']) {
        engines.push({ name: 'nodejs',
                       fullPath: argv['cmd-nodejs'] || 'node',
                       jsPrefix: NODEJS_HEADER });
    }
    if (argv['run-rhino']) {
        engines.push({ name: 'rhino',
                       fullPath: argv['cmd-rhino'] || 'rhino',
                       jsPrefix: RHINO_HEADER });
    }
    if (argv['run-smjs']) {
        engines.push({ name: 'smjs',
                       fullPath: argv['cmd-smjs'] || 'smjs',
                       jsPrefix: SMJS_HEADER });
    }
    engine_api = { name: 'api',
                   cPrefix: API_TEST_HEADER };

    testcases = findTestCasesSync(argv._);
    testcases.sort();
    //console.log(testcases);

    queue1 = async.queue(function (task, callback) {
        executeTest(task, function testDone(err, val) {
            var tmp;
            results[task.testcase.name][task.engine.name] = val;
            if (argv.verbose) {
                tmp = '        ' + task.engine.name + (task.valgrind ? '/vg' : '');
                console.log(tmp.substring(tmp.length - 8) + ': ' + task.testcase.name);
            }
            if (argv['test-sleep'] > 0) {
                setTimeout(callback, Number(argv['test-sleep']));
            } else {
                callback();
            }
        });
    }, argv['num-threads']);

    queue2 = async.queue(function (task, callback) {
        if (task.dummy) {
            callback();
            return;
        }
        diffText(task.src, task.dst, function (err, val) {
            task.resultObject[task.resultKey] = val;
            callback();
        });
    }, argv['num-threads']);

    queue1.drain = function() {
        // Second parallel step: run diffs
        queue2.push({ dummy: true });  // ensure queue is not empty
        queueDiffTasks();
    };

    queue2.drain = function() {
        // summary and exit
        var summary = analyzeResults();
        console.log('\n----------------------------------------------------------------------------\n');
        printSummary();
        console.log('\n----------------------------------------------------------------------------\n');
        if (argv['log-file']) {
            console.log('Writing test output to: ' + argv['log-file']);
            createLogFile(argv['log-file']);
        }
        console.log('All done.');
        process.exit(summary.exitCode);
    };

    // First parallel step: run testcases with selected engines
    queueExecTasks();

    // Periodic indication of how much to go
    execStartTime = new Date().getTime();
    execStartQueue = queue1.length();
    var timer = setInterval(function () {
        // not exact; not in queued != finished
        var now = new Date().getTime();
        var rate = (execStartQueue - queue1.length()) / ((now - execStartTime) / 1000);
        var eta = Math.ceil(queue1.length() / rate);
        console.log('Still running, testcase task queue length: ' + queue1.length() + ', eta ' + eta + ' second(s)');
    }, 10000);
}

testRunnerMain();
