// Generate duk_config.h.
//
// Based on genconfig.py.  No Duktape 1.x feature option support.

'use strict';

const { readFileYaml, pathJoin } = require('../util/fs');
const { Snippet, scanHelperSnippets } = require('./snippet');
const { FileBuilder } = require('./file_builder');
const { scanUseDefs } = require('./use_defs');
const { cIntEncode, cStrEncode } = require('../util/cquote');
const { createBareObject } = require('../util/bare');

// Assume these provides (as well as anything not prefixed 'DUK_') come from outside of duk_config.h.
const assumedProvides = {
    DUK_SINGLE_FILE: true,            // compiling Duktape from a single source file (duktape.c) version
    DUK_COMPILING_DUKTAPE: true,      // compiling Duktape (not user application)
    DUK_CONFIG_H_INCLUDED: true,      // artifact, include guard
    DUK_F_PACKED_TVAL_PROVIDED: true  // artifact, internal header signalling for DUK_USE_PACKED_TVAL
};

const platformRequiredProvides = {
    DUK_USE_OS_STRING: true
};

const architectureRequiredProvides = {
    DUK_USE_ARCH_STRING: true
};

const compilerRequiredProvides = {
    DUK_USE_COMPILER_STRING: true,
    DUK_USE_BRANCH_HINTS: true,
    DUK_USE_VARIADIC_MACROS: true,
    DUK_USE_UNION_INITIALIZERS: true
};

function validateFileProvides(absFn, provideMap) {
    var sn = new Snippet(absFn);

    for (let k of Object.getOwnPropertyNames(provideMap)) {
        if (!sn.provides[k]) {
            throw new TypeError('config file ' + absFn + ' is missing ' + k);
        }
    }
}

function filterUseDefs(useDefs, fn) {
    var useDefsList = Object.keys(useDefs).sort();
    return useDefsList.map((def) => useDefs[def]).filter(fn);
}

// Insert missing define dependencies into index 'idx_deps' repeatedly
// until no unsatisfied dependencies exist.  This is used to pull in
// the required DUK_F_xxx helper defines without pulling them all in.
// The resolution mechanism also ensures dependencies are pulled in the
// correct order; DUK_F_xxx helpers may depend on each other as long as
// there are no circular dependencies.
function fillDependenciesForSnippets(ret, idxDeps, baseDirectory) {
    var snippets = ret.vals;
    let provided = createBareObject({});

    // Scan available DUK_F_xxx helpers.

    let helpers = scanHelperSnippets(baseDirectory);

    // Certain defines are expected to come from outside, mark them
    // as 'provided'.

    for (let p of Object.keys(assumedProvides).sort()) {
        provided[p] = true;
    }

    // Figure out which snippets are needed to fulfill the missing defines.
    // This set contracts as snippets are added to the list, but may also
    // expand because added snippets depend on further defines.  At this point
    // we don't yet know which snippet include order is acceptable.

    function scanMissing() {
        let missing = createBareObject({});

        for (let sn of snippets) {
            for (let p of Object.keys(sn.provides).sort()) {
                provided[p] = true;
            }
            for (let r of Object.keys(sn.requires).sort()) {
                if (!provided[r]) {
                    // 'r' is required but not provided by anything yet.
                    // We expect 'r' to be a DUK_F_ snippet.  We would
                    // like to pull in 'r', but can only do so if it
                    // in turn doesn't depend on something.

                    missing[r] = true;
                }
            }
        }

        return missing;
    }

    let snippetList = [];
 missing_loop:
    for (;;) {
        let missing = scanMissing();
        let missingList = Object.keys(missing).sort();
        if (missingList.length === 0) {
            break;
        }
        for (let m of Object.keys(missing).sort()) {
            for (let i = 0; i < helpers.length; i++) {
                let sn = helpers[i];
                if (sn.provides[m]) {
                    snippetList.push(sn);
                    for (let p of Object.keys(sn.provides).sort()) {
                        provided[p] = true;
                    }
                    void helpers.splice(i, 1);
                    continue missing_loop;
                }
            }
        }
        throw new TypeError('failed to find snippet providers, still missing: ' + missingList.join(','));
    }

    // Now we have a list of snippets we need to include.  Include snippets in
    // an order that avoids any use-before-define.  Specific order doesn't
    // matter otherwise, but aim for repeatable order.

 snippet_loop:
    while (snippetList.length > 0) {
        for (let i = 0; i < snippetList.length; i++) {
            let sn = snippetList[i];
            for (let p of Object.keys(sn.requires).sort()) {
                if (!provided[p]) {
                    continue;
                }
            }
            ret.vals.splice(idxDeps++, 0, sn);
            ret.vals.splice(idxDeps++, 0, new Snippet([ '' ]));  // empty line
            void snippetList.splice(i, 1);
            continue snippet_loop;
        }
        throw new TypeError('failed to resolve snippet inclusion order');
    }
}

// If snippet provided (defined or undefined) DUK_USE_PACKED_TVAL,
// signal that fill-in is not needed.
function emitSnippetTvalCheck(ret, absFn) {
    let sn = new Snippet(absFn);
    ret.vals.push(sn);

    if (sn.provides['DUK_USE_PACKED_TVAL']) {
        ret.line('#define DUK_F_PACKED_TVAL_PROVIDED');
    }
}

// Emit autodetection ifdef checks for a platform, architecture, or compiler.
function emitAutodetectionHelper(ret, meta, subDirectory, configDirectory, requiredProvidesMap, comment) {
    meta.autodetect.forEach((elem, idx) => {
        var check = elem.check;
        var include = elem.include;
        var absFn = pathJoin(configDirectory, subDirectory, include);

        validateFileProvides(absFn, requiredProvidesMap);

        if (idx === 0) {
            ret.line('#if defined(' + check + ')');
        } else if (!check) {
            ret.line('#else');
        } else {
            ret.line('#elif defined(' + check + ')');
        }
        ret.line('/* --- ' + elem.name + ' --- */');

        emitSnippetTvalCheck(ret, absFn);
    });
    ret.line('#endif  /*' + comment + '*/');
}

function emitPlatform(ret, configDirectory, platform) {
    let absFn = pathJoin(configDirectory, 'platforms', 'platform_' + platform + '.h.in');
    ret.chdrBlockHeading('Platform: ' + platform);
    ret.snippetRelative('platform_cppextras.h.in');
    ret.empty();
    emitSnippetTvalCheck(ret, absFn);
}

function emitPlatformAutodetection(ret, platforms, configDirectory) {
    ret.chdrBlockHeading('Platform autodetection');
    ret.snippetRelative('platform_cppextras.h.in');
    ret.empty();
    emitAutodetectionHelper(ret, platforms, 'platforms', configDirectory, platformRequiredProvides, 'autodetect platform');
}

function emitArchitecture(ret, configDirectory, architecture) {
    let absFn = pathJoin(configDirectory, 'architectures', 'architecture_' + architecture + '.h.in');
    ret.chdrBlockHeading('Architecture: ' + architecture);
    emitSnippetTvalCheck(ret, absFn);
}

function emitArchitectureAutodetection(ret, architectures, configDirectory) {
    ret.chdrBlockHeading('Architecture autodetection');
    emitAutodetectionHelper(ret, architectures, 'architectures', configDirectory, architectureRequiredProvides, 'autodetect architecture');
}

function emitCompiler(ret, configDirectory, compiler) {
    let absFn = pathJoin(configDirectory, 'compilers', 'compiler_' + compiler + '.h.in');
    ret.chdrBlockHeading('Compiler: ' + compiler);
    emitSnippetTvalCheck(ret, absFn);
}

function emitCompilerAutodetection(ret, compilers, configDirectory) {
    ret.chdrBlockHeading('Compiler autodetection');
    emitAutodetectionHelper(ret, compilers, 'compilers', configDirectory, compilerRequiredProvides, 'autodetect compiler');
}

// Add a header snippet for checking consistency of DUK_USE_xxx config
// options, e.g. inconsistent options, invalid option values.
function emitConfigOptionChecks(ret, useDefs, sanityStrict) {
    ret.chdrBlockHeading('Checks for config option consistency (DUK_USE_xxx)');
    ret.empty();

    for (let doc of filterUseDefs(useDefs, () => true)) {
        if (doc.removed) {
            ret.line('#if defined(' + doc.define + ')');
            ret.cppWarningOrError('unsupported config option used (option has been removed): ' + doc.define, sanityStrict);
            ret.line('#endif');
        } else if (doc.deprecated) {
            ret.line('#if defined(' + doc.define + ')');
            ret.cppWarningOrError('unsupported config option used (option has been deprecatedd): ' + doc.define, sanityStrict);
            ret.line('#endif');
        }

        for (let req of (doc.requires || [])) {
            ret.line('#if defined(' + doc.define + ') && !defined(' + req + ')');
            ret.cppWarningOrError('config option ' + doc.define + ' requires option ' + req + ' (which is missing)', sanityStrict);
            ret.line('#endif');
        }
        for (let con of (doc.conflicts || [])) {
            ret.line('#if defined(' + doc.define + ') && defined(' + con + ')');
            ret.cppWarningOrError('config option ' + doc.define + ' conflicts with option ' + con + ' (which is also defined)', sanityStrict);
            ret.line('#endif');
        }
    }

    ret.empty();
    ret.snippetRelative('cpp_exception_sanity.h.in');
    ret.empty();
}

// Emit a #define / #undef for an option based on a config option metadata
// or forced value.
function emitOptionDefine(ret, defname, defval, undefDone, activeOpts, activeVals) {
    if (typeof defval === 'boolean' && defval === true) {
        ret.line('#define ' + defname);
        activeOpts[defname] = true;
        activeVals[defname] = true;
    } else if (typeof defval === 'boolean' && defval === false) {
        if (undefDone) {
            // Default value is false, and caller has emitted an
            // unconditional #undef, so don't emit a duplicate.
        } else {
            ret.line('#undef ' + defname);
        }
        activeOpts[defname] = false;
        activeVals[defname] = false;
    } else if (typeof defval === 'number') {
        if (Math.floor(defval) === defval) {
            ret.line('#define ' + defname + ' ' + cIntEncode(defval));
            activeOpts[defname] = true;
            activeVals[defname] = defval;
        } else {
            throw new TypeError('invalid define value: ' + defval);
        }
    } else if (typeof defval === 'string') {
        // verbatim value (not a string to be C-quoted)
        ret.line('#define ' + defname + ' ' + defval);
        activeOpts[defname] = true;
        activeVals[defname] = { verbatim: defval };
    } else if (typeof defval === 'object' && defval !== null) {
        if ('verbatim' in defval) {
            // verbatim text for the entire line
            ret.line(defval.verbatim);
            activeVals[defname] = { verbatim: defval.verbatim };
        } else if ('string' in defval) {
            // C string value
            ret.line('#define ' + defname + ' ' + cStrEncode(defval.string));
            activeVals[defname] = defval.string;
        } else {
            throw new TypeError('unsupported value for option ' + defname);
        }
        activeOpts[defname] = true;
    } else {
        throw new TypeError('unsupported value for option ' + defname);
    }
}

function emitDukUseOptions(ret, useDefs, forcedOpts, opts) {
    ret.chdrBlockHeading('Config options');

    let activeOpts = createBareObject({});
    let activeVals = createBareObject({});

    ret.line('/* Forced options */');

    function optFilter(doc) {
        if (doc.removed && opts.omitRemovedConfigOptions ||
            doc.deprecated && opts.omitDeprecatedConfigOptions ||
            doc.unused && opts.omitUnusedConfigOptions) {
            return false;
        }
        return true;
    }

    let tmp1 = new Snippet(ret.join().split('\n'));
    let forcedFound = createBareObject({});
    for (let doc of filterUseDefs(useDefs, optFilter)) {
        let defname = doc.define;
        if (!(defname in forcedOpts)) {
            continue;
        }
        if (!('default' in doc)) {
            throw new TypeError('config option ' + defname + ' is missing default value');
        }
        let undefDone = false;
        if (tmp1.provides[defname]) {
            ret.line('#undef ' + defname);
            undefDone = true;
        }

        emitOptionDefine(ret, defname, forcedOpts[defname], undefDone, activeOpts, activeVals);
        forcedFound[defname] = true;
    }
    for (let k in forcedOpts) {
        if (!forcedFound[k]) {
            throw new TypeError('unknown forced option: ' + k);
        }
    }

    ret.line('/* Default options */');

    let tmp2 = new Snippet(ret.join().split('\n'));
    for (let doc of filterUseDefs(useDefs, (doc) => !doc.removed)) {
        let defname = doc.define;
        if (tmp2.provides[defname]) {
            continue;
        }
        if (!('default' in doc)) {
            throw new TypeError('config option ' + defname + ' is missing default value');
        }

        emitOptionDefine(ret, defname, doc.default, false, activeOpts, activeVals);
    }

    return { activeOpts, activeVals };
}

function emitFixupLines(ret, fixupLines) {
    ret.chdrBlockHeading('Fixups');
    fixupLines.forEach((line) => {
        ret.line(line);
    });
}

function emitOverrideDefinesSection(ret) {
    ret.line('/*');
    ret.line(' *  You may add overriding #define/#undef directives below for');
    ret.line(' *  customization.  You of course cannot un-#include or un-typedef');
    ret.line(' *  anything; these require direct changes above.');
    ret.line(' */');
    ret.empty();
    ret.line('/* __OVERRIDE_DEFINES__ */');
    ret.empty();
}

function checkRecommendedOptions(ret, useDefs, forcedOpts) {
    for (let doc of filterUseDefs(useDefs, () => true)) {
        if (doc.warn_if_missing === true && typeof forcedOpts[doc.define] === 'undefined') {
            // Awkward handling for DUK_USE_CPP_EXCEPTIONS + DUK_USE_FATAL_HANDLER.
            // DUK_USE_FATAL_HANDLER is recommended, but only when not using C++ exceptions.
            if (doc.define === 'DUK_USE_FATAL_HANDLER' && typeof forcedOpts['DUK_USE_CPP_EXCEPTIONS'] !== 'undefined') {
                /* nop */
            } else {
                console.log('recommended config option ' + doc.define + ' not provided');
                ret.line('/* Recommended config option ' + doc.define + ' not provided */');
            }
        }
    }
}

function generateDukConfigHeader(opts) {
    var configDirectory = opts.configDirectory;
    var forcedOpts = opts.forcedOpts || {};
    Object.setPrototypeOf(forcedOpts, null);  // ensure bare

    var useDefs = scanUseDefs(pathJoin(configDirectory, 'config-options'));
    //var useDefsList = Object.keys(useDefs).sort();

    var platforms = readFileYaml(pathJoin(configDirectory, 'platforms.yaml'));
    var architectures = readFileYaml(pathJoin(configDirectory, 'architectures.yaml'));
    var compilers = readFileYaml(pathJoin(configDirectory, 'compilers.yaml'));

    var ret = new FileBuilder(pathJoin(configDirectory, 'header-snippets'), createBareObject({
        useCppWarning: opts.useCppWarning
    }));

    ret.line('/*');
    ret.line(' *  duk_config.h configuration header');
    ret.line(' *');
    ret.line(' *  Git commit: ' + opts.gitCommit);
    ret.line(' *  Git describe: ' + opts.gitDescribe);
    ret.line(' *  Git branch: ' + opts.gitBranch);
    ret.line(' *');
    if (typeof opts.platform === 'string') {
        ret.line(' *  Platform: ' + opts.platform);
    } else {
        ret.line(' *  Supported platforms:');
        for (let platf of platforms.autodetect) {
            ret.line(' *      - ' + (platf.name || platf.check));
        }
    }
    ret.line(' *');
    if (typeof opts.architecture === 'string') {
        ret.line(' *  Architecture: ' + opts.architecture);
    } else {
        ret.line(' *  Supported architectures:');
        for (let arch of architectures.autodetect) {
            ret.line(' *      - ' + (arch.name || arch.check));
        }
    }
    ret.line(' *');
    if (typeof opts.compiler === 'string') {
        ret.line(' *  Compiler: ' + opts.compiler);
    } else {
        ret.line(' *  Supported compilers:');
        for (let comp of compilers.autodetect) {
            ret.line(' *      - ' + (comp.name || comp.check));
        }
    }
    ret.line(' *');
    ret.line(' */');
    ret.empty();

    ret.line('#if !defined(DUK_CONFIG_H_INCLUDED)')
    ret.line('#define DUK_CONFIG_H_INCLUDED')
    ret.empty()

    // Warn about missing recommended options.
    checkRecommendedOptions(ret, useDefs, forcedOpts);
    ret.empty();

    ret.chdrBlockHeading('Intermediate helper defines')

    // DLL build affects visibility attributes on Windows but unfortunately
    // cannot be detected automatically from preprocessor defines or such.
    // DLL build status is hidden behind DUK_F_DLL_BUILD.
    if (opts.dll) {
        ret.line('/* Configured for DLL build. */');
        ret.line('#define DUK_F_DLL_BUILD');
    } else {
        ret.line('/* Not configured for DLL build. */');
        ret.line('#undef DUK_F_DLL_BUILD');
    }
    ret.empty();

    var idxDeps = ret.vals.length;  // Position where to emit DUK_F_xxx dependencies later.

    if (typeof opts.platform === 'string') {
        emitPlatform(ret, configDirectory, opts.platform);
    } else {
        emitPlatformAutodetection(ret, platforms, configDirectory);
    }

    ret.empty();
    ret.snippetRelative('platform_sharedincludes.h.in');
    ret.empty();

    if (typeof opts.architecture === 'string') {
        emitArchitecture(ret, configDirectory, opts.architecture);
    } else {
        emitArchitectureAutodetection(ret, architectures, configDirectory);
    }

    ret.empty();

    if (typeof opts.compiler === 'string') {
        emitCompiler(ret, configDirectory, opts.compiler);
    } else {
        emitCompilerAutodetection(ret, compilers, configDirectory);
    }

    ret.empty();

    // DUK_F_UCLIBC is special because __UCLIBC__ is provided by an #include
    // file, so the check must happen after platform includes.  It'd be nice
    // for this to be automatic (e.g. DUK_F_UCLIBC.h.in could indicate the
    // dependency somehow).

    ret.snippetAbsolute(pathJoin(configDirectory, 'helper-snippets', 'DUK_F_UCLIBC.h.in'));
    ret.empty();

    // Number typedefs.
    // XXX: Platform/compiler could provide types; if so, need some signaling
    // defines like DUK_F_TYPEDEFS_DEFINED.

    if (opts.c99TypesOnly) {
        ret.snippetRelative('types1.h.in');
        ret.line('/* C99 types assumed */');
        ret.snippetRelative('types_c99.h.in');
        ret.empty();
    } else {
        ret.snippetRelative('types1.h.in');
        ret.line('#if defined(DUK_F_HAVE_INTTYPES)');
        ret.line('/* C99 or compatible */');
        ret.empty();
        ret.snippetRelative('types_c99.h.in');
        ret.empty();
        ret.line('#else  /* C99 types */');
        ret.empty();
        ret.snippetRelative('types_legacy.h.in');
        ret.empty();
        ret.line('#endif  /* C99 types */');
        ret.empty();
    }
    ret.snippetRelative('types2.h.in');
    ret.empty();
    ret.snippetRelative('64bitops.h.in');
    ret.empty();

    // Platform, architecture, compiler fillins.  These are after all
    // detection so that e.g. DUK_SPRINTF() can be provided by platform
    // or compiler before trying a fill-in.

    ret.chdrBlockHeading('Fill-ins for platform, architecture, and compiler');

    ret.snippetRelative('platform_fillins.h.in');
    ret.empty();
    ret.snippetRelative('architecture_fillins.h.in');
    let emitByteOrderFillin = true;  // Could be omitted if byteorder provided by all active architecture files
    if (emitByteOrderFillin) {
        ret.empty();
        ret.snippetRelative('byteorder_fillin.h.in');
    }
    let emitAlignmentFillin = true;  // Could be omitted if alignment provided by all active architecture files
    if (emitAlignmentFillin) {
        ret.empty();
        ret.snippetRelative('alignment_fillin.h.in');
    }
    ret.empty();
    ret.snippetRelative('compiler_fillins.h.in');
    ret.empty();
    ret.snippetRelative('inline_workaround.h.in');
    ret.empty();
    let emitPackedTvalFillin = true;  // Could be omitted if packed tval provided by all active architecture files
    if (emitPackedTvalFillin) {
        ret.empty();
        ret.snippetRelative('packed_tval_fillin.h.in');
    }
    ret.empty();

    // Object layout.
    ret.snippetRelative('object_layout.h.in');
    ret.empty();

    // Detect and reject 'fast math'.
    ret.snippetRelative('reject_fast_math.h.in');
    ret.empty();

    // Emit DUK_USE_xxx options with user provided values or defaults.  If
    // an option is already provided by something above and user wants to
    // provide a value, #undef it first.
    let { activeOpts, activeVals } = emitDukUseOptions(ret, useDefs, forcedOpts, opts);
    ret.empty();

    // Fixups provided by user.
    if (typeof opts.fixupLines === 'object' && opts.fixupLines !== null && Array.isArray(opts.fixupLines)) {
        emitFixupLines(ret, opts.fixupLines);
        ret.empty();
    }

    // Add a section with __OVERRIDE_DEFINES__ for possible script edits.
    emitOverrideDefinesSection(ret);

    // Some headers are only included if final DUK_USE_xxx option settings
    // indicate they're needed, for example C++ <exception>.
    ret.chdrBlockHeading('Conditional includes');
    ret.snippetRelative('platform_conditionalincludes.h.in');
    ret.empty();

    // Date provider snippet is after custom header and overrides, so that
    // the user may define e.g. DUK_USE_DATE_NOW_GETTIMEOFDAY in their
    // custom header.
    ret.snippetRelative('date_provider.h.in');
    ret.empty();

    // Insert all required snippets now that we know what's needed.
    fillDependenciesForSnippets(ret, idxDeps, pathJoin(configDirectory, 'helper-snippets'));

    // Sanity checks.
    if (opts.emitConfigSanityCheck) {
        emitConfigOptionChecks(ret, useDefs, opts.sanityStrict);
    }

    // Derived defines (DUK_USE_INTEGER_LE, etc) from DUK_USE_BYTEORDER.
    // Duktape internals currently rely on the derived defines.  This is
    // after sanity checks because the derived defines are marked removed.
    ret.snippetRelative('byteorder_derived.h.in');
    ret.empty();

    ret.line('#endif  /* DUK_CONFIG_H_INCLUDED */');

    let hdrdata = ret.join().replace(/\n{2,}/g, '\n\n');  // squash multiple empty lines to one

    return {
        configHeaderString: hdrdata,
        activeOpts,
        activeVals
    };
}
exports.generateDukConfigHeader = generateDukConfigHeader;
