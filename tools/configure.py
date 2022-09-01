#!/usr/bin/env python
#
#  Compatibility stub which now executes JS-based tooling.
#
#  Should be Python2 and Python3 compatible.

import os
import sys
import time
import subprocess
import optparse
import yaml
import tempfile

def detect_nodejs():
    try:
        cmd = [ 'nodejs', '-e', 'console.log("test")' ]
        res = subprocess.check_output(cmd)
        if res[:4] == 'test'.encode('utf-8'):
            return 'nodejs'
    except:
        pass

    try:
        cmd = [ 'node', '-e', 'console.log("test")' ]
        res = subprocess.check_output(cmd)
        if res[:4] == 'test'.encode('utf-8'):
            return 'node'
    except:
        pass

    return None

def main():
    sys.stderr.write('\n')
    sys.stderr.write('****************************************************************************\n')
    sys.stderr.write('*** Duktape python tooling is obsolete, migrate to JS-based tooling!     ***\n')
    sys.stderr.write('*** This tool now internally invokes the JS-based tooling.               ***\n')
    sys.stderr.write('*** Minimum Node.js version is 14.x.                                     ***\n')
    sys.stderr.write('****************************************************************************\n')
    sys.stderr.write('\n')
    time.sleep(2)

    parser = optparse.OptionParser(
        usage='Usage: %prog [options]',
        description='Compatibility stub for JS-based tooling'
    )

    # Forced options from multiple sources are gathered into a shared list
    # so that the override order remains the same as on the command line.
    force_options_yaml = []
    def add_force_option_yaml(option, opt, value, parser):
        force_options_yaml.append(value)
    def add_force_option_file(option, opt, value, parser):
        with open(value, 'rb') as f:
            force_options_yaml.append(f.read())
    def add_force_option_define(option, opt, value, parser):
        defname, eq, defval = value.partition('=')
        if not eq:
            doc = { defname: True }
        else:
            defname, paren, defargs = defname.partition('(')
            if not paren:
                doc = { defname: defval }
            else:
                doc = { defname: { 'verbatim': '#define %s%s%s %s' % (defname, paren, defargs, defval) } }
        force_options_yaml.append(yaml.safe_dump(doc))
    def add_force_option_undefine(option, opt, value, parser):
        tmp = value.split('=')
        if len(tmp) == 1:
            doc = { tmp[0]: False }
        else:
            raise Exception('invalid option value: %r' % value)
        force_options_yaml.append(yaml.safe_dump(doc))

    fixup_header_lines = []
    def add_fixup_header_line(option, opt, value, parser):
        fixup_header_lines.append(value)
    def add_fixup_header_file(option, opt, value, parser):
        with open(value, 'rb') as f:
            for line in f:
                if line[-1] == '\n':
                    line = line[:-1]
                fixup_header_lines.append(line)

    # Log level options.
    parser.add_option('--quiet', dest='quiet', action='store_true', default=False, help='Suppress info messages (show warnings)')
    parser.add_option('--verbose', dest='verbose', action='store_true', default=False, help='Show verbose debug messages')

    # Options for configure.py tool itself.
    parser.add_option('--source-directory', dest='source_directory', default=None, help='Directory with raw input sources (defaulted based on configure.py script path)')
    parser.add_option('--output-directory', dest='output_directory', default=None, help='Directory for output files (created automatically if it doesn\'t exist, reused if safe)')
    parser.add_option('--license-file', dest='license_file', default=None, help='Source for LICENSE.txt (defaulted based on configure.py script path)')
    parser.add_option('--authors-file', dest='authors_file', default=None, help='Source for AUTHORS.rst (defaulted based on configure.py script path)')
    parser.add_option('--git-commit', dest='git_commit', default=None, help='Force git commit hash')
    parser.add_option('--git-describe', dest='git_describe', default=None, help='Force git describe')
    parser.add_option('--git-branch', dest='git_branch', default=None, help='Force git branch name')
    parser.add_option('--duk-dist-meta', dest='duk_dist_meta', default=None, help='duk_dist_meta.json to read git commit etc info from')

    # Options for combining sources.
    parser.add_option('--separate-sources', dest='separate_sources', action='store_true', default=False, help='Output separate sources instead of combined source (default is combined)')
    parser.add_option('--line-directives', dest='line_directives', action='store_true', default=False, help='Output #line directives in combined source (default is false)')

    # Options for built-ins.
    parser.add_option('--rom-support', dest='rom_support', action='store_true', help='Add support for ROM strings/objects (increases duktape.c size considerably)')
    parser.add_option('--rom-auto-lightfunc', dest='rom_auto_lightfunc', action='store_true', default=False, help='Convert ROM built-in function properties into lightfuncs automatically whenever possible')
    parser.add_option('--user-builtin-metadata', dest='obsolete_builtin_metadata', default=None, help=optparse.SUPPRESS_HELP)
    parser.add_option('--builtin-file', dest='builtin_files', metavar='FILENAME', action='append', default=[], help='Built-in string/object YAML metadata to be applied over default built-ins (multiple files may be given, applied in sequence)')

    # Options for Unicode.
    parser.add_option('--unicode-data', dest='unicode_data', default=None, help='Provide custom UnicodeData.txt')
    parser.add_option('--special-casing', dest='special_casing', default=None, help='Provide custom SpecialCasing.txt')

    # Options for genconfig.py.
    parser.add_option('--config-metadata', dest='config_metadata', default=None, help='metadata directory (defaulted based on configure.py script path)')
    parser.add_option('--platform', dest='platform', default=None, help='platform (default is autodetect)')
    parser.add_option('--compiler', dest='compiler', default=None, help='compiler (default is autodetect)')
    parser.add_option('--architecture', dest='architecture', default=None, help='architecture (default is autodetec)')
    parser.add_option('--c99-types-only', dest='c99_types_only', action='store_true', default=False, help='assume C99 types, no legacy type detection')
    parser.add_option('--dll', dest='dll', action='store_true', default=False, help='dll build of Duktape, affects symbol visibility macros especially on Windows')
    parser.add_option('--support-feature-options', dest='support_feature_options', action='store_true', default=False, help=optparse.SUPPRESS_HELP)
    parser.add_option('--emit-legacy-feature-check', dest='emit_legacy_feature_check', action='store_true', default=False, help='emit preprocessor checks to reject legacy feature options (DUK_OPT_xxx)')
    parser.add_option('--emit-config-sanity-check', dest='emit_config_sanity_check', action='store_true', default=False, help='emit preprocessor checks for config option consistency (DUK_USE_xxx)')
    parser.add_option('--omit-removed-config-options', dest='omit_removed_config_options', action='store_true', default=False, help='omit removed config options from generated headers')
    parser.add_option('--omit-deprecated-config-options', dest='omit_deprecated_config_options', action='store_true', default=False, help='omit deprecated config options from generated headers')
    parser.add_option('--omit-unused-config-options', dest='omit_unused_config_options', action='store_true', default=False, help='omit unused config options from generated headers')
    parser.add_option('--add-active-defines-macro', dest='add_active_defines_macro', action='store_true', default=False, help='add DUK_ACTIVE_DEFINES macro, for development only')
    parser.add_option('--define', type='string', metavar='OPTION', dest='force_options_yaml', action='callback', callback=add_force_option_define, default=force_options_yaml, help='force #define option using a C compiler like syntax, e.g. "--define DUK_USE_DEEP_C_STACK" or "--define DUK_USE_TRACEBACK_DEPTH=10"')
    parser.add_option('-D', type='string', metavar='OPTION', dest='force_options_yaml', action='callback', callback=add_force_option_define, default=force_options_yaml, help='synonym for --define, e.g. "-DDUK_USE_DEEP_C_STACK" or "-DDUK_USE_TRACEBACK_DEPTH=10"')
    parser.add_option('--undefine', type='string', metavar='OPTION', dest='force_options_yaml', action='callback', callback=add_force_option_undefine, default=force_options_yaml, help='force #undef option using a C compiler like syntax, e.g. "--undefine DUK_USE_DEEP_C_STACK"')
    parser.add_option('-U', type='string', metavar='OPTION', dest='force_options_yaml', action='callback', callback=add_force_option_undefine, default=force_options_yaml, help='synonym for --undefine, e.g. "-UDUK_USE_DEEP_C_STACK"')
    parser.add_option('--option-yaml', type='string', metavar='YAML', dest='force_options_yaml', action='callback', callback=add_force_option_yaml, default=force_options_yaml, help='force option(s) using inline YAML (e.g. --option-yaml "DUK_USE_DEEP_C_STACK: true")')
    parser.add_option('--option-file', type='string', metavar='FILENAME', dest='force_options_yaml', action='callback', callback=add_force_option_file, default=force_options_yaml, help='YAML file(s) providing config option overrides')
    parser.add_option('--fixup-file', type='string', metavar='FILENAME', dest='fixup_header_lines', action='callback', callback=add_fixup_header_file, default=fixup_header_lines, help='C header snippet file(s) to be appended to generated header, useful for manual option fixups')
    parser.add_option('--fixup-line', type='string', metavar='LINE', dest='fixup_header_lines', action='callback', callback=add_fixup_header_line, default=fixup_header_lines, help='C header fixup line to be appended to generated header (e.g. --fixup-line "#define DUK_USE_FASTINT")')
    parser.add_option('--sanity-warning', dest='sanity_strict', action='store_false', default=True, help='emit a warning instead of #error for option sanity check issues')
    parser.add_option('--use-cpp-warning', dest='use_cpp_warning', action='store_true', default=False, help='emit a (non-portable) #warning when appropriate')
    parser.add_option('--nodejs-command', dest='nodejs_command', default=None, help='Force Node.js command name')

    entry_cwd = os.getcwd()
    script_path = sys.path[0]  # http://stackoverflow.com/questions/4934806/how-can-i-find-scripts-directory-with-python

    (opts, args) = parser.parse_args()
    if len(args) > 0:
        raise Exception('unexpected arguments: %r' % args)

    if opts.obsolete_builtin_metadata is not None:
        raise Exception('--user-builtin-metadata has been removed, use --builtin-file instead')

    if opts.nodejs_command is None:
        nodejs_command = detect_nodejs()
    else:
        nodejs_command = opts.nodejs_command
    if nodejs_command is None:
        raise Exception('failed to detect Node.js, override with --nodejs-command')

    duktool_path = None
    for fn in [
        os.path.join(script_path, 'duktool.js'),
        os.path.join(script_path, '..', 'src-tools', 'index.js'),
        os.path.join(script_path, '..', 'src-tools', 'duktool.js')
    ]:
        if os.path.isfile(fn):
            duktool_path = fn
            break
    if duktool_path is None:
        raise Exception('could not find duktool.js or src-tools/index.js')

    cmd = [
        nodejs_command,
        duktool_path,
        'configure'
    ]
    if opts.output_directory is not None:
        cmd += [ '--output-directory', opts.output_directory ]
    if opts.source_directory is not None:
        cmd += [ '--source-directory', opts.source_directory ]
    else:
        src_dir = os.path.join(script_path, '..', 'src-input')
        if os.path.isdir(src_dir) and os.path.isfile(os.path.join(src_dir, 'duktape.h.in')):
            cmd += [ '--source-directory', src_dir ]
    if opts.config_metadata is not None:
        cmd += [ '--config-directory', opts.config_metadata ]

    forced = {}
    for i in force_options_yaml:
        doc = yaml.safe_load(i)
        for k,v in doc.items():
            forced[k] = v
    opts_fd, opts_fn = tempfile.mkstemp()
    with os.fdopen(opts_fd, 'wb') as f:
        f.write(yaml.safe_dump(forced).encode('utf-8'))
    cmd += [ '--option-file', opts_fn ]

    fixup_fd, fixup_fn = tempfile.mkstemp()
    with os.fdopen(fixup_fd, 'wb') as f:
        f.write(('\n'.join(fixup_header_lines) + '\n').encode('utf-8'))
    cmd += [ '--fixup-file', fixup_fn ]

    for i in opts.builtin_files:
        cmd += [ '--builtin-file', i ]

    if opts.line_directives:
        cmd += [ '--line-directives' ]

    if opts.platform is not None:
        cmd += [ '--platform', opts.platform ]
    if opts.compiler is not None:
        cmd += [ '--compiler', opts.compiler ]
    if opts.architecture is not None:
        cmd += [ '--architecture', opts.architecture ]

    if opts.dll:
        cmd += [ '--dll' ]
    if opts.c99_types_only:
        cmd += [ '--c99-types-only' ]

    sys.stderr.write('*** Executing JS-based tooling with command: ' + repr(cmd) + '\n\n')
    subprocess.check_call(cmd)

if __name__ == '__main__':
    main()
