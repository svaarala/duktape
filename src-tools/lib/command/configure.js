'use strict';

const { configureSources } = require('../configure/configure_sources');
const { pathJoin, readFileUtf8, readFileYaml } = require('../util/fs');
const { parse: parseYaml } = require('../util/yaml');
const { stripLastNewline, normalizeNewlines } = require('../util/string_util');
const { createBareObject } = require('../util/bare');
const { createTempDir } = require('../util/fs');
const { assert } = require('../util/assert');

const configureCommandSpec = createBareObject({
    description: 'Prepare Duktape source and header files for application build',
    optionsInitCallback: (opts) => {
        opts.forcedOptions = createBareObject({});
        opts.fixupLines = [];
        opts.userBuiltinFiles = [];
    },
    options: createBareObject({
        'source-directory': { type: 'path', default: void 0, description: 'Directory with raw input sources (defaulted based on script path)' },
        'output-directory': { type: 'path', default: void 0, required: true, description: 'Directory for output files (created automatically, must not exist)'},
        'temp-directory': { type: 'path', default: void 0, description: 'Force a specific temp directory (default is to create automatically)' },
        'config-directory': { type: 'path', default: void 0, description: 'Directory with config metadata (defaulted based on script path)' },
        'config-metadata': { type: 'path', default: void 0, deprecated: true, description: 'Deprecated version of --config-directory, do not use' },
        'option-file': { type: 'path', repeat: true, callback: (v, opts) => {
            let doc = readFileYaml(v);
            Object.assign(opts.forcedOptions, doc);
        }, description: 'YAML file(s) providing config option overrides (may be given multiple times)' },
        'option-yaml': { type: 'string', repeat: true, callback: (v, opts) => {
            let doc = parseYaml(v);
            Object.assign(opts.forcedOptions, doc);
        }, description: 'Override option(s) using inline YAML, e.g. --option-yaml "DUK_USE_DEEP_C_STACK: true"' },
        'define': { type: 'string', short: 'D', repeat: true, callback: (v, opts) => {
            let m;
            // -DFOO(a,b,c)=((a)+(b)+(c))
            m = /^([A-Z0-9_]+)\((.*?)\)=(.*?)$/.exec(v);
            if (m) {
                let defName = m[1];
                let defArgs = m[2];
                let defValue = m[3];
                opts.forcedOptions[defName] = { verbatim: '#define ' + defName + '(' + defArgs + ') ' + defValue };
                return;
            }
            // -DFOO=123
            // -DFOO="foo"
            m = /^([A-Z0-9_]+)=(.*?)$/.exec(v);
            if (m) {
                let defName = m[1];
                let defValue = m[2];
                opts.forcedOptions[defName] = defValue;
                return;
            }
            // -DFOO
            m = /^([A-Z0-9_]+)$/.exec(v);
            if (m) {
                let defName = m[1];
                opts.forcedOptions[defName] = true;
                return;
            }
            throw new TypeError('invalid --define option value: ' + v);
        }, description: 'Override option using a C compiler like syntax, e.g. "--define FOO", "-DFOO=123", or "-DFOO(a,b)=((a)+(b))"' },
        'undefine': { type: 'string', short: 'U', repeat: true, callback: (v, opts) => {
            // -UFOO
            // No other variants for undefining.
            let m;
            m = /^([A-Z0-9_]+)$/.exec(v);
            if (m) {
                let defName = m[1];
                opts.forcedOptions[defName] = false;
                return;
            }
            throw new TypeError('invalid --undefine option value: ' + v);
        }, description: 'Override option using a C compiler like syntax, e.g. "--undefine FOO" or "-UFOO"' },
        'fixup-line': { type: 'string', repeat: true, callback: (v, opts) => {
            opts.fixupLines.push(v);
        }, description: 'C header fixup line to be appended to generated header, e.g. --fixup-line "#define DUK_USE_FASTINT"' },
        'fixup-file': { type: 'path', repeat: true, callback: (v, opts) => {
            let data = stripLastNewline(normalizeNewlines(readFileUtf8(v)));
            data.split('\n').forEach((line) => {
                opts.fixupLines.push(line);
            });
        }, description: 'C header snippet file to be appended to generated header, useful for manual option fixups, e.g. --fixup-file my_fixups.h' },
        'builtin-file': { type: 'path', repeat: true, callback: (v, opts) => {
            opts.userBuiltinFiles.push(v);
        }, description: 'Built-in string/object YAML metadata to be applied over default built-ins (multiple files may be given, applied in sequence)' },
        'separate-sources': { type: 'boolean', default: false, value: true, deprecated: true, description: 'Output separate sources instead of amalgamated source (default is amalgamated)' },
        'line-directives': { type: 'boolean', default: false, value: true, description: 'Output #line directives in amalgamated source (default is false)' },
        'dll': { type: 'boolean', default: false, value: true, description: 'Enable DLL build of Duktape, affects symbol visibility macros especially on Windows' },
        'c99-types-only': { type: 'boolean', default: false, value: true, description: 'Assume C99 types, no legacy type detection' },
        'use-cpp-warning': { type: 'boolean', default: false, value: true, description: 'Emit a (non-portable) #warning when appropriate, e.g. for config setting mismatches' },
        'rom-support': { type: 'boolean', default: false, value: true, description: 'Add support for ROM strings/objects' },
        'rom-auto-lightfunc': { type: 'boolean', default: false, value: true, description: 'Convert ROM built-in function properties into lightfuncs automatically whenever possible' },
        'sanity-warning': { type: 'boolean', default: false, value: true, description: 'Emit a warning instead of #error for option sanity check issues (combine with --use-cpp-warning)' },
        'omit-removed-config-options': { type: 'boolean', default: false, value: true, description: 'Omit removed config options from generated headers' },
        'omit-deprecated-config-options': { type: 'boolean', default: false, value: true, description: 'Omit deprecated config options from generated headers' },
        'omit-unused-config-options': { type: 'boolean', default: false, value: true, description: 'Omit unused config options from generated headers' },
        'emit-config-sanity-check': { type: 'boolean', default: false, value: true, description: 'Emit preprocessor checks for config option consistency (DUK_USE_xxx)' },
        'emit-legacy-feature-check': { type: 'boolean', default: false, value: true, deprecated: true, description: 'Emit preprocessor checks to reject legacy feature options (DUK_OPT_xxx)' }
    })
});
exports.configureCommandSpec = configureCommandSpec;

function configureCommand(cmdline, autoDuktapeRoot) {
    var opts = cmdline.commandOpts;

    if (!autoDuktapeRoot && opts['source-directory']) {
        autoDuktapeRoot = pathJoin(opts['source-directory'], '..');
    }
    var sourceDirectory = opts['source-directory'] || pathJoin(autoDuktapeRoot, 'src-input');
    var outputDirectory = assert(opts['output-directory'], '--output-directory required');
    var configDirectory = opts['config-directory'] || opts['config-metadata'] || pathJoin(autoDuktapeRoot, 'config');
    var tempDirectory = assert(opts['temp-directory'] || createTempDir());
    var licenseFile = opts['license-file'] || pathJoin(autoDuktapeRoot, 'LICENSE.txt');
    var authorsFile= opts['authors-file'] || pathJoin(autoDuktapeRoot, 'AUTHORS.rst');
    var unicodeDataFile = opts['unicode-data'] || pathJoin(sourceDirectory, 'UnicodeData.txt');
    var specialCasingFile = opts['special-casing'] || pathJoin(sourceDirectory, 'SpecialCasing.txt');
    var forcedOptions = assert(opts.forcedOptions);
    var fixupLines = assert(opts.fixupLines);
    var userBuiltinFiles = assert(opts.userBuiltinFiles);

    if (opts['separate-sources']) {
        throw new TypeError('separate sources no longer supported');
    }
    if (opts['emit-legacy-feature-check']) {
        throw new TypeError('--emit-legacy-feature-check no longer supported');
    }
    if (opts['rom-support'] === false) {
        console.log('assuming --rom-support (now always enabled)');
        opts['rom-support'] = true;
    }

    return configureSources({
        sourceDirectory,
        outputDirectory,
        configDirectory,
        tempDirectory,
        licenseFile,
        authorsFile,
        unicodeDataFile,
        specialCasingFile,
        dukDistMetaFile: opts['duk-dist-meta'],
        gitCommit: opts['git-commit'],
        gitDescribe: opts['git-describe'],
        gitBranch: opts['git-branch'],
        platform: opts['platform'],
        architecture: opts['architecture'],
        compiler: opts['compiler'],
        forcedOptions,
        fixupLines,
        userBuiltinFiles,
        romAutoLightFunc: opts['rom-auto-lightfunc'],
        lineDirectives: opts['line-directives'],
        c99TypesOnly: opts['c99-types-only'],
        dll: opts['dll'],
        romSupport: opts['rom-support'],
        emitConfigSanityCheck: opts['emit-config-sanity-check'],
        sanityStrict: !opts['sanity-warning'],
        useCppWarning: opts['use-cpp-warning'],
        omitRemovedConfigOptions: opts['omit-removed-config-options'],
        omitDeprecatedConfigOptions: opts['omit-deprecated-config-options'],
        omitUnusedConfigOptions: opts['omit-unused-config-options']
    });
}
exports.configureCommand = configureCommand;
