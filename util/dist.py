#!/usr/bin/env python2
#
#  Create a distributable Duktape package into 'dist' directory.  The contents
#  of this directory can then be packaged into a source distributable.
#

import logging
import sys
logging.basicConfig(level=logging.INFO, stream=sys.stdout, format='%(name)-21s %(levelname)-7s %(message)s')
logger = logging.getLogger('dist.py')
logger.setLevel(logging.INFO)

import os
import re
import json
import shutil
import glob
import optparse
import subprocess
import logging

# Helpers.

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
                logger.warning(' command %r failed, return default' % cmd)
                return default
            raise Exception('command failed, return code %d: %r' % (proc.returncode, cmd))
        return ret[0]
    except:
        if default is not None:
            logger.warning('command %r failed, return default' % cmd)
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
            logger.debug('Deleting %r' % os.path.join(dirpath, fn))
            os.unlink(os.path.join(dirpath, fn))
            #print('Deleting matching file: %r' % fn)

def glob_files(pattern):
    return glob.glob(pattern)

def cstring(x):
    return '"' + x + '"'  # good enough for now

# Get Duktape version number as an integer.  DUK_VERSION is grepped from
# duktape.h.in: it is needed for the public API and we want to avoid
# defining it in multiple places.
def get_duk_version():
    r = re.compile(r'^#define\s+DUK_VERSION\s+(.*?)L?\s*$')
    with open(os.path.join('src-input', 'duktape.h.in'), 'rb') as f:
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

def create_dist_directories(dist):
    if os.path.exists(dist):
        raise Exception('dist target directory %s already exists, please delete first' % repr(dist))
    mkdir(dist)
    mkdir(os.path.join(dist, 'src-input'))
    mkdir(os.path.join(dist, 'tools'))
    mkdir(os.path.join(dist, 'config'))
    mkdir(os.path.join(dist, 'extras'))
    mkdir(os.path.join(dist, 'extras', 'duk-v1-compat'))
    mkdir(os.path.join(dist, 'extras', 'print-alert'))
    mkdir(os.path.join(dist, 'extras', 'console'))
    mkdir(os.path.join(dist, 'extras', 'logging'))
    mkdir(os.path.join(dist, 'extras', 'minimal-printf'))
    mkdir(os.path.join(dist, 'extras', 'module-duktape'))
    mkdir(os.path.join(dist, 'extras', 'module-node'))
    mkdir(os.path.join(dist, 'extras', 'alloc-pool'))
    mkdir(os.path.join(dist, 'extras', 'cbor'))
    mkdir(os.path.join(dist, 'polyfills'))
    #mkdir(os.path.join(dist, 'doc'))  # Empty, so omit
    mkdir(os.path.join(dist, 'licenses'))
    mkdir(os.path.join(dist, 'debugger'))
    mkdir(os.path.join(dist, 'debugger', 'static'))
    mkdir(os.path.join(dist, 'examples'))
    mkdir(os.path.join(dist, 'examples', 'hello'))
    mkdir(os.path.join(dist, 'examples', 'eval'))
    mkdir(os.path.join(dist, 'examples', 'cmdline'))
    mkdir(os.path.join(dist, 'examples', 'eventloop'))
    mkdir(os.path.join(dist, 'examples', 'guide'))
    mkdir(os.path.join(dist, 'examples', 'coffee'))
    mkdir(os.path.join(dist, 'examples', 'jxpretty'))
    mkdir(os.path.join(dist, 'examples', 'sandbox'))
    mkdir(os.path.join(dist, 'examples', 'alloc-logging'))
    mkdir(os.path.join(dist, 'examples', 'alloc-torture'))
    mkdir(os.path.join(dist, 'examples', 'alloc-hybrid'))
    mkdir(os.path.join(dist, 'examples', 'debug-trans-socket'))
    mkdir(os.path.join(dist, 'examples', 'debug-trans-dvalue'))
    mkdir(os.path.join(dist, 'examples', 'codepage-conv'))
    mkdir(os.path.join(dist, 'examples', 'dummy-date-provider'))
    mkdir(os.path.join(dist, 'examples', 'cpp-exceptions'))

# Spot check a few files to ensure we're in Duktape repo root, as dist only
# works from there.

def check_cwd_duktape_repo_root():
    if not (os.path.isfile(os.path.join('src-input', 'duktape.h.in')) and \
            os.path.isfile(os.path.join('config', 'platforms.yaml'))):
        sys.stderr.write('\n')
        sys.stderr.write('*** Working directory must be Duktape repo checkout root!\n')
        sys.stderr.write('\n')
        raise Exception('Incorrect working directory')

# Option parsing.

def parse_options():
    parser = optparse.OptionParser()
    parser.add_option('--repo-directory', dest='repo_directory', default=None, help='Duktape repo directory (default is CWD)')
    parser.add_option('--output-directory', dest='output_directory', default=None, help='Dist output directory (created automatically, must not exist; default is <repo>/dist)')
    parser.add_option('--git-commit', dest='git_commit', default=None, help='Force git commit hash')
    parser.add_option('--git-describe', dest='git_describe', default=None, help='Force git describe')
    parser.add_option('--git-branch', dest='git_branch', default=None, help='Force git branch name')
    parser.add_option('--create-spdx', dest='create_spdx', action='store_true', default=False, help='Create SPDX license file')
    parser.add_option('--rom-support', dest='rom_support', action='store_true', help=optparse.SUPPRESS_HELP)
    parser.add_option('--rom-auto-lightfunc', dest='rom_auto_lightfunc', action='store_true', default=False, help=optparse.SUPPRESS_HELP)
    parser.add_option('--user-builtin-metadata', dest='user_builtin_metadata', action='append', default=[], help=optparse.SUPPRESS_HELP)
    parser.add_option('--quiet', dest='quiet', action='store_true', default=False, help='Suppress info messages (show warnings)')
    parser.add_option('--verbose', dest='verbose', action='store_true', default=False, help='Show verbose debug messages')
    (opts, args) = parser.parse_args()

    return opts, args

# Python module check and friendly errors.

def check_python_modules(opts):
    # dist.py doesn't need yaml but other dist utils will; check for it and
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

    try:
        if opts.create_spdx:
            import rdflib
    except:
        # Tolerate missing rdflib, just warn about it.
        _warning('rdflib', 'python-rdflib', 'rdflib')
        #failed = True

    if failed:
        sys.stderr.write('\n')
        raise Exception('Missing some required Python modules')

def main():
    # Basic option parsing, Python module check, CWD check.

    opts, args = parse_options()

    # Log level.
    forward_loglevel = []
    if opts.quiet:
        logger.setLevel(logging.WARNING)
        forward_loglevel = [ '--quiet' ]
    elif opts.verbose:
        logger.setLevel(logging.DEBUG)
        forward_loglevel = [ '--verbose' ]

    check_python_modules(opts)

    if opts.repo_directory is None:
        opts.repo_directory = os.path.abspath('.')
        logger.info('No --repo-directory option, defaulting to current directory %s' % opts.repo_directory)
        check_cwd_duktape_repo_root()
    opts.repo_directory = os.path.abspath(opts.repo_directory)
    logger.debug('Using repo directory: %s' % opts.repo_directory)

    if opts.output_directory is None:
        opts.output_directory = os.path.abspath(os.path.join(opts.repo_directory, 'dist'))
        logger.info('No --output-directory option, defaulting to repo/dist directory %s' % opts.output_directory)
    opts.output_directory = os.path.abspath(opts.output_directory)
    logger.debug('Using output directory: %s' % opts.output_directory)

    # Obsolete options check.

    if opts.rom_support or opts.rom_auto_lightfunc:
        raise Exception('obsolete ROM support argument(s), use tools/configure.py instead')
    if len(opts.user_builtin_metadata) > 0:
        raise Exception('obsolete --user-builtin-metadata argument, use tools/configure.py and --builtin-file instead')

    # Figure out directories, git info, Duktape version, etc.

    entry_cwd = os.getcwd()
    dist = opts.output_directory
    os.chdir(opts.repo_directory)

    duk_version, duk_major, duk_minor, duk_patch, duk_version_formatted = get_duk_version()

    if opts.git_commit is not None:
        git_commit = opts.git_commit
    else:
        git_commit = exec_get_stdout([ 'git', 'rev-parse', 'HEAD' ], default='external').strip()
    if opts.git_describe is not None:
        git_describe = opts.git_describe
    else:
        git_describe = exec_get_stdout([ 'git', 'describe', '--always', '--dirty' ], default='external').strip()
    if opts.git_branch is not None:
        git_branch = opts.git_branch
    else:
        git_branch = exec_get_stdout([ 'git', 'rev-parse', '--abbrev-ref', 'HEAD' ], default='external').strip()

    git_commit_cstring = cstring(git_commit)
    git_describe_cstring = cstring(git_describe)
    git_branch_cstring = cstring(git_branch)

    logger.info('Dist for Duktape version %s, commit %s, describe %s, branch %s' % \
                (duk_version_formatted, git_commit, git_describe, git_branch))

    # Create dist directory structure, copy files.

    logger.debug('Create dist directories and copy static files')

    os.chdir(opts.repo_directory)
    create_dist_directories(dist)

    os.chdir(opts.repo_directory)

    copy_files([
        'builtins.yaml',
        'duk_alloc_default.c',
        'duk_api_buffer.c',
        'duk_api_bytecode.c',
        'duk_api_call.c',
        'duk_api_codec.c',
        'duk_api_compile.c',
        'duk_api_debug.c',
        'duk_api_heap.c',
        'duk_api_internal.h',
        'duk_api_inspect.c',
        'duk_api_memory.c',
        'duk_api_object.c',
        'duk_api_random.c',
        'duk_api_stack.c',
        'duk_api_string.c',
        'duk_api_time.c',
        'duk_bi_array.c',
        'duk_bi_boolean.c',
        'duk_bi_buffer.c',
        'duk_bi_cbor.c',
        'duk_bi_date.c',
        'duk_bi_date_unix.c',
        'duk_bi_date_windows.c',
        'duk_bi_duktape.c',
        'duk_bi_encoding.c',
        'duk_bi_error.c',
        'duk_bi_function.c',
        'duk_bi_global.c',
        'duk_bi_json.c',
        'duk_bi_math.c',
        'duk_bi_number.c',
        'duk_bi_object.c',
        'duk_bi_performance.c',
        'duk_bi_pointer.c',
        'duk_bi_promise.c',
        'duk_bi_protos.h',
        'duk_bi_proxy.c',
        'duk_bi_reflect.c',
        'duk_bi_regexp.c',
        'duk_bi_string.c',
        'duk_bi_symbol.c',
        'duk_bi_thread.c',
        'duk_bi_thrower.c',
        'duk_dblunion.h',
        'duk_debug_fixedbuffer.c',
        'duk_debugger.c',
        'duk_debugger.h',
        'duk_debug.h',
        'duk_debug_macros.c',
        'duk_debug_vsnprintf.c',
        'duk_error_augment.c',
        'duk_error.h',
        'duk_error_longjmp.c',
        'duk_error_macros.c',
        'duk_error_misc.c',
        'duk_error_throw.c',
        'duk_exception.h',
        'duk_fltunion.h',
        'duk_forwdecl.h',
        'duk_harray.h',
        'duk_hboundfunc.h',
        'duk_hbuffer_alloc.c',
        'duk_hbuffer_assert.c',
        'duk_hbuffer.h',
        'duk_hbuffer_ops.c',
        'duk_hbufobj.h',
        'duk_hbufobj_misc.c',
        'duk_hcompfunc.h',
        'duk_heap_alloc.c',
        'duk_heap.h',
        'duk_heap_hashstring.c',
        'duk_heaphdr.h',
        'duk_heaphdr_assert.c',
        'duk_heap_finalize.c',
        'duk_heap_markandsweep.c',
        'duk_heap_memory.c',
        'duk_heap_misc.c',
        'duk_heap_refcount.c',
        'duk_heap_stringcache.c',
        'duk_heap_stringtable.c',
        'duk_henv.h',
        'duk_hnatfunc.h',
        'duk_hobject_alloc.c',
        'duk_hobject_assert.c',
        'duk_hobject_class.c',
        'duk_hobject_enum.c',
        'duk_hobject.h',
        'duk_hobject_misc.c',
        'duk_hobject_pc2line.c',
        'duk_hobject_props.c',
        'duk_hproxy.h',
        'duk_hstring.h',
        'duk_hstring_assert.c',
        'duk_hstring_misc.c',
        'duk_hthread_alloc.c',
        'duk_hthread_builtins.c',
        'duk_hthread.h',
        'duk_hthread_misc.c',
        'duk_hthread_stacks.c',
        'duk_internal.h',
        'duk_jmpbuf.h',
        'duk_js_arith.c',
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
        'duk_refcount.h',
        'duk_regexp_compiler.c',
        'duk_regexp_executor.c',
        'duk_regexp.h',
        'duk_replacements.c',
        'duk_replacements.h',
        'duk_selftest.c',
        'duk_selftest.h',
        'duk_strings.h',
        'duktape.h.in',
        'duk_tval.c',
        'duk_tval.h',
        'duk_unicode.h',
        'duk_unicode_support.c',
        'duk_unicode_tables.c',
        'duk_util.h',
        'duk_util_bitdecoder.c',
        'duk_util_bitencoder.c',
        'duk_util_bufwriter.c',
        'duk_util_hashbytes.c',
        'duk_util_tinyrandom.c',
        'duk_util_double.c',
        'duk_util_cast.c',
        'duk_util_memory.c',
        'duk_util_memrw.c',
        'duk_util_misc.c',
        'strings.yaml',
        'SpecialCasing.txt',
        'SpecialCasing-8bit.txt',
        'UnicodeData.txt',
        'UnicodeData-8bit.txt',
    ], 'src-input', os.path.join(dist, 'src-input'))

    for fn in [
        'tags.yaml',
        'platforms.yaml',
        'architectures.yaml',
        'compilers.yaml',
        'platforms',
        'architectures',
        'compilers',
        'feature-options',
        'config-options',
        'helper-snippets',
        'header-snippets',
        'examples'
    ]:
        # Copy directories in their entirety
        if os.path.isfile(os.path.join('config', fn)):
            shutil.copyfile(os.path.join('config', fn), os.path.join(dist, 'config', fn))
        else:
            shutil.copytree(os.path.join('config', fn), os.path.join(dist, 'config', fn))

    copy_files([
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
        'json2yaml.py',
        'merge_debug_meta.py',
        'prepare_unicode_data.py',
        'resolve_combined_lineno.py',
        'scan_strings.py',
        'scan_used_stridx_bidx.py',
        'yaml2json.py',
    ], 'tools', os.path.join(dist, 'tools'))

    copy_files([
        'README.rst'
    ], 'config', os.path.join(dist, 'config'))

    copy_files([
        'README.rst',
        'Makefile',
        'package.json',
        'duk_debug.js',
        'duk_debug_proxy.js',
        'duk_classnames.yaml',
        'duk_debugcommands.yaml',
        'duk_debugerrors.yaml',
        'duk_opcodes.yaml'
    ], 'debugger', os.path.join(dist, 'debugger'))

    copy_files([
        'index.html',
        'style.css',
        'webui.js'
    ], os.path.join('debugger', 'static'), os.path.join(dist, 'debugger', 'static'))

    copy_files([
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
    ], 'polyfills', os.path.join(dist, 'polyfills'))

    copy_files([
        'README.rst'
    ], 'examples', os.path.join(dist, 'examples'))

    copy_files([
        'README.rst',
        'duk_cmdline.c',
        'duk_cmdline.h',
        'duk_cmdline_lowmem.c'
    ], os.path.join('examples', 'cmdline'), os.path.join(dist, 'examples', 'cmdline'))

    copy_files([
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
    ], os.path.join('examples', 'eventloop'), os.path.join(dist, 'examples', 'eventloop'))

    copy_files([
        'README.rst',
        'hello.c'
    ], os.path.join('examples', 'hello'), os.path.join(dist, 'examples', 'hello'))

    copy_files([
        'README.rst',
        'eval.c'
    ], os.path.join('examples', 'eval'), os.path.join(dist, 'examples', 'eval'))

    copy_files([
        'README.rst',
        'fib.js',
        'process.js',
        'processlines.c',
        'prime.js',
        'primecheck.c',
        'uppercase.c'
    ], os.path.join('examples', 'guide'), os.path.join(dist, 'examples', 'guide'))

    copy_files([
        'README.rst',
        'globals.coffee',
        'hello.coffee',
        'mandel.coffee'
    ], os.path.join('examples', 'coffee'), os.path.join(dist, 'examples', 'coffee'))

    copy_files([
        'README.rst',
        'jxpretty.c'
    ], os.path.join('examples', 'jxpretty'), os.path.join(dist, 'examples', 'jxpretty'))

    copy_files([
        'README.rst',
        'sandbox.c'
    ], os.path.join('examples', 'sandbox'), os.path.join(dist, 'examples', 'sandbox'))

    copy_files([
        'README.rst',
        'duk_alloc_logging.c',
        'duk_alloc_logging.h',
        'log2gnuplot.py'
    ], os.path.join('examples', 'alloc-logging'), os.path.join(dist, 'examples', 'alloc-logging'))

    copy_files([
        'README.rst',
        'duk_alloc_torture.c',
        'duk_alloc_torture.h'
    ], os.path.join('examples', 'alloc-torture'), os.path.join(dist, 'examples', 'alloc-torture'))

    copy_files([
        'README.rst',
        'duk_alloc_hybrid.c',
        'duk_alloc_hybrid.h'
    ], os.path.join('examples', 'alloc-hybrid'), os.path.join(dist, 'examples', 'alloc-hybrid'))

    copy_files([
        'README.rst',
        'duk_trans_socket_unix.c',
        'duk_trans_socket_windows.c',
        'duk_trans_socket.h'
    ], os.path.join('examples', 'debug-trans-socket'), os.path.join(dist, 'examples', 'debug-trans-socket'))

    copy_files([
        'README.rst',
        'duk_trans_dvalue.c',
        'duk_trans_dvalue.h',
        'test.c',
        'Makefile'
    ], os.path.join('examples', 'debug-trans-dvalue'), os.path.join(dist, 'examples', 'debug-trans-dvalue'))

    copy_files([
        'README.rst',
        'duk_codepage_conv.c',
        'duk_codepage_conv.h',
        'test.c'
    ], os.path.join('examples', 'codepage-conv'), os.path.join(dist, 'examples', 'codepage-conv'))

    copy_files([
        'README.rst',
        'dummy_date_provider.c'
    ], os.path.join('examples', 'dummy-date-provider'), os.path.join(dist, 'examples', 'dummy-date-provider'))

    copy_files([
        'README.rst',
        'cpp_exceptions.cpp'
    ], os.path.join('examples', 'cpp-exceptions'), os.path.join(dist, 'examples', 'cpp-exceptions'))

    copy_files([
        'README.rst'
    ], 'extras', os.path.join(dist, 'extras'))

    copy_files([
        'README.rst',
        'duk_logging.c',
        'duk_logging.h',
        'test.c',
        'Makefile'
    ], os.path.join('extras', 'logging'), os.path.join(dist, 'extras', 'logging'))

    copy_files([
        'README.rst',
        'duk_v1_compat.c',
        'duk_v1_compat.h',
        'test.c',
        'Makefile',
        'test_eval1.js',
        'test_eval2.js',
        'test_compile1.js',
        'test_compile2.js'
    ], os.path.join('extras', 'duk-v1-compat'), os.path.join(dist, 'extras', 'duk-v1-compat'))

    copy_files([
        'README.rst',
        'duk_print_alert.c',
        'duk_print_alert.h',
        'test.c',
        'Makefile'
    ], os.path.join('extras', 'print-alert'), os.path.join(dist, 'extras', 'print-alert'))

    copy_files([
        'README.rst',
        'duk_console.c',
        'duk_console.h',
        'test.c',
        'Makefile'
    ], os.path.join('extras', 'console'), os.path.join(dist, 'extras', 'console'))

    copy_files([
        'README.rst',
        'duk_minimal_printf.c',
        'duk_minimal_printf.h',
        'Makefile',
        'test.c'
    ], os.path.join('extras', 'minimal-printf'), os.path.join(dist, 'extras', 'minimal-printf'))

    copy_files([
        'README.rst',
        'duk_module_duktape.c',
        'duk_module_duktape.h',
        'Makefile',
        'test.c'
    ], os.path.join('extras', 'module-duktape'), os.path.join(dist, 'extras', 'module-duktape'))

    copy_files([
        'README.rst',
        'duk_module_node.c',
        'duk_module_node.h',
        'Makefile',
        'test.c'
    ], os.path.join('extras', 'module-node'), os.path.join(dist, 'extras', 'module-node'))

    copy_files([
        'README.rst',
        'duk_alloc_pool.c',
        'duk_alloc_pool.h',
        'ptrcomp.yaml',
        'ptrcomp_fixup.h',
        'Makefile',
        'test.c'
    ], os.path.join('extras', 'alloc-pool'), os.path.join(dist, 'extras', 'alloc-pool'))

    copy_files([
        'README.rst',
        'cbordecode.py',
        'duk_cbor.c',
        'duk_cbor.h',
        'jsoncbor.c',
        'run_testvectors.js',
        'Makefile'
    ], os.path.join('extras', 'cbor'), os.path.join(dist, 'extras', 'cbor'))

    copy_files([
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
    ], 'dist-files', dist)

    copy_and_replace(os.path.join('dist-files', 'Makefile.sharedlibrary'), os.path.join(dist, 'Makefile.sharedlibrary'), {
        '@DUK_VERSION@': str(duk_version),
        '@SONAME_VERSION@': str(int(duk_version / 100))  # 10500 -> 105
    })

    copy_and_replace(os.path.join('dist-files', 'README.rst'), os.path.join(dist, 'README.rst'), {
        '@DUK_VERSION_FORMATTED@': duk_version_formatted,
        '@DUK_MAJOR@': str(duk_major),
        '@DUK_MINOR@': str(duk_minor),
        '@DUK_PATCH@': str(duk_patch),
        '@GIT_COMMIT@': git_commit,
        '@GIT_DESCRIBE@': git_describe,
        '@GIT_BRANCH@': git_branch
    })

    copy_files([
        'LICENSE.txt',  # not strict RST so keep .txt suffix
        'AUTHORS.rst'
    ], '.', os.path.join(dist))

    # RELEASES.rst is only updated in master.  It's not included in the dist to
    # make maintenance fixes easier to make.

    copy_files([
        'murmurhash2.txt',
        'lua.txt',
        'commonjs.txt',
        'xoroshiro128plus.txt',
        'splitmix64.txt'
    ], 'licenses', os.path.join(dist, 'licenses'))

    # Merge debugger metadata.

    merged = exec_print_stdout([
        sys.executable, os.path.join('tools', 'merge_debug_meta.py'),
        '--output', os.path.join(dist, 'debugger', 'duk_debug_meta.json'),
        '--class-names', os.path.join('debugger', 'duk_classnames.yaml'),
        '--debug-commands', os.path.join('debugger', 'duk_debugcommands.yaml'),
        '--debug-errors', os.path.join('debugger', 'duk_debugerrors.yaml'),
        '--opcodes', os.path.join('debugger', 'duk_opcodes.yaml')
    ] + forward_loglevel)

    # Add a build metadata file.

    doc = {
        'type': 'duk_dist_meta',
        'comment': 'Metadata for Duktape distributable',
        'git_commit': git_commit,
        'git_branch': git_branch,
        'git_describe': git_describe,
        'duk_version': duk_version,
        'duk_version_string': duk_version_formatted
    }
    with open(os.path.join(dist, 'duk_dist_meta.json'), 'wb') as f:
        f.write(json.dumps(doc, indent=4))

    # Build prepared sources with default config.  This is done using
    # tools and metadata in the dist directory.

    logger.debug('Create prepared sources for default configuration')

    def prep_default_sources(dirname, extraopts):
        cmd = [
            sys.executable, os.path.join(dist, 'tools', 'configure.py'),
            '--source-directory', os.path.join(dist, 'src-input'),
            '--output-directory', os.path.join(dist, dirname),
            '--config-metadata', os.path.join(dist, 'config'),
            '--git-commit', git_commit, '--git-describe', git_describe, '--git-branch', git_branch,
            '--omit-removed-config-options', '--omit-unused-config-options',
            '--emit-config-sanity-check', '--emit-legacy-feature-check'
        ]
        cmd += extraopts
        if opts.rom_support:
            cmd.append('--rom-support')
        if opts.rom_auto_lightfunc:
            cmd.append('--rom-auto-lightfunc')
        for i in opts.user_builtin_metadata:
            cmd.append('--user-builtin-metadata')
            cmd.append(i)
        cmd += forward_loglevel
        exec_print_stdout(cmd)

    prep_default_sources('src', [])

    # Duktape 2.x:
    #prep_default_sources('src', [ '--line-directives' ])
    #prep_default_sources('src-noline', [])
    #prep_default_sources('src-separate', [ '--separate-sources' ])

    # Clean up remaining temp files.

    delete_matching_files(dist, lambda x: x[-4:] == '.tmp')
    delete_matching_files(os.path.join(dist, 'tools'), lambda x: x[-4:] == '.pyc')

    # Create SPDX license once all other files are in place (and cleaned).

    if opts.create_spdx:
        logger.debug('Create SPDX license')
        try:
            exec_get_stdout([
                sys.executable,
                os.path.join('tools', 'create_spdx_license.py'),
                os.path.join(dist, 'license.spdx')
            ])
        except:
            logger.warning('')
            logger.warning('***')
            logger.warning('*** WARNING: Failed to create SPDX license, this should not happen for an official release!')
            logger.warning('***')
            logger.warning('')
    else:
        logger.debug('Skip SPDX license creation')

    logger.info('Dist finished successfully')

if __name__ == '__main__':
    main()
