#!/usr/bin/env python2
#
#  Create a distributable Duktape package into 'dist' directory.  The contents
#  of this directory can then be packaged into a source distributable.
#
#  The distributed source files contain all profiles and variants in one.
#  A developer should be able to use the distributed source as follows:
#
#    1. Add the Duktape source files to their project, whichever build
#       tool they use (make, scons, etc)
#
#    2. Add the Duktape header files to their include path.
#
#    3. Optionally define some DUK_OPT_xxx feature options.
#
#    4. Compile their program (which uses Duktape API).
#
#  In addition to sources, documentation, example programs, and some
#  example Makefiles are packaged into the dist package.
#

import os
import sys
import re
import shutil
import glob
import optparse
import subprocess
import tarfile

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

def glob_files(pattern):
	return glob.glob(pattern)

def cstring(x):
	return '"' + x + '"'  # good enough for now

# DUK_VERSION is grepped from duk_api_public.h.in: it is needed for the
# public API and we want to avoid defining it in two places.
def get_duk_version():
	r = re.compile(r'^#define\s+DUK_VERSION\s+(.*?)L?\s*$')
	with open(os.path.join('src', 'duk_api_public.h.in'), 'rb') as f:
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
	if os.path.isdir(dist):
		shutil.rmtree(dist)
	mkdir(dist)
	mkdir(os.path.join(dist, 'src-separate'))
	mkdir(os.path.join(dist, 'src'))
	mkdir(os.path.join(dist, 'src-noline'))
	mkdir(os.path.join(dist, 'config'))
	mkdir(os.path.join(dist, 'extras'))
	mkdir(os.path.join(dist, 'extras', 'duk-v1-compat'))
	mkdir(os.path.join(dist, 'extras', 'print-alert'))
	mkdir(os.path.join(dist, 'extras', 'console'))
	mkdir(os.path.join(dist, 'extras', 'logging'))
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

# Path check

if not (os.path.isfile(os.path.join('src', 'duk_api_public.h.in')) and \
        os.path.isfile(os.path.join('config', 'genconfig.py'))):
	sys.stderr.write('\n')
	sys.stderr.write('*** Working directory must be Duktape repo checkout root!\n')
	sys.stderr.write('\n')
	raise Exception('Incorrect working directory')

# Option parsing

parser = optparse.OptionParser()
parser.add_option('--create-spdx', dest='create_spdx', action='store_true', default=False, help='Create SPDX license file')
parser.add_option('--minify', dest='minify', default='none', help='Minifier: none, closure, uglifyjs, uglifyjs2')
parser.add_option('--git-commit', dest='git_commit', default=None, help='Force git commit hash')
parser.add_option('--git-describe', dest='git_describe', default=None, help='Force git describe')
parser.add_option('--git-branch', dest='git_branch', default=None, help='Force git branch name')
parser.add_option('--rom-support', dest='rom_support', action='store_true', help='Add support for ROM strings/objects (increases duktape.c size considerably)')
parser.add_option('--user-builtin-metadata', dest='user_builtin_metadata', action='append', default=[], help='User strings and objects to add, YAML format (can be repeated for multiple overrides)')
(opts, args) = parser.parse_args()

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

	try:
		if opts.create_spdx:
			import rdflib
	except:
		_warning('rdflib', 'python-rdflib', 'rdflib')
		failed = True

	if failed:
		sys.stderr.write('\n')
		raise Exception('Missing some required Python modules')

check_python_modules()

# Figure out directories, git info, etc

entry_pwd = os.getcwd()
dist = os.path.join(entry_pwd, 'dist')
distsrcsep = os.path.join(dist, 'src-separate')
distsrccom = os.path.join(dist, 'src')
distsrcnol = os.path.join(dist, 'src-noline')  # src-noline/duktape.c is same as src/duktape.c
                                               # but without line directives
                                               # https://github.com/svaarala/duktape/pull/363

duk_version, duk_major, duk_minor, duk_patch, duk_version_formatted = get_duk_version()

if opts.git_commit is not None:
	git_commit = opts.git_commit
else:
	git_commit = exec_get_stdout([ 'git', 'rev-parse', 'HEAD' ], default='external').strip()
git_commit_cstring = cstring(git_commit)

if opts.git_describe is not None:
	git_describe = opts.git_describe
else:
	git_describe = exec_get_stdout([ 'git', 'describe', '--always', '--dirty' ], default='external').strip()
git_describe_cstring = cstring(git_describe)

if opts.git_branch is not None:
	git_branch = opts.git_branch
else:
	git_branch = exec_get_stdout([ 'git', 'rev-parse', '--abbrev-ref', 'HEAD' ], default='external').strip()
git_branch_cstring = cstring(git_branch)

print('Dist for Duktape version %s, commit %s, describe %s, branch %s' % \
      (duk_version_formatted, git_commit, git_describe, git_branch))

print('Create dist directories and copy static files')

# Create dist directory structure

create_dist_directories(dist)

# Copy most files directly

os.chdir(entry_pwd)
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
	'duk_debug_heap.c',
	'duk_debug_macros.c',
	'duk_debug_vsnprintf.c',
	'duk_error_augment.c',
	'duk_error.h',
	'duk_error_longjmp.c',
	'duk_error_macros.c',
	'duk_error_misc.c',
	'duk_error_throw.c',
	'duk_forwdecl.h',
	'duk_hbuffer_alloc.c',
	'duk_hbuffer.h',
	'duk_hbuffer_ops.c',
	'duk_hcompiledfunction.h',
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
	'duk_hnativefunction.h',
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
	'duk_hbufferobject.h',
	'duk_hbufferobject_misc.c',
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
], 'src', distsrcsep)

os.chdir(os.path.join(entry_pwd, 'config'))
create_targz(os.path.join(dist, 'config', 'genconfig_metadata.tar.gz'), [
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
	'other-defines',
	'examples'
])
os.chdir(entry_pwd)

copy_files([
	'README.rst',
	'genconfig.py'
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
	'duk_opcodes.yaml',
	'merge_debug_meta.py'
], 'debugger', os.path.join(dist, 'debugger'))
copy_files([
	'index.html',
	'style.css',
	'webui.js'
], os.path.join('debugger', 'static'), os.path.join(dist, 'debugger', 'static'))

copy_files([
	'console-minimal.js',
	'object-prototype-definegetter.js',
	'object-prototype-definesetter.js',
	'object-assign.js',
	'performance-now.js',
	'duktape-isfastint.js',
	'duktape-error-setter-writable.js',
	'duktape-error-setter-nonwritable.js'
], 'polyfills', os.path.join(dist, 'polyfills'))

copy_files([
	'README.rst'
], 'examples', os.path.join(dist, 'examples'))

copy_files([
	'README.rst',
	'duk_cmdline.c',
	'duk_cmdline_ajduk.c'
], os.path.join('examples', 'cmdline'), os.path.join(dist, 'examples', 'cmdline'))

copy_files([
	'README.rst',
	'c_eventloop.c',
	'c_eventloop.js',
	'ecma_eventloop.js',
	'main.c',
	'poll.c',
	'ncurses.c',
	'socket.c',
	'fileio.c',
	'curses-timers.js',
	'basic-test.js',
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
	'duk_v1_compat.h'
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
	'Makefile.cmdline',
	'Makefile.dukdebug',
	'Makefile.eventloop',
	'Makefile.hello',
	'Makefile.eval',
	'Makefile.coffee',
	'Makefile.jxpretty',
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
	'commonjs.txt'
], 'licenses', os.path.join(dist, 'licenses'))

# Build temp versions of LICENSE.txt and AUTHORS.rst for embedding into
# autogenerated C/H files.

copy_and_cquote('LICENSE.txt', os.path.join(dist, 'LICENSE.txt.tmp'))
copy_and_cquote('AUTHORS.rst', os.path.join(dist, 'AUTHORS.rst.tmp'))

print('Create duk_config.h headers')

# Merge debugger metadata.
merged = exec_print_stdout([
	sys.executable, os.path.join('debugger', 'merge_debug_meta.py'),
	'--output', os.path.join(dist, 'debugger', 'duk_debug_meta.json'),
	'--class-names', os.path.join('debugger', 'duk_classnames.yaml'),
	'--debug-commands', os.path.join('debugger', 'duk_debugcommands.yaml'),
	'--debug-errors', os.path.join('debugger', 'duk_debugerrors.yaml'),
	'--opcodes', os.path.join('debugger', 'duk_opcodes.yaml')
])

# Build default duk_config.h from snippets using genconfig.
exec_print_stdout([
	sys.executable, os.path.join('config', 'genconfig.py'), '--metadata', 'config',
	'--output', os.path.join(dist, 'duk_config.h.tmp'),
	'--git-commit', git_commit, '--git-describe', git_describe, '--git-branch', git_branch,
	'--omit-removed-config-options', '--omit-unused-config-options',
	'--emit-config-sanity-check',
	'--support-feature-options',
	'duk-config-header'
])

copy_file(os.path.join(dist, 'duk_config.h.tmp'), os.path.join(distsrccom, 'duk_config.h'))
copy_file(os.path.join(dist, 'duk_config.h.tmp'), os.path.join(distsrcnol, 'duk_config.h'))
copy_file(os.path.join(dist, 'duk_config.h.tmp'), os.path.join(distsrcsep, 'duk_config.h'))
#copy_file(os.path.join(dist, 'duk_config.h.tmp'), os.path.join(dist, 'config', 'duk_config.h-autodetect'))

# Build duk_config.h without feature option support.
exec_print_stdout([
	sys.executable, os.path.join('config', 'genconfig.py'), '--metadata', 'config',
	'--output', os.path.join(dist, 'config', 'duk_config.h-modular-static'),
	'--git-commit', git_commit, '--git-describe', git_describe, '--git-branch', git_branch,
	'--omit-removed-config-options', '--omit-unused-config-options',
	'--emit-legacy-feature-check', '--emit-config-sanity-check',
	'duk-config-header'
])
exec_print_stdout([
	sys.executable, os.path.join('config', 'genconfig.py'), '--metadata', 'config',
	'--output', os.path.join(dist, 'config', 'duk_config.h-modular-dll'),
	'--git-commit', git_commit, '--git-describe', git_describe, '--git-branch', git_branch,
	'--omit-removed-config-options', '--omit-unused-config-options',
	'--emit-legacy-feature-check', '--emit-config-sanity-check',
	'--dll',
	'duk-config-header'
])

# Generate a few barebones config examples
def genconfig_barebones(platform, architecture, compiler):
	exec_print_stdout([
		sys.executable, os.path.join('config', 'genconfig.py'), '--metadata', 'config',
		'--output', os.path.join(dist, 'config', 'duk_config.h-%s-%s-%s' % (platform, architecture, compiler)),
		'--git-commit', git_commit, '--git-describe', git_describe, '--git-branch', git_branch,
		'--platform', platform, '--architecture', architecture, '--compiler', compiler,
		'--omit-removed-config-options', '--omit-unused-config-options',
		'--emit-legacy-feature-check', '--emit-config-sanity-check',
		'duk-config-header'
	])

#genconfig_barebones('linux', 'x86', 'gcc')
#genconfig_barebones('linux', 'x64', 'gcc')
#genconfig_barebones('linux', 'x86', 'clang')
#genconfig_barebones('linux', 'x64', 'clang')
#genconfig_barebones('windows', 'x86', 'msvc')
#genconfig_barebones('windows', 'x64', 'msvc')
#genconfig_barebones('apple', 'x86', 'gcc')
#genconfig_barebones('apple', 'x64', 'gcc')
#genconfig_barebones('apple', 'x86', 'clang')
#genconfig_barebones('apple', 'x64', 'clang')

# Build duktape.h from parts, with some git-related replacements.
# The only difference between single and separate file duktape.h
# is the internal DUK_SINGLE_FILE define.
#
# Newline after 'i \':
# http://stackoverflow.com/questions/25631989/sed-insert-line-command-osx
copy_and_replace(os.path.join('src', 'duktape.h.in'), os.path.join(distsrccom, 'duktape.h'), {
	'@DUK_SINGLE_FILE@': '#define DUK_SINGLE_FILE',
	'@LICENSE_TXT@': read_file(os.path.join(dist, 'LICENSE.txt.tmp'), strip_last_nl=True),
	'@AUTHORS_RST@': read_file(os.path.join(dist, 'AUTHORS.rst.tmp'), strip_last_nl=True),
	'@DUK_API_PUBLIC_H@': read_file(os.path.join('src', 'duk_api_public.h.in'), strip_last_nl=True),
	'@DUK_DBLUNION_H@': read_file(os.path.join('src', 'duk_dblunion.h.in'), strip_last_nl=True),
	'@DUK_VERSION_FORMATTED@': duk_version_formatted,
	'@GIT_COMMIT@': git_commit,
	'@GIT_COMMIT_CSTRING@': git_commit_cstring,
	'@GIT_DESCRIBE@': git_describe,
	'@GIT_DESCRIBE_CSTRING@': git_describe_cstring,
	'@GIT_BRANCH@': git_branch,
	'@GIT_BRANCH_CSTRING@': git_branch_cstring
})
# keep the line so line numbers match between the two variant headers
copy_and_replace(os.path.join(distsrccom, 'duktape.h'), os.path.join(distsrcsep, 'duktape.h'), {
	'#define DUK_SINGLE_FILE': '#undef DUK_SINGLE_FILE'
})
copy_file(os.path.join(distsrccom, 'duktape.h'), os.path.join(distsrcnol, 'duktape.h'))

# Initjs code: built-in Ecmascript code snippets which are evaluated when
# a new global context is created.  There are multiple minifiers, closure
# seems to be doing the best job so only that is enabled now.  Obfuscating
# the code is not a goal, although that happens as an unwanted side effect.
#
# Closure compiler --compilation_level ADVANCED_OPTIMIZATIONS breaks some
# of the existing code, so don't use it.

# XXX: currently assumes minifiers are present in static paths, it'd be
# better to take the minifier path as an argument.

def minify_none(src):
	return read_file(src)

	copy_file(os.path.join('src', 'duk_initjs.js'), os.path.join(distsrcsep, 'duk_initjs_min.js'))

def minify_uglifyjs(src):
	ret = exec_get_stdout([
		'UglifyJS/bin/uglifyjs',
		'--ascii',
		'--no-dead-code',
		'--no-copyright'
	], input=read_file(src))
	return ret

def minify_uglifyjs2(src):
	ret = exec_get_stdout([
		'UglifyJS2/bin/uglifyjs',
		src,
		'--screw-ie8',
		'--compress warnings=false'
	])
	return ret

def minify_closure(src):
	ret = exec_get_stdout([
		'java',
		'-jar', 'compiler.jar',
		'--warning_level','QUIET',
		'--language_in', 'ECMASCRIPT5',
		'--compilation_level', 'SIMPLE_OPTIMIZATIONS',
		src
	])
	return ret

initjs_src = os.path.join('src', 'duk_initjs.js')
if opts.minify == 'none':
	print('*** No minifier, this should not happen for an official build')
	initjs_out = minify_none(initjs_src)
	print('No minifier: %d bytes' % len(initjs_out))
elif opts.minify == 'uglifyjs':
	initjs_out = minify_uglifyjs(initjs_src)
	print('Minified using UglifyJS: %d bytes' % len(initjs_out))
elif opts.minify == 'uglifyjs2':
	initjs_out = minify_uglifyjs2(initjs_src)
	print('Minified using UglifyJS2: %d bytes' % len(initjs_out))
elif opts.minify == 'closure':
	initjs_out = minify_closure(initjs_src)
	print('Minified using Closure: %d bytes' % len(initjs_out))
else:
	raise Exception('invalid minifier: %r' % opts.minify)

with open(os.path.join(distsrcsep, 'duk_initjs_min.js'), 'wb') as f:
	f.write(initjs_out)

# Autogenerated strings and built-in files
#
# There are currently no profile specific variants of strings/builtins, but
# this will probably change when functions are added/removed based on profile.

exec_print_stdout([
	sys.executable,
	os.path.join('src', 'genbuildparams.py'),
	'--version=' + str(duk_version),
	'--git-commit=' + git_commit,
	'--git-describe=' + git_describe,
	'--git-branch=' + git_branch,
	'--out-json=' + os.path.join(distsrcsep, 'buildparams.json.tmp'),
	'--out-header=' + os.path.join(distsrcsep, 'duk_buildparams.h.tmp')
])

res = exec_get_stdout([
	sys.executable,
	os.path.join('src', 'scan_used_stridx_bidx.py')
] + glob_files(os.path.join('src', '*.c')) \
  + glob_files(os.path.join('src', '*.h')) \
  + glob_files(os.path.join('src', '*.h.in'))
)
with open(os.path.join(dist, 'duk_used_stridx_bidx_defs.json.tmp'), 'wb') as f:
	f.write(res)

gb_opts = []
if opts.rom_support:
	# ROM string/object support is not enabled by default because
	# it increases the generated duktape.c considerably.
	print('Enabling --rom-support for genbuiltins.py')
	gb_opts.append('--rom-support')
for fn in opts.user_builtin_metadata:
	print('Forwarding --user-builtin-metadata %s' % fn)
	gb_opts.append('--user-builtin-metadata')
	gb_opts.append(fn)
exec_print_stdout([
	sys.executable,
	os.path.join('src', 'genbuiltins.py'),
	'--buildinfo=' + os.path.join(distsrcsep, 'buildparams.json.tmp'),
	'--used-stridx-metadata=' + os.path.join(dist, 'duk_used_stridx_bidx_defs.json.tmp'),
	'--strings-metadata=' + os.path.join('src', 'strings.yaml'),
	'--objects-metadata=' + os.path.join('src', 'builtins.yaml'),
	'--initjs-data=' + os.path.join(distsrcsep, 'duk_initjs_min.js'),
	'--out-header=' + os.path.join(distsrcsep, 'duk_builtins.h'),
	'--out-source=' + os.path.join(distsrcsep, 'duk_builtins.c'),
	'--out-metadata-json=' + os.path.join(dist, 'duk_build_meta.json')
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
	os.path.join('src', 'prepare_unicode_data.py'),
	os.path.join('src', 'UnicodeData.txt'),
	os.path.join(distsrcsep, 'UnicodeData-expanded.tmp')
])

def extract_chars(incl, excl, suffix):
	#print('- extract_chars: %s %s %s' % (incl, excl, suffix))
	res = exec_get_stdout([
		sys.executable,
		os.path.join('src', 'extract_chars.py'),
		'--unicode-data=' + os.path.join(distsrcsep, 'UnicodeData-expanded.tmp'),
		'--include-categories=' + incl,
		'--exclude-categories=' + excl,
		'--out-source=' + os.path.join(distsrcsep, 'duk_unicode_%s.c.tmp' % suffix),
		'--out-header=' + os.path.join(distsrcsep, 'duk_unicode_%s.h.tmp' % suffix),
		'--table-name=' + 'duk_unicode_%s' % suffix
	])
	with open(os.path.join(distsrcsep, suffix + '.txt'), 'wb') as f:
		f.write(res)

def extract_caseconv():
	#print('- extract_caseconv case conversion')
	res = exec_get_stdout([
		sys.executable,
		os.path.join('src', 'extract_caseconv.py'),
		'--command=caseconv_bitpacked',
		'--unicode-data=' + os.path.join(distsrcsep, 'UnicodeData-expanded.tmp'),
		'--special-casing=' + os.path.join('src', 'SpecialCasing.txt'),
		'--out-source=' + os.path.join(distsrcsep, 'duk_unicode_caseconv.c.tmp'),
		'--out-header=' + os.path.join(distsrcsep, 'duk_unicode_caseconv.h.tmp'),
		'--table-name-lc=duk_unicode_caseconv_lc',
		'--table-name-uc=duk_unicode_caseconv_uc'
	])
	with open(os.path.join(distsrcsep, 'caseconv.txt'), 'wb') as f:
		f.write(res)

	#print('- extract_caseconv canon lookup')
	res = exec_get_stdout([
		sys.executable,
		os.path.join('src', 'extract_caseconv.py'),
		'--command=re_canon_lookup',
		'--unicode-data=' + os.path.join(distsrcsep, 'UnicodeData-expanded.tmp'),
		'--special-casing=' + os.path.join('src', 'SpecialCasing.txt'),
		'--out-source=' + os.path.join(distsrcsep, 'duk_unicode_re_canon_lookup.c.tmp'),
		'--out-header=' + os.path.join(distsrcsep, 'duk_unicode_re_canon_lookup.h.tmp'),
		'--table-name-re-canon-lookup=duk_unicode_re_canon_lookup'
	])
	with open(os.path.join(distsrcsep, 'caseconv_re_canon_lookup.txt'), 'wb') as f:
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

copy_and_replace(os.path.join(distsrcsep, 'duk_unicode.h'), os.path.join(distsrcsep, 'duk_unicode.h'), {
	'#include "duk_unicode_ids_noa.h"': read_file(os.path.join(distsrcsep, 'duk_unicode_ids_noa.h.tmp'), strip_last_nl=True),
	'#include "duk_unicode_ids_noabmp.h"': read_file(os.path.join(distsrcsep, 'duk_unicode_ids_noabmp.h.tmp'), strip_last_nl=True),
	'#include "duk_unicode_ids_m_let_noa.h"': read_file(os.path.join(distsrcsep, 'duk_unicode_ids_m_let_noa.h.tmp'), strip_last_nl=True),
	'#include "duk_unicode_ids_m_let_noabmp.h"': read_file(os.path.join(distsrcsep, 'duk_unicode_ids_m_let_noabmp.h.tmp'), strip_last_nl=True),
	'#include "duk_unicode_idp_m_ids_noa.h"': read_file(os.path.join(distsrcsep, 'duk_unicode_idp_m_ids_noa.h.tmp'), strip_last_nl=True),
	'#include "duk_unicode_idp_m_ids_noabmp.h"': read_file(os.path.join(distsrcsep, 'duk_unicode_idp_m_ids_noabmp.h.tmp'), strip_last_nl=True),
	'#include "duk_unicode_caseconv.h"': read_file(os.path.join(distsrcsep, 'duk_unicode_caseconv.h.tmp'), strip_last_nl=True),
	'#include "duk_unicode_re_canon_lookup.h"': read_file(os.path.join(distsrcsep, 'duk_unicode_re_canon_lookup.h.tmp'), strip_last_nl=True)
})

copy_and_replace(os.path.join(distsrcsep, 'duk_unicode_tables.c'), os.path.join(distsrcsep, 'duk_unicode_tables.c'), {
	'#include "duk_unicode_ids_noa.c"': read_file(os.path.join(distsrcsep, 'duk_unicode_ids_noa.c.tmp'), strip_last_nl=True),
	'#include "duk_unicode_ids_noabmp.c"': read_file(os.path.join(distsrcsep, 'duk_unicode_ids_noabmp.c.tmp'), strip_last_nl=True),
	'#include "duk_unicode_ids_m_let_noa.c"': read_file(os.path.join(distsrcsep, 'duk_unicode_ids_m_let_noa.c.tmp'), strip_last_nl=True),
	'#include "duk_unicode_ids_m_let_noabmp.c"': read_file(os.path.join(distsrcsep, 'duk_unicode_ids_m_let_noabmp.c.tmp'), strip_last_nl=True),
	'#include "duk_unicode_idp_m_ids_noa.c"': read_file(os.path.join(distsrcsep, 'duk_unicode_idp_m_ids_noa.c.tmp'), strip_last_nl=True),
	'#include "duk_unicode_idp_m_ids_noabmp.c"': read_file(os.path.join(distsrcsep, 'duk_unicode_idp_m_ids_noabmp.c.tmp'), strip_last_nl=True),
	'#include "duk_unicode_caseconv.c"': read_file(os.path.join(distsrcsep, 'duk_unicode_caseconv.c.tmp'), strip_last_nl=True),
	'#include "duk_unicode_re_canon_lookup.c"': read_file(os.path.join(distsrcsep, 'duk_unicode_re_canon_lookup.c.tmp'), strip_last_nl=True)
})

# Clean up some temporary files

delete_matching_files(distsrcsep, lambda x: x[-4:] == '.tmp')
delete_matching_files(distsrcsep, lambda x: x in [
	'ws.txt',
	'let.txt', 'let_noa.txt', 'let_noabmp.txt',
	'ids.txt', 'ids_noa.txt', 'ids_noabmp.txt',
	'ids_m_let.txt', 'ids_m_let_noa.txt', 'ids_m_let_noabmp.txt',
	'idp_m_ids.txt', 'idp_m_ids_noa.txt', 'idp_m_ids_noabmp.txt'
])
delete_matching_files(distsrcsep, lambda x: x[0:8] == 'caseconv' and x[-4:] == '.txt')

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

exec_print_stdout([
	sys.executable,
	os.path.join('util', 'combine_src.py'),
	'--source-dir', distsrcsep,
	'--output-source', os.path.join(distsrccom, 'duktape.c'),
	'--output-metadata', os.path.join(distsrccom, 'metadata.json'),
	'--duk-version', str(duk_version),
	'--git-commit', git_commit,
	'--git-describe', git_describe,
	'--git-branch', git_branch,
	'--license-file', os.path.join(dist, 'LICENSE.txt.tmp'),
	'--authors-file', os.path.join(dist, 'AUTHORS.rst.tmp'),
	'--line-directives'
])

exec_print_stdout([
	sys.executable,
	os.path.join('util', 'combine_src.py'),
	'--source-dir', distsrcsep,
	'--output-source', os.path.join(distsrcnol, 'duktape.c'),
	'--output-metadata', os.path.join(distsrcnol, 'metadata.json'),
	'--duk-version', str(duk_version),
	'--git-commit', git_commit,
	'--git-describe', git_describe,
	'--git-branch', git_branch,
	'--license-file', os.path.join(dist, 'LICENSE.txt.tmp'),
	'--authors-file', os.path.join(dist, 'AUTHORS.rst.tmp')
])

# Clean up remaining temp files
delete_matching_files(dist, lambda x: x[-4:] == '.tmp')

# Create SPDX license once all other files are in place (and cleaned)
if opts.create_spdx:
	print('Create SPDX license')
	exec_get_stdout([
		sys.executable,
		os.path.join('util', 'create_spdx_license.py'),
		os.path.join(dist, 'license.spdx')
	])
else:
	print('Skip SPDX license creation')

print('Dist finished successfully')
