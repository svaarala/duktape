#!/usr/bin/env python2
#
#  Config-and-prepare: create a duk_config.h and combined/separate sources
#  for configuration options specified on the command line.
#

import os
import sys
import re
import shutil
import glob
import optparse
import tarfile
import json
import yaml
import subprocess

# Helpers

def exec_get_stdout(cmd, input=None, default=None, print_stdout=False):
    try:
        proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        ret = proc.communicate(input=input)
        if print_stdout:
            sys.stdout.write(ret[0])
            sys.stdout.flush()
        if proc.returncode != 0:
            sys.stdout.write(ret[1])  # print stderr on error
            sys.stdout.flush()
            if default is not None:
                print('WARNING: command %r failed, return default' % cmd)
                return default
            raise Exception('command failed, return code %d: %r' % (proc.returncode, cmd))
        return ret[0]
    except:
        if default is not None:
            print('WARNING: command %r failed, return default' % cmd)
            return default
        raise

def exec_print_stdout(cmd, input=None):
    ret = exec_get_stdout(cmd, input=input, print_stdout=True)

def mkdir(path):
    os.mkdir(path)

def copy_file(src, dst):
    with open(src, 'rb') as f_in:
        with open(dst, 'wb') as f_out:
            f_out.write(f_in.read())

def copy_files(filelist, srcdir, dstdir):
    for i in filelist:
        copy_file(os.path.join(srcdir, i), os.path.join(dstdir, i))

def copy_and_replace(src, dst, rules):
    # Read and write separately to allow in-place replacement
    keys = sorted(rules.keys())
    res = []
    with open(src, 'rb') as f_in:
        for line in f_in:
            for k in keys:
                line = line.replace(k, rules[k])
            res.append(line)
    with open(dst, 'wb') as f_out:
        f_out.write(''.join(res))

def copy_and_cquote(src, dst):
    with open(src, 'rb') as f_in:
        with open(dst, 'wb') as f_out:
            f_out.write('/*\n')
            for line in f_in:
                line = line.decode('utf-8')
                f_out.write(' *  ')
                for c in line:
                    if (ord(c) >= 0x20 and ord(c) <= 0x7e) or (c in '\x0a'):
                        f_out.write(c.encode('ascii'))
                    else:
                        f_out.write('\\u%04x' % ord(c))
            f_out.write(' */\n')

def read_file(src, strip_last_nl=False):
    with open(src, 'rb') as f:
        data = f.read()
        if len(data) > 0 and data[-1] == '\n':
            data = data[:-1]
        return data

def delete_matching_files(dirpath, cb):
    for fn in os.listdir(dirpath):
        if os.path.isfile(os.path.join(dirpath, fn)) and cb(fn):
            #print('Deleting %r' % os.path.join(dirpath, fn))
            os.unlink(os.path.join(dirpath, fn))

def create_targz(dstfile, filelist):
    # https://docs.python.org/2/library/tarfile.html#examples

    def _add(tf, fn):  # recursive add
        #print('Adding to tar: ' + fn)
        if os.path.isdir(fn):
            for i in sorted(os.listdir(fn)):
                _add(tf, os.path.join(fn, i))
        elif os.path.isfile(fn):
            tf.add(fn)
        else:
            raise Exception('invalid file: %r' % fn)

    with tarfile.open(dstfile, 'w:gz') as tf:
        for fn in filelist:
            _add(tf, fn)

def cstring(x):
    return '"' + x + '"'  # good enough for now

# DUK_VERSION is grepped from duk_api_public.h.in: it is needed for the
# public API and we want to avoid defining it in two places.
def get_duk_version(apiheader_filename):
    r = re.compile(r'^#define\s+DUK_VERSION\s+(.*?)L?\s*$')
    with open(apiheader_filename, 'rb') as f:
        for line in f:
            m = r.match(line)
            if m is not None:
                duk_version = int(m.group(1))
                duk_major = duk_version / 10000
                duk_minor = (duk_version % 10000) / 100
                duk_patch = duk_version % 100
                duk_version_formatted = '%d.%d.%d' % (duk_major, duk_minor, duk_patch)
                return duk_version, duk_major, duk_minor, duk_patch, duk_version_formatted

    raise Exception('cannot figure out duktape version')

# Python module check and friendly errors

def check_python_modules():
    # make_dist.py doesn't need yaml but other dist utils will; check for it and
    # warn if it is missing.
    failed = False

    def _warning(module, aptPackage, pipPackage):
        sys.stderr.write('\n')
        sys.stderr.write('*** NOTE: Could not "import %s" needed for dist.  Install it using e.g.:\n' % module)
        sys.stderr.write('\n')
        sys.stderr.write('    # Linux\n')
        sys.stderr.write('    $ sudo apt-get install %s\n' % aptPackage)
        sys.stderr.write('\n')
        sys.stderr.write('    # Windows\n')
        sys.stderr.write('    > pip install %s\n' % pipPackage)

    try:
        import yaml
    except ImportError:
        _warning('yaml', 'python-yaml', 'PyYAML')
        failed = True

    if failed:
        sys.stderr.write('\n')
        raise Exception('Missing some required Python modules')

check_python_modules()

# Option parsing

def main():
    parser = optparse.OptionParser()

    # Forced options from multiple sources are gathered into a shared list
    # so that the override order remains the same as on the command line.
    force_options_yaml = []
    def add_force_option_yaml(option, opt, value, parser):
        # XXX: check that YAML parses
        force_options_yaml.append(value)
    def add_force_option_file(option, opt, value, parser):
        # XXX: check that YAML parses
        with open(value, 'rb') as f:
            force_options_yaml.append(f.read())
    def add_force_option_define(option, opt, value, parser):
        tmp = value.split('=')
        if len(tmp) == 1:
            doc = { tmp[0]: True }
        elif len(tmp) == 2:
            doc = { tmp[0]: tmp[1] }
        else:
            raise Exception('invalid option value: %r' % value)
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

    # Options for config-and-prepare tool itself.
    parser.add_option('--source-directory', dest='source_directory', default=None, help='Directory with raw input sources (src-input/)')
    parser.add_option('--output-directory', dest='output_directory', default=None, help='Directory for output files, must already exist')
    parser.add_option('--duk-build-meta', dest='duk_build_meta', default=None, help='duk_build_meta.json for git commit info etc')
    parser.add_option('--git-commit', dest='git_commit', default=None, help='Force git commit hash')
    parser.add_option('--git-describe', dest='git_describe', default=None, help='Force git describe')
    parser.add_option('--git-branch', dest='git_branch', default=None, help='Force git branch name')

    # Options forwarded to genbuiltins.py.
    parser.add_option('--rom-support', dest='rom_support', action='store_true', help='Add support for ROM strings/objects (increases duktape.c size considerably)')
    parser.add_option('--rom-auto-lightfunc', dest='rom_auto_lightfunc', action='store_true', default=False, help='Convert ROM built-in function properties into lightfuncs automatically whenever possible')
    parser.add_option('--user-builtin-metadata', dest='user_builtin_metadata', action='append', default=[], help='User strings and objects to add, YAML format (can be repeated for multiple overrides)')

    # Options forwarded to genconfig.py.
    parser.add_option('--config-metadata', dest='config_metadata', default=None, help='metadata directory or metadata tar.gz file')
    parser.add_option('--platform', dest='platform', default=None, help='platform (default is autodetect)')
    parser.add_option('--compiler', dest='compiler', default=None, help='compiler (default is autodetect)')
    parser.add_option('--architecture', dest='architecture', default=None, help='architecture (default is autodetec)')
    parser.add_option('--c99-types-only', dest='c99_types_only', action='store_true', default=False, help='assume C99 types, no legacy type detection')
    parser.add_option('--dll', dest='dll', action='store_true', default=False, help='dll build of Duktape, affects symbol visibility macros especially on Windows')
    parser.add_option('--support-feature-options', dest='support_feature_options', action='store_true', default=False, help='support DUK_OPT_xxx feature options in duk_config.h')
    parser.add_option('--emit-legacy-feature-check', dest='emit_legacy_feature_check', action='store_true', default=False, help='emit preprocessor checks to reject legacy feature options (DUK_OPT_xxx)')
    parser.add_option('--emit-config-sanity-check', dest='emit_config_sanity_check', action='store_true', default=False, help='emit preprocessor checks for config option consistency (DUK_OPT_xxx)')
    parser.add_option('--omit-removed-config-options', dest='omit_removed_config_options', action='store_true', default=False, help='omit removed config options from generated headers')
    parser.add_option('--omit-deprecated-config-options', dest='omit_deprecated_config_options', action='store_true', default=False, help='omit deprecated config options from generated headers')
    parser.add_option('--omit-unused-config-options', dest='omit_unused_config_options', action='store_true', default=False, help='omit unused config options from generated headers')
    parser.add_option('--add-active-defines-macro', dest='add_active_defines_macro', action='store_true', default=False, help='add DUK_ACTIVE_DEFINES macro, for development only')
    parser.add_option('--define', type='string', dest='force_options_yaml', action='callback', callback=add_force_option_define, default=force_options_yaml, help='force #define option using a C compiler like syntax, e.g. "--define DUK_USE_DEEP_C_STACK" or "--define DUK_USE_TRACEBACK_DEPTH=10"')
    parser.add_option('-D', type='string', dest='force_options_yaml', action='callback', callback=add_force_option_define, default=force_options_yaml, help='synonym for --define, e.g. "-DDUK_USE_DEEP_C_STACK" or "-DDUK_USE_TRACEBACK_DEPTH=10"')
    parser.add_option('--undefine', type='string', dest='force_options_yaml', action='callback', callback=add_force_option_undefine, default=force_options_yaml, help='force #undef option using a C compiler like syntax, e.g. "--undefine DUK_USE_DEEP_C_STACK"')
    parser.add_option('-U', type='string', dest='force_options_yaml', action='callback', callback=add_force_option_undefine, default=force_options_yaml, help='synonym for --undefine, e.g. "-UDUK_USE_DEEP_C_STACK"')
    parser.add_option('--option-yaml', type='string', dest='force_options_yaml', action='callback', callback=add_force_option_yaml, default=force_options_yaml, help='force option(s) using inline YAML (e.g. --option-yaml "DUK_USE_DEEP_C_STACK: true")')
    parser.add_option('--option-file', type='string', dest='force_options_yaml', action='callback', callback=add_force_option_file, default=force_options_yaml, help='YAML file(s) providing config option overrides')
    parser.add_option('--fixup-file', type='string', dest='fixup_header_lines', action='callback', callback=add_fixup_header_file, default=fixup_header_lines, help='C header snippet file(s) to be appended to generated header, useful for manual option fixups')
    parser.add_option('--fixup-line', type='string', dest='fixup_header_lines', action='callback', callback=add_fixup_header_line, default=fixup_header_lines, help='C header fixup line to be appended to generated header (e.g. --fixup-line "#define DUK_USE_FASTINT")')
    parser.add_option('--sanity-warning', dest='sanity_strict', action='store_false', default=True, help='emit a warning instead of #error for option sanity check issues')
    parser.add_option('--use-cpp-warning', dest='use_cpp_warning', action='store_true', default=False, help='emit a (non-portable) #warning when appropriate')

    (opts, args) = parser.parse_args()

    assert(opts.source_directory)
    srcdir = opts.source_directory
    assert(opts.output_directory)
    outdir = opts.output_directory

    # Figure out directories, git info, etc

    entry_pwd = os.getcwd()

    duk_build_meta = None
    if opts.duk_build_meta is not None:
        with open(opts.duk_build_meta, 'rb') as f:
            duk_build_meta = json.loads(f.read())

    duk_version, duk_major, duk_minor, duk_patch, duk_version_formatted = \
        get_duk_version(os.path.join(srcdir, 'duk_api_public.h.in'))

    git_commit = None
    git_branch = None
    git_describe = None

    if duk_build_meta is not None:
        git_commit = duk_build_meta['git_commit']
        git_branch = duk_build_meta['git_branch']
        git_describe = duk_build_meta['git_describe']
    else:
        print('No --duk-build-meta, git commit information determined automatically')

    if opts.git_commit is not None:
        git_commit = opts.git_commit
    if opts.git_describe is not None:
        git_describe = opts.git_describe
    if opts.git_branch is not None:
        git_branch = opts.git_branch

    if git_commit is None:
        git_commit = exec_get_stdout([ 'git', 'rev-parse', 'HEAD' ], default='external').strip()
    if git_describe is None:
        git_describe = exec_get_stdout([ 'git', 'describe', '--always', '--dirty' ], default='external').strip()
    if git_branch is None:
        git_branch = exec_get_stdout([ 'git', 'rev-parse', '--abbrev-ref', 'HEAD' ], default='external').strip()

    git_commit = str(git_commit)
    git_describe = str(git_describe)
    git_branch = str(git_branch)

    git_commit_cstring = cstring(git_commit)
    git_describe_cstring = cstring(git_describe)
    git_branch_cstring = cstring(git_branch)

    print('Config-and-prepare for Duktape version %s, commit %s, describe %s, branch %s' % \
          (duk_version_formatted, git_commit, git_describe, git_branch))

    # For now, create the src/, src-noline/, and src-separate/ structure into the
    # output directory.  Later on the output directory should get the specific
    # variant output directly.
    mkdir(os.path.join(outdir, 'src'))
    mkdir(os.path.join(outdir, 'src-noline'))
    mkdir(os.path.join(outdir, 'src-separate'))

    # Separate sources are mostly copied as is at present.
    copy_files([
        'duk_alloc_default.c',
        'duk_api_internal.h',
        'duk_api_stack.c',
        'duk_api_heap.c',
        'duk_api_buffer.c',
        'duk_api_call.c',
        'duk_api_codec.c',
        'duk_api_compile.c',
        'duk_api_bytecode.c',
        'duk_api_memory.c',
        'duk_api_object.c',
        'duk_api_string.c',
        'duk_api_time.c',
        'duk_api_debug.c',
        'duk_bi_array.c',
        'duk_bi_boolean.c',
        'duk_bi_buffer.c',
        'duk_bi_date.c',
        'duk_bi_date_unix.c',
        'duk_bi_date_windows.c',
        'duk_bi_duktape.c',
        'duk_bi_error.c',
        'duk_bi_function.c',
        'duk_bi_global.c',
        'duk_bi_json.c',
        'duk_bi_math.c',
        'duk_bi_number.c',
        'duk_bi_object.c',
        'duk_bi_pointer.c',
        'duk_bi_protos.h',
        'duk_bi_regexp.c',
        'duk_bi_string.c',
        'duk_bi_proxy.c',
        'duk_bi_thread.c',
        'duk_bi_thrower.c',
        'duk_debug_fixedbuffer.c',
        'duk_debug.h',
        'duk_debug_macros.c',
        'duk_debug_vsnprintf.c',
        'duk_error_augment.c',
        'duk_error.h',
        'duk_error_longjmp.c',
        'duk_error_macros.c',
        'duk_error_misc.c',
        'duk_error_throw.c',
        'duk_forwdecl.h',
        'duk_harray.h',
        'duk_hbuffer_alloc.c',
        'duk_hbuffer.h',
        'duk_hbuffer_ops.c',
        'duk_hcompfunc.h',
        'duk_heap_alloc.c',
        'duk_heap.h',
        'duk_heap_hashstring.c',
        'duk_heaphdr.h',
        'duk_heap_markandsweep.c',
        'duk_heap_memory.c',
        'duk_heap_misc.c',
        'duk_heap_refcount.c',
        'duk_heap_stringcache.c',
        'duk_heap_stringtable.c',
        'duk_hnatfunc.h',
        'duk_hobject_alloc.c',
        'duk_hobject_class.c',
        'duk_hobject_enum.c',
        'duk_hobject_finalizer.c',
        'duk_hobject.h',
        'duk_hobject_misc.c',
        'duk_hobject_pc2line.c',
        'duk_hobject_props.c',
        'duk_hstring.h',
        'duk_hstring_misc.c',
        'duk_hthread_alloc.c',
        'duk_hthread_builtins.c',
        'duk_hthread.h',
        'duk_hthread_misc.c',
        'duk_hthread_stacks.c',
        'duk_hbufobj.h',
        'duk_hbufobj_misc.c',
        'duk_debugger.c',
        'duk_debugger.h',
        'duk_internal.h',
        'duk_jmpbuf.h',
        'duk_exception.h',
        'duk_js_bytecode.h',
        'duk_js_call.c',
        'duk_js_compiler.c',
        'duk_js_compiler.h',
        'duk_js_executor.c',
        'duk_js.h',
        'duk_json.h',
        'duk_js_ops.c',
        'duk_js_var.c',
        'duk_lexer.c',
        'duk_lexer.h',
        'duk_numconv.c',
        'duk_numconv.h',
        'duk_regexp_compiler.c',
        'duk_regexp_executor.c',
        'duk_regexp.h',
        'duk_tval.c',
        'duk_tval.h',
        'duk_unicode.h',
        'duk_unicode_support.c',
        'duk_unicode_tables.c',
        'duk_util_bitdecoder.c',
        'duk_util_bitencoder.c',
        'duk_util.h',
        'duk_util_hashbytes.c',
        'duk_util_hashprime.c',
        'duk_util_misc.c',
        'duk_util_tinyrandom.c',
        'duk_util_bufwriter.c',
        'duk_selftest.c',
        'duk_selftest.h',
        'duk_strings.h',
        'duk_replacements.c',
        'duk_replacements.h'
    ], srcdir, os.path.join(outdir, 'src-separate'))

    # Build temp versions of LICENSE.txt and AUTHORS.rst for embedding into
    # autogenerated C/H files.

    # XXX: use a proper temp directory

    copy_and_cquote('LICENSE.txt', os.path.join(outdir, 'LICENSE.txt.tmp'))
    copy_and_cquote('AUTHORS.rst', os.path.join(outdir, 'AUTHORS.rst.tmp'))

    # Create a duk_config.h.
    # XXX: might be easier to invoke genconfig directly
    def forward_genconfig_options():
        res = []
        res += [ '--metadata', os.path.abspath(opts.config_metadata) ]  # rename option, --config-metadata => --metadata
        if opts.platform is not None:
            res += [ '--platform', opts.platform ]
        if opts.compiler is not None:
            res += [ '--compiler', opts.compiler ]
        if opts.architecture is not None:
            res += [ '--architecture', opts.architecture ]
        if opts.c99_types_only:
            res += [ '--c99-types-only' ]
        if opts.dll:
            res += [ '--dll' ]
        if opts.support_feature_options:
            res += [ '--support-feature-options' ]
        if opts.emit_legacy_feature_check:
            res += [ '--emit-legacy-feature-check' ]
        if opts.emit_config_sanity_check:
            res += [ '--emit-config-sanity-check' ]
        if opts.omit_removed_config_options:
            res += [ '--omit-removed-config-options' ]
        if opts.omit_deprecated_config_options:
            res += [ '--omit-deprecated-config-options' ]
        if opts.omit_unused_config_options:
            res += [ '--omit-unused-config-options' ]
        if opts.add_active_defines_macro:
            res += [ '--add-active-defines-macro' ]
        for i in force_options_yaml:
            res += [ '--option-yaml', i ]
        for i in fixup_header_lines:
            res += [ '--fixup-linu', i ]
        if not opts.sanity_strict:
            res += [ '--sanity-warning' ]
        if opts.use_cpp_warning:
            res += [ '--use-cpp-warning' ]
        return res

    cmd = [
        sys.executable, os.path.join('tools', 'genconfig.py'),
        '--output', os.path.join(outdir, 'duk_config.h.tmp'),
        '--git-commit', git_commit, '--git-describe', git_describe, '--git-branch', git_branch
    ]
    cmd += forward_genconfig_options()
    cmd += [
        'duk-config-header'
    ]
    print(repr(cmd))
    exec_print_stdout(cmd)

    copy_file(os.path.join(outdir, 'duk_config.h.tmp'), os.path.join(outdir, 'src', 'duk_config.h'))
    copy_file(os.path.join(outdir, 'duk_config.h.tmp'), os.path.join(outdir, 'src-noline', 'duk_config.h'))
    copy_file(os.path.join(outdir, 'duk_config.h.tmp'), os.path.join(outdir, 'src-separate', 'duk_config.h'))

    # Build duktape.h from parts, with some git-related replacements.
    # The only difference between single and separate file duktape.h
    # is the internal DUK_SINGLE_FILE define.
    #
    # Newline after 'i \':
    # http://stackoverflow.com/questions/25631989/sed-insert-line-command-osx
    copy_and_replace(os.path.join(srcdir, 'duktape.h.in'), os.path.join(outdir, 'src', 'duktape.h'), {
        '@DUK_SINGLE_FILE@': '#define DUK_SINGLE_FILE',
        '@LICENSE_TXT@': read_file(os.path.join(outdir, 'LICENSE.txt.tmp'), strip_last_nl=True),
        '@AUTHORS_RST@': read_file(os.path.join(outdir, 'AUTHORS.rst.tmp'), strip_last_nl=True),
        '@DUK_API_PUBLIC_H@': read_file(os.path.join(srcdir, 'duk_api_public.h.in'), strip_last_nl=True),
        '@DUK_DBLUNION_H@': read_file(os.path.join(srcdir, 'duk_dblunion.h.in'), strip_last_nl=True),
        '@DUK_VERSION_FORMATTED@': duk_version_formatted,
        '@GIT_COMMIT@': git_commit,
        '@GIT_COMMIT_CSTRING@': git_commit_cstring,
        '@GIT_DESCRIBE@': git_describe,
        '@GIT_DESCRIBE_CSTRING@': git_describe_cstring,
        '@GIT_BRANCH@': git_branch,
        '@GIT_BRANCH_CSTRING@': git_branch_cstring
    })
    # keep the line so line numbers match between the two variant headers
    copy_and_replace(os.path.join(outdir, 'src', 'duktape.h'), os.path.join(outdir, 'src-separate', 'duktape.h'), {
        '#define DUK_SINGLE_FILE': '#undef DUK_SINGLE_FILE'
    })
    copy_file(os.path.join(outdir, 'src', 'duktape.h'), os.path.join(outdir, 'src-noline', 'duktape.h'))

    # Autogenerated strings and built-in files
    #
    # There are currently no profile specific variants of strings/builtins, but
    # this will probably change when functions are added/removed based on profile.

    # XXX: nuke this util, it's pointless
    exec_print_stdout([
        sys.executable,
        os.path.join('tools', 'genbuildparams.py'),
        '--version=' + str(duk_version),
        '--git-commit=' + git_commit,
        '--git-describe=' + git_describe,
        '--git-branch=' + git_branch,
        '--out-json=' + os.path.join(outdir, 'src-separate', 'buildparams.json.tmp'),
        '--out-header=' + os.path.join(outdir, 'src-separate', 'duk_buildparams.h.tmp')
    ])

    res = exec_get_stdout([
        sys.executable,
        os.path.join('tools', 'scan_used_stridx_bidx.py')
    ] + glob.glob(os.path.join(srcdir, '*.c')) \
      + glob.glob(os.path.join(srcdir, '*.h')) \
      + glob.glob(os.path.join(srcdir, '*.h.in'))
    )
    with open(os.path.join(outdir, 'duk_used_stridx_bidx_defs.json.tmp'), 'wb') as f:
        f.write(res)

    gb_opts = []
    gb_opts.append('--ram-support')  # enable by default
    if opts.rom_support:
        # ROM string/object support is not enabled by default because
        # it increases the generated duktape.c considerably.
        print('Enabling --rom-support for genbuiltins.py')
        gb_opts.append('--rom-support')
    if opts.rom_auto_lightfunc:
        print('Enabling --rom-auto-lightfunc for genbuiltins.py')
        gb_opts.append('--rom-auto-lightfunc')
    for fn in opts.user_builtin_metadata:
        print('Forwarding --user-builtin-metadata %s' % fn)
        gb_opts.append('--user-builtin-metadata')
        gb_opts.append(fn)
    exec_print_stdout([
        sys.executable,
        os.path.join('tools', 'genbuiltins.py'),
        '--buildinfo=' + os.path.join(outdir, 'src-separate', 'buildparams.json.tmp'),
        '--used-stridx-metadata=' + os.path.join(outdir, 'duk_used_stridx_bidx_defs.json.tmp'),
        '--strings-metadata=' + os.path.join(srcdir, 'strings.yaml'),
        '--objects-metadata=' + os.path.join(srcdir, 'builtins.yaml'),
        '--out-header=' + os.path.join(outdir, 'src-separate', 'duk_builtins.h'),
        '--out-source=' + os.path.join(outdir, 'src-separate', 'duk_builtins.c'),
        '--out-metadata-json=' + os.path.join(outdir, 'duk_build_meta.json')
    ] + gb_opts)

    # Autogenerated Unicode files
    #
    # Note: not all of the generated headers are used.  For instance, the
    # match table for "WhiteSpace-Z" is not used, because a custom piece
    # of code handles that particular match.
    #
    # UnicodeData.txt contains ranges expressed like this:
    #
    #   4E00;<CJK Ideograph, First>;Lo;0;L;;;;;N;;;;;
    #   9FCB;<CJK Ideograph, Last>;Lo;0;L;;;;;N;;;;;
    #
    # These are currently decoded into individual characters as a prestep.
    #
    # For IDPART:
    #   UnicodeCombiningMark -> categories Mn, Mc
    #   UnicodeDigit -> categories Nd
    #   UnicodeConnectorPunctuation -> categories Pc

    # Whitespace (unused now)
    WHITESPACE_INCL='Zs'  # USP = Any other Unicode space separator
    WHITESPACE_EXCL='NONE'

    # Unicode letter (unused now)
    LETTER_INCL='Lu,Ll,Lt,Lm,Lo'
    LETTER_EXCL='NONE'
    LETTER_NOA_INCL='Lu,Ll,Lt,Lm,Lo'
    LETTER_NOA_EXCL='ASCII'
    LETTER_NOABMP_INCL=LETTER_NOA_INCL
    LETTER_NOABMP_EXCL='ASCII,NONBMP'

    # Identifier start
    # E5 Section 7.6
    IDSTART_INCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F'
    IDSTART_EXCL='NONE'
    IDSTART_NOA_INCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F'
    IDSTART_NOA_EXCL='ASCII'
    IDSTART_NOABMP_INCL=IDSTART_NOA_INCL
    IDSTART_NOABMP_EXCL='ASCII,NONBMP'

    # Identifier start - Letter: allows matching of (rarely needed) 'Letter'
    # production space efficiently with the help of IdentifierStart.  The
    # 'Letter' production is only needed in case conversion of Greek final
    # sigma.
    IDSTART_MINUS_LETTER_INCL=IDSTART_NOA_INCL
    IDSTART_MINUS_LETTER_EXCL='Lu,Ll,Lt,Lm,Lo'
    IDSTART_MINUS_LETTER_NOA_INCL=IDSTART_NOA_INCL
    IDSTART_MINUS_LETTER_NOA_EXCL='Lu,Ll,Lt,Lm,Lo,ASCII'
    IDSTART_MINUS_LETTER_NOABMP_INCL=IDSTART_NOA_INCL
    IDSTART_MINUS_LETTER_NOABMP_EXCL='Lu,Ll,Lt,Lm,Lo,ASCII,NONBMP'

    # Identifier start - Identifier part
    # E5 Section 7.6: IdentifierPart, but remove IdentifierStart (already above)
    IDPART_MINUS_IDSTART_INCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F,Mn,Mc,Nd,Pc,200C,200D'
    IDPART_MINUS_IDSTART_EXCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F'
    IDPART_MINUS_IDSTART_NOA_INCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F,Mn,Mc,Nd,Pc,200C,200D'
    IDPART_MINUS_IDSTART_NOA_EXCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F,ASCII'
    IDPART_MINUS_IDSTART_NOABMP_INCL=IDPART_MINUS_IDSTART_NOA_INCL
    IDPART_MINUS_IDSTART_NOABMP_EXCL='Lu,Ll,Lt,Lm,Lo,Nl,0024,005F,ASCII,NONBMP'

    print('Expand UnicodeData.txt ranges')

    exec_print_stdout([
        sys.executable,
        os.path.join('tools', 'prepare_unicode_data.py'),
        os.path.join(srcdir, 'UnicodeData.txt'),
        os.path.join(outdir, 'src-separate', 'UnicodeData-expanded.tmp')
    ])

    def extract_chars(incl, excl, suffix):
        #print('- extract_chars: %s %s %s' % (incl, excl, suffix))
        res = exec_get_stdout([
            sys.executable,
            os.path.join('tools', 'extract_chars.py'),
            '--unicode-data=' + os.path.join(outdir, 'src-separate', 'UnicodeData-expanded.tmp'),
            '--include-categories=' + incl,
            '--exclude-categories=' + excl,
            '--out-source=' + os.path.join(outdir, 'src-separate', 'duk_unicode_%s.c.tmp' % suffix),
            '--out-header=' + os.path.join(outdir, 'src-separate', 'duk_unicode_%s.h.tmp' % suffix),
            '--table-name=' + 'duk_unicode_%s' % suffix
        ])
        with open(os.path.join(outdir, 'src-separate', suffix + '.txt'), 'wb') as f:
            f.write(res)

    def extract_caseconv():
        #print('- extract_caseconv case conversion')
        res = exec_get_stdout([
            sys.executable,
            os.path.join('tools', 'extract_caseconv.py'),
            '--command=caseconv_bitpacked',
            '--unicode-data=' + os.path.join(outdir, 'src-separate', 'UnicodeData-expanded.tmp'),
            '--special-casing=' + os.path.join(srcdir, 'SpecialCasing.txt'),
            '--out-source=' + os.path.join(outdir, 'src-separate', 'duk_unicode_caseconv.c.tmp'),
            '--out-header=' + os.path.join(outdir, 'src-separate', 'duk_unicode_caseconv.h.tmp'),
            '--table-name-lc=duk_unicode_caseconv_lc',
            '--table-name-uc=duk_unicode_caseconv_uc'
        ])
        with open(os.path.join(outdir, 'src-separate', 'caseconv.txt'), 'wb') as f:
            f.write(res)

        #print('- extract_caseconv canon lookup')
        res = exec_get_stdout([
            sys.executable,
            os.path.join('tools', 'extract_caseconv.py'),
            '--command=re_canon_lookup',
            '--unicode-data=' + os.path.join(outdir, 'src-separate', 'UnicodeData-expanded.tmp'),
            '--special-casing=' + os.path.join(srcdir, 'SpecialCasing.txt'),
            '--out-source=' + os.path.join(outdir, 'src-separate', 'duk_unicode_re_canon_lookup.c.tmp'),
            '--out-header=' + os.path.join(outdir, 'src-separate', 'duk_unicode_re_canon_lookup.h.tmp'),
            '--table-name-re-canon-lookup=duk_unicode_re_canon_lookup'
        ])
        with open(os.path.join(outdir, 'src-separate', 'caseconv_re_canon_lookup.txt'), 'wb') as f:
            f.write(res)

    print('Create Unicode tables for codepoint classes')
    extract_chars(WHITESPACE_INCL, WHITESPACE_EXCL, 'ws')
    extract_chars(LETTER_INCL, LETTER_EXCL, 'let')
    extract_chars(LETTER_NOA_INCL, LETTER_NOA_EXCL, 'let_noa')
    extract_chars(LETTER_NOABMP_INCL, LETTER_NOABMP_EXCL, 'let_noabmp')
    extract_chars(IDSTART_INCL, IDSTART_EXCL, 'ids')
    extract_chars(IDSTART_NOA_INCL, IDSTART_NOA_EXCL, 'ids_noa')
    extract_chars(IDSTART_NOABMP_INCL, IDSTART_NOABMP_EXCL, 'ids_noabmp')
    extract_chars(IDSTART_MINUS_LETTER_INCL, IDSTART_MINUS_LETTER_EXCL, 'ids_m_let')
    extract_chars(IDSTART_MINUS_LETTER_NOA_INCL, IDSTART_MINUS_LETTER_NOA_EXCL, 'ids_m_let_noa')
    extract_chars(IDSTART_MINUS_LETTER_NOABMP_INCL, IDSTART_MINUS_LETTER_NOABMP_EXCL, 'ids_m_let_noabmp')
    extract_chars(IDPART_MINUS_IDSTART_INCL, IDPART_MINUS_IDSTART_EXCL, 'idp_m_ids')
    extract_chars(IDPART_MINUS_IDSTART_NOA_INCL, IDPART_MINUS_IDSTART_NOA_EXCL, 'idp_m_ids_noa')
    extract_chars(IDPART_MINUS_IDSTART_NOABMP_INCL, IDPART_MINUS_IDSTART_NOABMP_EXCL, 'idp_m_ids_noabmp')

    print('Create Unicode tables for case conversion')
    extract_caseconv()

    print('Combine sources and clean up')

    # Inject autogenerated files into source and header files so that they are
    # usable (for all profiles and define cases) directly.
    #
    # The injection points use a standard C preprocessor #include syntax
    # (earlier these were actual includes).

    copy_and_replace(os.path.join(outdir, 'src-separate', 'duk_unicode.h'), os.path.join(outdir, 'src-separate', 'duk_unicode.h'), {
        '#include "duk_unicode_ids_noa.h"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_ids_noa.h.tmp'), strip_last_nl=True),
        '#include "duk_unicode_ids_noabmp.h"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_ids_noabmp.h.tmp'), strip_last_nl=True),
        '#include "duk_unicode_ids_m_let_noa.h"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_ids_m_let_noa.h.tmp'), strip_last_nl=True),
        '#include "duk_unicode_ids_m_let_noabmp.h"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_ids_m_let_noabmp.h.tmp'), strip_last_nl=True),
        '#include "duk_unicode_idp_m_ids_noa.h"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_idp_m_ids_noa.h.tmp'), strip_last_nl=True),
        '#include "duk_unicode_idp_m_ids_noabmp.h"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_idp_m_ids_noabmp.h.tmp'), strip_last_nl=True),
        '#include "duk_unicode_caseconv.h"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_caseconv.h.tmp'), strip_last_nl=True),
        '#include "duk_unicode_re_canon_lookup.h"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_re_canon_lookup.h.tmp'), strip_last_nl=True)
    })

    copy_and_replace(os.path.join(outdir, 'src-separate', 'duk_unicode_tables.c'), os.path.join(outdir, 'src-separate', 'duk_unicode_tables.c'), {
        '#include "duk_unicode_ids_noa.c"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_ids_noa.c.tmp'), strip_last_nl=True),
        '#include "duk_unicode_ids_noabmp.c"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_ids_noabmp.c.tmp'), strip_last_nl=True),
        '#include "duk_unicode_ids_m_let_noa.c"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_ids_m_let_noa.c.tmp'), strip_last_nl=True),
        '#include "duk_unicode_ids_m_let_noabmp.c"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_ids_m_let_noabmp.c.tmp'), strip_last_nl=True),
        '#include "duk_unicode_idp_m_ids_noa.c"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_idp_m_ids_noa.c.tmp'), strip_last_nl=True),
        '#include "duk_unicode_idp_m_ids_noabmp.c"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_idp_m_ids_noabmp.c.tmp'), strip_last_nl=True),
        '#include "duk_unicode_caseconv.c"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_caseconv.c.tmp'), strip_last_nl=True),
        '#include "duk_unicode_re_canon_lookup.c"': read_file(os.path.join(outdir, 'src-separate', 'duk_unicode_re_canon_lookup.c.tmp'), strip_last_nl=True)
    })

    # Clean up some temporary files

    delete_matching_files(os.path.join(outdir, 'src-separate'), lambda x: x[-4:] == '.tmp')
    delete_matching_files(os.path.join(outdir, 'src-separate'), lambda x: x in [
        'ws.txt',
        'let.txt', 'let_noa.txt', 'let_noabmp.txt',
        'ids.txt', 'ids_noa.txt', 'ids_noabmp.txt',
        'ids_m_let.txt', 'ids_m_let_noa.txt', 'ids_m_let_noabmp.txt',
        'idp_m_ids.txt', 'idp_m_ids_noa.txt', 'idp_m_ids_noabmp.txt'
    ])
    delete_matching_files(os.path.join(outdir, 'src-separate'), lambda x: x[0:8] == 'caseconv' and x[-4:] == '.txt')

    # Create a combined source file, duktape.c, into a separate combined source
    # directory.  This allows user to just include "duktape.c", "duktape.h", and
    # "duk_config.h" into a project and maximizes inlining and size optimization
    # opportunities even with older compilers.  Because some projects include
    # these files into their repository, the result should be deterministic and
    # diffable.  Also, it must retain __FILE__/__LINE__ behavior through
    # preprocessor directives.  Whitespace and comments can be stripped as long
    # as the other requirements are met.  For some users it's preferable *not*
    # to use #line directives in the combined source, so a separate variant is
    # created for that, see: https://github.com/svaarala/duktape/pull/363.

    def create_source_prologue(license_file, authors_file):
        res = []

        # Because duktape.c/duktape.h/duk_config.h are often distributed or
        # included in project sources as is, add a license reminder and
        # Duktape version information to the duktape.c header (duktape.h
        # already contains them).

        duk_major = duk_version / 10000
        duk_minor = duk_version / 100 % 100
        duk_patch = duk_version % 100
        res.append('/*')
        res.append(' *  Single source autogenerated distributable for Duktape %d.%d.%d.' % (duk_major, duk_minor, duk_patch))
        res.append(' *')
        res.append(' *  Git commit %s (%s).' % (git_commit, git_describe))
        res.append(' *  Git branch %s.' % git_branch)
        res.append(' *')
        res.append(' *  See Duktape AUTHORS.rst and LICENSE.txt for copyright and')
        res.append(' *  licensing information.')
        res.append(' */')
        res.append('')

        # Add LICENSE.txt and AUTHORS.rst to combined source so that they're automatically
        # included and are up-to-date.

        res.append('/* LICENSE.txt */')
        with open(license_file, 'rb') as f:
            for line in f:
                res.append(line.strip())
        res.append('')
        res.append('/* AUTHORS.rst */')
        with open(authors_file, 'rb') as f:
            for line in f:
                res.append(line.strip())

        return '\n'.join(res) + '\n'

    def select_combined_sources():
        # These files must appear before the alphabetically sorted
        # ones so that static variables get defined before they're
        # used.  We can't forward declare them because that would
        # cause C++ issues (see GH-63).  When changing, verify by
        # compiling with g++.
        handpick = [
            'duk_replacements.c',
            'duk_debug_macros.c',
            'duk_builtins.c',
            'duk_error_macros.c',
            'duk_unicode_support.c',
            'duk_util_misc.c',
            'duk_util_hashprime.c',
            'duk_hobject_class.c'
        ]

        files = []
        for fn in handpick:
            files.append(fn)

        for fn in sorted(os.listdir(os.path.join(outdir, 'src-separate'))):
            f_ext = os.path.splitext(fn)[1]
            if f_ext not in [ '.c' ]:
                continue
            if fn in files:
                continue
            files.append(fn)

        res = map(lambda x: os.path.join(outdir, 'src-separate', x), files)
        #print(repr(files))
        #print(repr(res))
        return res

    with open(os.path.join(outdir, 'prologue.tmp'), 'wb') as f:
        f.write(create_source_prologue(os.path.join(outdir, 'LICENSE.txt.tmp'), os.path.join(outdir, 'AUTHORS.rst.tmp')))

    exec_print_stdout([
        sys.executable,
        os.path.join('tools', 'combine_src.py'),
        '--include-path', os.path.join(outdir, 'src-separate'),
        '--include-exclude', 'duk_config.h',  # don't inline
        '--include-exclude', 'duktape.h',     # don't inline
        '--prologue', os.path.join(outdir, 'prologue.tmp'),
        '--output-source', os.path.join(outdir, 'src', 'duktape.c'),
        '--output-metadata', os.path.join(outdir, 'src', 'metadata.json'),
        '--line-directives'
    ] + select_combined_sources())

    exec_print_stdout([
        sys.executable,
        os.path.join('tools', 'combine_src.py'),
        '--include-path', os.path.join(outdir, 'src-separate'),
        '--include-exclude', 'duk_config.h',  # don't inline
        '--include-exclude', 'duktape.h',     # don't inline
        '--prologue', os.path.join(outdir, 'prologue.tmp'),
        '--output-source', os.path.join(outdir, 'src-noline', 'duktape.c'),
        '--output-metadata', os.path.join(outdir, 'src-noline', 'metadata.json')
    ] + select_combined_sources())

    # Clean up remaining temp files
    delete_matching_files(outdir, lambda x: x[-4:] == '.tmp')

    print('Config-and-prepare finished successfully')

if __name__ == '__main__':
    main()
