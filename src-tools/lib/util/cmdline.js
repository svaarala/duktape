/*
 *  Command line parsing using a simple command line syntax:
 *
 *      [generic options] [command] [command options and positionals]
 *
 *  Option forms:
 *
 *      --long-option value
 *      --long-option=value
 *      --long-option            (if option takes no value)
 *      -Svalue
 *      -S                       (if option takes no value)
 *
 *  A homegrown command line parser is used to minimize external dependencies
 *  (important when self-hosting the tooling with Duktape).
 */

'use strict';

const { createBareObject } = require('./bare');
const { layoutTextColumns } = require('./text_layout');
const { assert } = require('./assert');

// Create help text for a specific command.
function createCommandHelp(spec, command, commandSpec) {
    var res = [];
    res.push('Command: ' + command);
    res.push('');
    let columns = [ [], [], [], [] ];
    for (let k of Object.getOwnPropertyNames(commandSpec.options).sort()) {
        let v = commandSpec.options[k];
        columns[0].push('--' + k + (v.short ? '/-' + v.short : ''));
        if (typeof v.value === 'undefined') {
            columns[1].push('<' + v.type + '>');
        } else {
            columns[1].push('(no argument)');
        }
        columns[2].push('(default: ' + v.default + ')');
        columns[3].push(v.description || '');
    }
    let lines = layoutTextColumns(columns).map((x) => '    ' + x);
    res = res.concat(lines);
    return res.join('\n');
}

// Create help text for all commands, i.e. a command summary.
function createGenericHelp(spec) {
    var res = [];
    res.push('Commands:');
    res.push('');
    let columns = [ [], [] ];
    for (let k of Object.getOwnPropertyNames(spec.commands).sort()) {
        let v = spec.commands[k];
        columns[0].push(k);
        columns[1].push(v.description || '');
    }
    let lines = layoutTextColumns(columns).map((x) => '    ' + x);
    res = res.concat(lines);
    return res.join('\n');
}

// Create a bare object for holding option values based on default
// values.  If command spec has .optionsInitCallback, call it to
// initialize any option parsing state (hosted in the options object).
function initOptionDefaults(spec) {
    let res = createBareObject({});
    for (let k of Object.getOwnPropertyNames(spec.options).sort()) {
        let optSpec = spec.options[k];
        assert(optSpec);
        if (optSpec.default !== void 0) {
            res[k] = optSpec.default;
        }
    }
    if (typeof spec.optionsInitCallback === 'function') {
        spec.optionsInitCallback(res);
    }
    return res;
}

// True if option takes an argument (--foo 123), false if not (--foo).
function optionTakesArgument(optSpec) {
    return typeof optSpec.value === 'undefined';
}

// Add an option value to the active option set.
function addOptionValue(optSpec, opts, name, value) {
    let old = opts[name];
    if (optSpec.callback) {
        // Custom handling.
        optSpec.callback(value, opts);
        return;
    }
    if (typeof old === 'object' && old !== null && Array.isArray(old)) {
        old.push(value);
    } else {
        opts[name] = value;
    }
}

// Check required options.
function checkRequiredOptions(spec, opts) {
    let options = spec.options || {};
    for (let optName of Object.getOwnPropertyNames(options || {}).sort()) {
        let opt = options[optName];
        if (opt.required && !(optName in opts)) {
            throw new TypeError('missing required option --' + optName);
        }
    }
}

// Parse command line given a clean argv (excluding nodejs etc) and a
// global command/option specification object.
function parseCommandLine(argv, argSpec) {
    var genericOpts = initOptionDefaults(argSpec);
    var command = void 0;
    var commandOpts = void 0;
    var commandPositional = [];
    var genericSpec = assert(argSpec);
    var commandSpec = void 0;

    function parseLongOption(i, t) {
        let m1 = /^--([\w-]+?)=(.*?)$/.exec(t);
        let m2 = /^--([\w-]+?)$/.exec(t);
        let activeSpec = (command ? commandSpec : genericSpec);
        let activeOpts = (command ? commandOpts : genericOpts);
        let optName;
        let optValue;
        let retval = 1;

        if (m1) {
            // --long-name=value
            optName = m1[1];
            let optSpec = activeSpec.options[optName];
            if (!optSpec) {
                throw new TypeError('unknown option: ' + t);
            }
            if (optionTakesArgument(optSpec)) {
                optValue = m1[2];
            } else {
                throw new TypeError('argument given to an option requiring no value: ' + t);
            }
            addOptionValue(optSpec, activeOpts, optName, optValue);
        } else if (m2) {
            // --long-name value
            // --long-name
            optName = m2[1];
            let optSpec = activeSpec.options[optName];
            if (!optSpec) {
                throw new TypeError('unknown option: ' + t);
            }
            if (optionTakesArgument(optSpec)) {
                if (i + 1 >= argv.length) {
                    throw new TypeError('argument missing required value: ' + t);
                }
                retval = 2;
                optValue = argv[i + 1];
            } else {
                optValue = optSpec.value;
            }
            addOptionValue(optSpec, activeOpts, optName, optValue);
        } else {
            throw new TypeError('failed to parse argument: ' + t);
        }

        return retval;
    }

    function parseShortOption(i, t) {
        let activeSpec = (command ? commandSpec : genericSpec);
        let activeOpts = (command ? commandOpts : genericOpts);
        let retval = 1;

        assert(t.charAt(0) === '-');
        let optChar = t.charAt(1);
        if (optChar === '') {
            throw new TypeError('invalid short option: ' + t);
        }
        let optValue = t.substring(2);

        let optName;
        let optSpec;
        for (let k of Object.getOwnPropertyNames(activeSpec.options).sort()) {
            let tmpSpec = activeSpec.options[k];
            assert(tmpSpec);
            if (tmpSpec.short === optChar) {
                console.debug('short option ' + optChar + ' maps to long option ' + k);
                optName = k;
                optSpec = tmpSpec;
                break;
            }
        }
        if (!optName) {
            throw new TypeError('unknown short option: ' + t);
        }
        assert(optSpec);

        if (optionTakesArgument(optSpec)) {
            if (!optValue) {
                if (i + 1 >= argv.length) {
                    throw new TypeError('missing argument for option: ' + t);
                }
                optValue = argv[i + 1];
                retval = 2;
            }
        } else {
            if (optValue) {
                throw new TypeError('argument given to an option requiring no value: ' + t);
            }
        }

        addOptionValue(optSpec, activeOpts, optName, optValue);
        return retval;
    }

    for (var i = 0; i < argv.length;) {
        let t = argv[i];
        if (t === '--help' || t === '-h') {
            let help = (command ? createCommandHelp(argSpec, command, commandSpec) :
                                  createGenericHelp(argSpec));
            return { help };
        }
        if (t.startsWith('--')) {
            console.debug('parse long option at index ' + i + ': ' + t);
            i += parseLongOption(i, t);
        } else if (t.startsWith('-')) {
            console.debug('parse short option at index ' + i + ': ' + t);
            i += parseShortOption(i, t);
        } else {
            if (command === void 0) {
                console.debug('parse command at index ' + i + ': ' + t);
                command = t;
                if (!argSpec.commands[command]) {
                    throw new TypeError('unknown command: ' + command);
                }
                commandSpec = argSpec.commands[command];
                assert(commandSpec);
                commandOpts = initOptionDefaults(commandSpec);
            } else {
                console.debug('parse command positional at index ' + i + ': ' + t);
                commandPositional.push(t);
            }
            i++;
        }
    }

    // Check required options.
    if (genericSpec) {
        checkRequiredOptions(genericSpec, genericOpts);
    }
    if (commandSpec) {
        checkRequiredOptions(commandSpec, commandOpts);
    }

    return { genericOpts, command, commandOpts, commandPositional };
}
exports.parseCommandLine = parseCommandLine;
