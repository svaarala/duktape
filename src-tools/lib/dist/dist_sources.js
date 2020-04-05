// Based on Duktape 2.x util/dist.py.

'use strict';

const {
    pathJoin,
    getCwd,
    mkdir,
    dirname,
    copyFile,
    dirExists,
    fileExists,
    listDir,
    writeFileJsonPretty,
    readFileYaml
} = require('../util/fs');
const { getDukVersion } = require('../configure/duk_version');
const { getGitInfo } = require('../configure/git_info');
const { cStrEncode } = require('../util/cquote');
const { assert } = require('../util/assert');
const { configureSources } = require('../configure/configure_sources');
const { sourceFiles } = require('../configure/source_files');
const { copyFileUtf8AtSignReplace } = require('../configure/util');
const { validateRepoDirectory } = require('./util');
const { createBareObject } = require('../util/bare');
const { sortObjectKeysRecursive } = require('../util/sort');

function copyListCompare(a, b) {
    var an = a.target || a.relativeTarget || '';
    var bn = b.target || b.relativeTarget || '';
    if (an > bn) { return 1; }
    if (an < bn) { return -1; }
    return 0;
}

function straightCopy(filenames, source, target) {
    target = target || source;  // default target dir from source
    return filenames.map((fn) => {
        return { relativeSource: pathJoin(source, fn), relativeTarget: pathJoin(target, fn) };
    });
}

function straightCopyFlatDir(sourceDir, targetDir) {
    return listDir(sourceDir).map((fn) => {
        assert(fileExists(pathJoin(sourceDir, fn)));
        return {
            source: pathJoin(sourceDir, fn),
            target: pathJoin(targetDir, fn)
        };
    });
}

function getSourceFiles() {
    var res = [];

    res = res.concat(straightCopy(sourceFiles, 'src-input'));

    res = res.concat(straightCopy([
        'duktape.h.in',  // excluded from sourceFiles
        'builtins.yaml',
        'strings.yaml',
        'SpecialCasing.txt',
        'SpecialCasing-8bit.txt',
        'UnicodeData.txt',
        'UnicodeData-8bit.txt',
    ], 'src-input'));

    return res;
}

function getConfigFiles(args) {
    var repoDirectory = assert(args.repoDirectory);
    var outputDirectory = assert(args.outputDirectory);
    var res = [];

    res = res.concat(straightCopy([
        'tags.yaml',
        'platforms.yaml',
        'architectures.yaml',
        'compilers.yaml',
        'README.rst'
    ], 'config'));

    [
        'platforms',
        'architectures',
        'compilers',
        'feature-options',
        'config-options',
        'helper-snippets',
        'header-snippets',
        'examples'
    ].forEach((dir) => {
        res = res.concat(straightCopyFlatDir(pathJoin(repoDirectory, 'config', dir),
                                             pathJoin(outputDirectory, 'config', dir)));
    });

    return res;
}

function getToolsFiles() {
    return straightCopy([
        'configure.py',
        'combine_src.py',
        'create_spdx_license.py',
        'duk_meta_to_strarray.py',
        'dukutil.py',
        'dump_bytecode.py',
        'extract_caseconv.py',
        'extract_chars.py',
        'extract_unique_options.py',
        'genbuiltins.py',
        'genconfig.py',
        'merge_debug_meta.py',
        'prepare_unicode_data.py',
        'resolve_combined_lineno.py',
        'scan_strings.py',
        'scan_used_stridx_bidx.py',
    ], 'tools');
}

function getDebuggerFiles() {
    var res = [];

    res = res.concat(straightCopy([
        'README.rst',
        'Makefile',
        'package.json',
        'duk_debug.js',
        'duk_debug_proxy.js',
        'duk_classnames.yaml',
        'duk_debugcommands.yaml',
        'duk_debugerrors.yaml',
        'duk_opcodes.yaml'
    ], 'debugger'));

    res = res.concat(straightCopy([
        'index.html',
        'style.css',
        'webui.js'
    ], pathJoin('debugger', 'static')));

    return res;
}

function getPolyfillsFiles() {
    return straightCopy([
        'console-minimal.js',
        'global.js',
        'object-prototype-definegetter.js',
        'object-prototype-definesetter.js',
        'object-assign.js',
        'performance-now.js',
        'duktape-isfastint.js',
        'duktape-error-setter-writable.js',
        'duktape-error-setter-nonwritable.js',
        'duktape-buffer.js',
        'promise.js'
    ], 'polyfills');
}

function getExamplesFiles() {
    var res = [];

    res = res.concat(straightCopy([
        'README.rst'
    ], 'examples'));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_cmdline.c',
        'duk_cmdline.h',
        'duk_cmdline_lowmem.c'
    ], pathJoin('examples', 'cmdline')));

    res = res.concat(straightCopy([
        'README.rst',
        'c_eventloop.c',
        'c_eventloop.h',
        'c_eventloop.js',
        'ecma_eventloop.js',
        'main.c',
        'poll.c',
        'socket.c',
        'fileio.c',
        'basic-test.js',
        'timer-test.js',
        'server-socket-test.js',
        'client-socket-test.js'
    ], pathJoin('examples', 'eventloop')));

    res = res.concat(straightCopy([
        'README.rst',
        'hello.c'
    ], pathJoin('examples', 'hello')));

    res = res.concat(straightCopy([
        'README.rst',
        'eval.c'
    ], pathJoin('examples', 'eval')));

    res = res.concat(straightCopy([
        'README.rst',
        'fib.js',
        'process.js',
        'processlines.c',
        'prime.js',
        'primecheck.c',
        'uppercase.c'
    ], pathJoin('examples', 'guide')));

    res = res.concat(straightCopy([
        'README.rst',
        'globals.coffee',
        'hello.coffee',
        'mandel.coffee'
    ], pathJoin('examples', 'coffee')));

    res = res.concat(straightCopy([
        'README.rst',
        'jxpretty.c'
    ], pathJoin('examples', 'jxpretty')));

    res = res.concat(straightCopy([
        'README.rst',
        'sandbox.c'
    ], pathJoin('examples', 'sandbox')));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_alloc_logging.c',
        'duk_alloc_logging.h',
        'log2gnuplot.py'
    ], pathJoin('examples', 'alloc-logging')));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_alloc_torture.c',
        'duk_alloc_torture.h'
    ], pathJoin('examples', 'alloc-torture')));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_alloc_hybrid.c',
        'duk_alloc_hybrid.h'
    ], pathJoin('examples', 'alloc-hybrid')));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_trans_socket_unix.c',
        'duk_trans_socket_windows.c',
        'duk_trans_socket.h'
    ], pathJoin('examples', 'debug-trans-socket')));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_trans_dvalue.c',
        'duk_trans_dvalue.h',
        'test.c',
        'Makefile'
    ], pathJoin('examples', 'debug-trans-dvalue')));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_codepage_conv.c',
        'duk_codepage_conv.h',
        'test.c'
    ], pathJoin('examples', 'codepage-conv')));

    res = res.concat(straightCopy([
        'README.rst',
        'dummy_date_provider.c'
    ], pathJoin('examples', 'dummy-date-provider')));

    res = res.concat(straightCopy([
        'README.rst',
        'cpp_exceptions.cpp'
    ], pathJoin('examples', 'cpp-exceptions')));

    return res;
}

function getExtrasFiles() {
    var res = [];

    res = res.concat(straightCopy([
        'README.rst'
    ], 'extras'));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_logging.c',
        'duk_logging.h',
        'test.c',
        'Makefile'
    ], pathJoin('extras', 'logging')));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_v1_compat.c',
        'duk_v1_compat.h',
        'test.c',
        'Makefile',
        'test_eval1.js',
        'test_eval2.js',
        'test_compile1.js',
        'test_compile2.js'
    ], pathJoin('extras', 'duk-v1-compat')));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_print_alert.c',
        'duk_print_alert.h',
        'test.c',
        'Makefile'
    ], pathJoin('extras', 'print-alert')));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_console.c',
        'duk_console.h',
        'test.c',
        'Makefile'
    ], pathJoin('extras', 'console')));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_minimal_printf.c',
        'duk_minimal_printf.h',
        'Makefile',
        'test.c'
    ], pathJoin('extras', 'minimal-printf')));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_module_duktape.c',
        'duk_module_duktape.h',
        'Makefile',
        'test.c'
    ], pathJoin('extras', 'module-duktape')));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_module_node.c',
        'duk_module_node.h',
        'Makefile',
        'test.c'
    ], pathJoin('extras', 'module-node')));

    res = res.concat(straightCopy([
        'README.rst',
        'duk_alloc_pool.c',
        'duk_alloc_pool.h',
        'ptrcomp.yaml',
        'ptrcomp_fixup.h',
        'Makefile',
        'test.c'
    ], pathJoin('extras', 'alloc-pool')));

    res = res.concat(straightCopy([
        'README.rst',
        'cbordecode.py',
        'duk_cbor.c',
        'duk_cbor.h',
        'jsoncbor.c',
        'run_testvectors.js',
        'Makefile'
    ], pathJoin('extras', 'cbor')));

    return res;
}

function getToplevelFiles() {
    var res = [];

    res = res.concat(straightCopy([
        'Makefile.cmdline',
        'Makefile.dukdebug',
        'Makefile.eventloop',
        'Makefile.hello',
        'Makefile.eval',
        'Makefile.coffee',
        'Makefile.jxpretty',
        'Makefile.jsoncbor',
        'Makefile.sandbox',
        'Makefile.codepage',
        'mandel.js'
    ], 'dist-files', '.'));

    res = res.concat(straightCopy([
        'LICENSE.txt',
        'AUTHORS.rst'
    ], '.'));

    return res;
}

function getLicensesFiles() {
    return straightCopy([
        'murmurhash2.txt',
        'lua.txt',
        'commonjs.txt',
        'xoroshiro128plus.txt',
        'splitmix64.txt'
    ], 'licenses');
}

// Create a list of files to copy into the dist directory.  Directories are
// created automatically based on the file list.
function createDistCopyList(args) {
    //var repoDirectory = assert(args.repoDirectory);
    var distTempDirectory = assert(args.distTempDirectory);
    var outputDirectory = assert(args.outputDirectory);
    var res = [];

    res = res.concat(getSourceFiles());
    res = res.concat(getConfigFiles(args));
    res = res.concat(getToolsFiles());
    res = res.concat(getDebuggerFiles());
    res = res.concat(getPolyfillsFiles());
    res = res.concat(getExamplesFiles());
    res = res.concat(getExtrasFiles());
    res = res.concat(getToplevelFiles());
    res = res.concat(getLicensesFiles());

    // Special files prepared through temp files (absolute paths).
    res.push({
        source: pathJoin(distTempDirectory, 'Makefile.sharedlibrary'),
        target: pathJoin(outputDirectory, 'Makefile.sharedlibrary')
    });
    res.push({
        source: pathJoin(distTempDirectory, 'README.rst'),
        target: pathJoin(outputDirectory, 'README.rst')
    });
    res.push({
        source: pathJoin(distTempDirectory, 'duk_debug_meta.json'),
        target: pathJoin(outputDirectory, 'debugger', 'duk_debug_meta.json')
    });

    // RELEASES.rst is only updated in master.  It's not included in the dist to
    // make maintenance fixes easier to make.

    return res;
}

function convertPathsToAbsolute(copyList, repoDirectory, outputDirectory) {
    return copyList.map((ent) => {
        var res = {};
        if (ent.source) {
            res.source = ent.source;
        } else if (ent.relativeSource) {
            res.source = pathJoin(repoDirectory, ent.relativeSource);
        } else {
            throw new TypeError('missing .source or .relativeSource');
        }
        if (ent.target) {
            res.target = ent.target;
        } else if (ent.relativeTarget) {
            res.target = pathJoin(outputDirectory, ent.relativeTarget);
        } else {
            throw new TypeError('missing .target or .relativeTarget');
        }
        return res;
    });
}

function getDistDirectories(copyList) {
    var dirs = {};
    copyList.forEach((ent) => {
        assert(ent.target);
        let dir = dirname(ent.target);
        dirs[dir] = true;
    });
    var dirList = Object.getOwnPropertyNames(dirs).sort();
    return dirList;
}

function createDistDirectories(dirList) {
    dirList.forEach((dir) => {
        if (!dirExists(dir)) {
            mkdir(dir);
        }
    });
}

function createTempFiles(args) {
    var repoDirectory = args.repoDirectory;
    var distTempDirectory = args.distTempDirectory;
    var dukVersion = args.dukVersion;

    copyFileUtf8AtSignReplace(pathJoin(repoDirectory, 'dist-files', 'Makefile.sharedlibrary'),
                              pathJoin(distTempDirectory, 'Makefile.sharedlibrary'), {
                                  DUK_VERSION: String(dukVersion),
                                  SONAME_VERSION: String(Math.floor(dukVersion / 100))  // 10500 -> 105
                              });

    copyFileUtf8AtSignReplace(pathJoin(repoDirectory, 'dist-files', 'README.rst'),
                              pathJoin(distTempDirectory, 'README.rst'), {
                                  DUK_VERSION_FORMATTED: args.dukVersionFormatted,
                                  DUK_MAJOR: String(args.dukMajor),
                                  DUK_MINOR: String(args.dukMinor),
                                  DUK_PATCH: String(args.dukPatch),
                                  GIT_COMMIT: args.gitCommit,
                                  GIT_DESCRIBE: args.gitDescribe,
                                  GIT_BRANCH: args.gitBranch
                              });

    var debugMeta = createBareObject({});
    [
        'duk_classnames.yaml',
        'duk_debugcommands.yaml',
        'duk_debugerrors.yaml',
        'duk_opcodes.yaml'
    ].forEach((fn) => {
        let doc = readFileYaml(pathJoin(repoDirectory, 'debugger', fn));
        for (let k of Object.getOwnPropertyNames(doc).sort()) {
            debugMeta[k] = doc[k];
        }
    });
    debugMeta = sortObjectKeysRecursive(debugMeta);  // normalize order
    writeFileJsonPretty(pathJoin(distTempDirectory, 'duk_debug_meta.json'), debugMeta);
}

function copyDistFiles(copyList) {
    copyList.forEach((ent) => {
        assert(ent.source);
        assert(ent.target);
        copyFile(ent.source, ent.target);
    });
}

function distSources(args) {
    var repoDirectory = assert(args.repoDirectory);
    var outputDirectory = assert(args.outputDirectory);
    var tempDirectory = assert(args.tempDirectory);
    var distTempDirectory = pathJoin(tempDirectory, 'dist-tmp');
    var dukVersion, dukMajor, dukMinor, dukPatch, dukVersionFormatted;
    var gitCommit = args.gitCommit;
    var gitDescribe = args.gitDescribe;
    var gitBranch = args.gitBranch;
    var autoGitCommit, autoGitDescribe, autoGitBranch;
    var gitCommitCString, gitDescribeCString, gitBranchCString;
    var entryCwd = getCwd();
    void entryCwd;

    validateRepoDirectory(repoDirectory);

    // Preparations: directories, git info, Duktape version, etc.

    ({ dukVersion, dukMajor, dukMinor, dukPatch, dukVersionFormatted } =
        getDukVersion(pathJoin(repoDirectory, 'src-input', 'duktape.h.in')));
    console.debug({ dukVersion, dukMajor, dukMinor, dukPatch, dukVersionFormatted });

    ({ gitCommit: autoGitCommit, gitDescribe: autoGitDescribe, gitBranch: autoGitBranch } = getGitInfo());
    gitCommit = gitCommit || autoGitCommit;
    gitDescribe = gitDescribe || autoGitDescribe;
    gitBranch = gitBranch || autoGitBranch;
    gitCommitCString = cStrEncode(gitCommit);
    gitBranchCString = cStrEncode(gitBranch);
    gitDescribeCString = cStrEncode(gitDescribe);
    console.debug({ gitCommit, gitDescribe, gitBranch,
                    gitCommitCString, gitDescribeCString, gitBranchCString });

    // Create dist directory structure and copy files.
    mkdir(outputDirectory);
    mkdir(distTempDirectory);

    createTempFiles({
        repoDirectory,
        distTempDirectory,
        dukVersion,
        dukVersionFormatted,
        dukMajor,
        dukMinor,
        dukPatch,
        gitCommit,
        gitDescribe,
        gitBranch
    });

    var copyListRelative = createDistCopyList({
        repoDirectory,
        distTempDirectory,
        outputDirectory
    }).sort(copyListCompare);
    var copyListAbsolute = convertPathsToAbsolute(copyListRelative, repoDirectory, outputDirectory).sort(copyListCompare);
    var dirList = getDistDirectories(copyListAbsolute);
    createDistDirectories(dirList);
    copyDistFiles(copyListAbsolute);

    // Build preconfigured source(s).
    configureSources({
        sourceDirectory: pathJoin(repoDirectory, 'src-input'),
        outputDirectory: pathJoin(outputDirectory, 'src'),
        configDirectory: pathJoin(repoDirectory, 'config'),
        tempDirectory: tempDirectory,
        licenseFile: pathJoin(repoDirectory, 'LICENSE.txt'),
        authorsFile: pathJoin(repoDirectory, 'AUTHORS.rst'),
        unicodeDataFile: pathJoin(repoDirectory, 'src-input', 'UnicodeData.txt'),
        specialCasingFile: pathJoin(repoDirectory, 'src-input', 'SpecialCasing.txt'),
        gitCommit,
        gitDescribe,
        gitBranch,
        lineDirectives: false,
        c99TypesOnly: false,
        dll: false,
        forcedOptions: [],
        fixupLines: []
    });

    // Create duk_dist_meta.json.
    var distMeta = {
        'type': 'duk_dist_meta',
        'comment': 'Metadata for Duktape distributable',
        'git_commit': gitCommit,
        'git_branch': gitBranch,
        'git_describe': gitDescribe,
        'duk_version': dukVersion,
        'duk_version_string': dukVersionFormatted
    };
    //distMeta = sortObjectKeysRecursive(distMeta);
    writeFileJsonPretty(pathJoin(outputDirectory, 'duk_dist_meta.json'), distMeta);
}
exports.distSources = distSources;
