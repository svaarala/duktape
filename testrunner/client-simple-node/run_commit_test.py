#!/usr/bin/env python2
#
#  Python commit test run script.
#
#  Intended to work on Linux, OS X, and Windows (both Cygwin and command
#  prompt).  Some notes for portability:
#
#    * Use os.path.join() to join paths.
#
#    * Use tarfile, zipfile, etc instead of native commands to avoid awkward
#      situations on Windows, e.g. we might be running from the command prompt
#      and executing 'tar' which is provided by Cygwin.  Paths will work very
#      badly in such situations.  In general, don't mix Cygwin / non-Cygwin
#      commands on Windows.
#
#    * Avoid Duktape Makefile targets: they sometimes depend on /tmp which
#      interferes with parallel runs.
#
#    * Avoid mixing Cygwin and non-Cygwin repo snapshots (and git commands),
#      as there are issues with mixed use like permissions causing unintended
#      diffs.
#

import os
import sys
import re
import optparse
import subprocess
import datetime
import traceback
import tarfile
import zipfile

#
#  Parameters and option parsing, some control globals
#

# Whitelisted repos, limit to main repo for now.
repo_whitelist = [
	'svaarala/duktape'
]

# Strict reponame filter.
re_reponame = re.compile(r'^[a-zA-Z0-9/-]+$')

# Parse arguments.
parser = optparse.OptionParser()
parser.add_option('--repo-full-name', dest='repo_full_name', help='Full name of repository, e.g. "svaarala/duktape"')
parser.add_option('--repo-clone-url', dest='repo_clone_url', help='Repo HTTPS clone URI, e.g. "https://github.com/svaarala/duktape.git"')
parser.add_option('--commit-name', dest='commit_name', help='Commit SHA hash or tag name')
parser.add_option('--context', dest='context', help='Context identifying test type, e.g. "linux-x64-qecmatest"')
parser.add_option('--temp-dir', dest='temp_dir', help='Automatic temp dir created by testclient, automatically deleted (recursively) by testclient when test is done')
parser.add_option('--repo-snapshot-dir', dest='repo_snapshot_dir', help='Directory for repo tar.gz snapshots for faster test init')

(opts, args) = parser.parse_args()
repo_full_name = opts.repo_full_name
assert(repo_full_name is not None)
repo_clone_url = opts.repo_clone_url
assert(repo_clone_url is not None)
commit_name = opts.commit_name
assert(commit_name is not None)
context = opts.context
assert(context is not None)
temp_dir = opts.temp_dir
assert(temp_dir is not None)
repo_snapshot_dir = opts.repo_snapshot_dir
assert(repo_snapshot_dir is not None)

#
#  Helpers
#

def newenv(**kw):
	ret = {}
	for k in os.environ.keys():
		ret[k] = str(os.environ[k])

	for k in kw.keys():
		ret[k] = str(kw[k])

	#print('Final environment: %r' % ret)
	return ret

def execute(cmd, env=None, catch=False, input='', dump_stdout=True, dump_stderr=True):
	print(' - ' + repr(cmd))

	success = True

	def dump(x):
		if isinstance(x, unicode):
			x = x.encode('utf-8')
		if len(x) == 0 or x[-1] != '\n':
			x = x + '\n'
		sys.stdout.write(x)

	proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
	ret = proc.communicate(input=input)
	if ret[0] != '' and dump_stdout:
		dump(ret[0])
	if ret[1] != '' and dump_stderr:
		dump(ret[1])
	if proc.returncode != 0:
		if catch:
			success = False
		else:
			raise Exception('command failed: %r' % cmd)

	return {
		'returncode': proc.returncode,
		'stdout': ret[0],
		'stderr': ret[1],
		'success': success
	}

def unpack_targz(fn):
	print('Extracting %s to %s' % (fn, os.getcwd()))
	t = tarfile.open(fn)
	t.extractall()
	t.close()

def unpack_zip(fn):
	print('Extracting %s to %s' % (fn, os.getcwd()))
	z = zipfile.ZipFile(fn, 'r')
	z.extractall()
	z.close()

def get_binary_size(fn):
	# Pattern works for Linux and OS X.
	res = execute([ 'size', fn ])
	m = re.compile(r'.*?^\s*(\d+)\s+(\d+)\s+(\d+)\s+(\d+).*?', re.MULTILINE | re.DOTALL).match(res['stdout'])
	if m is None:
		raise Exception('cannot figure out size for binary %r' % fn)
	return {
		'text': int(m.group(1)),
		'data': int(m.group(2)),
		'bss': int(m.group(3)),
		'total': int(m.group(4))
	}

def format_size_diff(newsz, oldsz):
	return '%d %d %d (%d %d %d): %d' % (
		newsz['text'], newsz['data'], newsz['bss'],
		oldsz['text'], oldsz['data'], oldsz['bss'],
		newsz['total'] - oldsz['total']
	)

output_description = None
def set_output_description(desc):
	global output_description
	output_description = desc

#
#  Test context handlers
#

def context_codepolicycheck():
	return execute([ 'make', 'codepolicycheck' ], env=newenv(TRAVIS=1), catch=True)['success']

def context_linux_x64_qecmatest():
	cwd = os.getcwd()
	execute([ 'make', 'duk', 'runtestsdeps' ])
	return execute([
		'node',
		os.path.join(cwd, 'runtests', 'runtests.js'),
		'--prep-test-path', os.path.join(cwd, 'util', 'prep_test.py'),
		'--minify-uglifyjs2', os.path.join(cwd, 'UglifyJS2', 'bin', 'uglifyjs'),
		'--util-include-path', os.path.join(cwd, 'tests', 'ecmascript'),
		'--known-issues', os.path.join(cwd, 'doc', 'testcase-known-issues.yaml'),
		'--run-duk', '--cmd-duk', os.path.join(cwd, 'duk'),
		'--num-threads', '1',
		'--log-file', os.path.join(cwd, 'test.out'),
		os.path.join(cwd, 'tests', 'ecmascript')
	], catch=True)['success']

def context_linux_x64_apitest():
	return execute([ 'make', 'apiprep' ], catch=True)['success']
	return execute([
		'node',
		os.path.join(cwd, 'runtests', 'runtests.js'),
		'--prep-test-path', os.path.join(cwd, 'util', 'prep_test.py'),
		'--minify-uglifyjs2', os.path.join(cwd, 'UglifyJS2', 'bin', 'uglifyjs'),
		'--util-include-path', os.path.join(cwd, 'tests', 'ecmascript'),
		'--known-issues', os.path.join(cwd, 'doc', 'testcase-known-issues.yaml'),
		'--run-duk', '--cmd-duk', os.path.join(cwd, 'duk'),
		'--num-threads', '1',
		'--log-file', os.path.join(cwd, 'test.out'),
		os.path.join(cwd, 'tests', 'api')
	], catch=True)['success']

def context_linux_x64_duk_clang():
	cwd = os.getcwd()
	execute([ 'make', 'duk-clang' ])
	res = execute([
		os.path.join(cwd, 'duk-clang'),
		'-e', 'print("hello world!");'
	])
	return res['stdout'] == 'hello world!\n'

def context_linux_x64_duk_gxx():
	cwd = os.getcwd()
	execute([ 'make', 'duk-g++' ])
	res = execute([
		os.path.join(cwd, 'duk-g++'),
		'-e', 'print("hello world!");'
	])
	return res['stdout'] == 'hello world!\n'

def context_helper_get_binary_size_diff(compfn):
	cwd = os.getcwd()
	execute([ 'git', 'clean', '-f' ])
	execute([ 'git', 'reset', '--hard' ])
	compfn()
	newsz = get_binary_size(os.path.join(cwd, 'duk'))
	execute([ 'git', 'clean', '-f' ])
	execute([ 'git', 'reset', '--hard' ])
	execute([ 'git', 'checkout', 'master' ])
	execute([ 'git', 'clean', '-f' ])
	execute([ 'git', 'reset', '--hard' ])
	execute([ 'make', 'clean' ])
	compfn()
	oldsz = get_binary_size(os.path.join(cwd, 'duk'))
	set_output_description(format_size_diff(newsz, oldsz))
	return True

def context_linux_x64_gcc_defsize_makeduk():
	cwd = os.getcwd()
	def comp():
		execute([ 'make', 'duk' ])
	return context_helper_get_binary_size_diff(comp)

def context_linux_x64_gcc_defsize_fltoetc():
	cwd = os.getcwd()
	def comp():
		execute([ 'make', 'dist' ])
		execute([
			'gcc', '-oduk',
			'-Os', '-fomit-frame-pointer',
			'-flto', '-fno-asynchronous-unwind-tables',
			'-ffunction-sections', '-Wl,--gc-sections',
			'-I' + os.path.join('dist', 'src'),
			'-I' + os.path.join('dist', 'examples', 'cmdline'),
			os.path.join(cwd, 'dist', 'src', 'duktape.c'),
			os.path.join(cwd, 'dist', 'examples', 'cmdline', 'duk_cmdline.c'),
			'-lm'
		])
	return context_helper_get_binary_size_diff(comp)

def context_helper_minsize_fltoetc(archopt):
	cwd = os.getcwd()
	def comp():
		execute([ 'make', 'dist' ])
		execute([
			'python2', os.path.join(cwd, 'config', 'genconfig.py'),
			'--metadata', os.path.join(cwd, 'config'),
			'--output', os.path.join(cwd, 'dist', 'src', 'duk_config.h'),
			'--option-file', os.path.join(cwd, 'config', 'examples', 'low_memory.yaml'),
			'duk-config-header'
		])
		execute([
			'gcc', '-oduk', archopt,
			'-Os', '-fomit-frame-pointer',
			'-flto', '-fno-asynchronous-unwind-tables',
			'-ffunction-sections', '-Wl,--gc-sections',
			'-I' + os.path.join('dist', 'src'),
			'-I' + os.path.join('dist', 'examples', 'cmdline'),
			os.path.join(cwd, 'dist', 'src', 'duktape.c'),
			os.path.join(cwd, 'dist', 'examples', 'cmdline', 'duk_cmdline.c'),
			'-lm'
		])
	return context_helper_get_binary_size_diff(comp)

def context_linux_x64_gcc_minsize_fltoetc():
	return context_helper_minsize_fltoetc('-m64')

def context_linux_x86_gcc_minsize_fltoetc():
	return context_helper_minsize_fltoetc('-m32')

def context_linux_x32_gcc_minsize_fltoetc():
	return context_helper_minsize_fltoetc('-mx32')

def context_linux_x64_cpp_exceptions():
	# For now rather simple: compile, run, and grep for my_class
	# destruction prints.  There are only 3 without C++ exceptions
	# and 15 with them.

	cwd = os.getcwd()

	execute([ 'make', 'dist' ])
	execute([
		'g++', '-oduk-cpp-exc',
		'-DDUK_OPT_CPP_EXCEPTIONS',
		'-I' + os.path.join(cwd, 'dist', 'src'),
		os.path.join(cwd, 'dist', 'src', 'duktape.c'),
		os.path.join(cwd, 'examples', 'cpp-exceptions', 'cpp_exceptions.cpp'),
		'-lm'
	])

	res = execute([
		os.path.join(cwd, 'duk-cpp-exc')
	])
	count = 0
	for line in res['stdout'].split('\n'):
		if 'my_class instance destroyed' in line:
			count += 1
	print('Destruct count: %d' % count)

	if count >= 15:
		print('C++ exceptions seem to be working')
		return True
	else:
		print('C++ exceptions don\'t seem to be working')
		return False

def context_linux_x86_ajduk():
	cwd = os.getcwd()
	execute([ 'make', 'ajduk' ])
	res = execute([
		os.path.join(cwd, 'ajduk'),
		'-e', 'print("hello world!");'
	])
	return 'hello world!\n' in res['stdout']

def context_linux_x86_ajduk_rombuild():
	cwd = os.getcwd()

	execute([
		'bash',
		os.path.join(cwd, 'util', 'example_rombuild.sh'),
	])

	got_hello = False
	got_startrek = False

	res = execute([
		os.path.join(cwd, 'ajduk'),
		'-e', 'print("hello world!");'
	])
	got_hello = ('hello world!\n' in res['stdout'])  # ajduk stdout has pool dumps etc
	print('Got hello: %r' % got_hello)

	res = execute([
		os.path.join(cwd, 'ajduk'),
		'-e', 'print("StarTrek.ent:", StarTrek.ent);'
	])
	got_startrek = ('StarTrek.ent: true\n' in res['stdout'])
	print('Got StarTrek: %r' % got_startrek)

	return got_hello and got_startrek

def context_linux_x64_test262test():
	cwd = os.getcwd()

	execute([ 'make', 'duk' ])

	# Unpack separately, 'make clean' wipes this.
	unpack_targz(os.path.join(repo_snapshot_dir, 'test262-es5-tests.tar.gz'))
	unpack_zip(os.path.join(cwd, 'es5-tests.zip'))

	os.chdir(os.path.join(cwd, 'test262-es5-tests'))
	res = execute([
		'python2',
		os.path.join(cwd, 'test262-es5-tests', 'tools', 'packaging', 'test262.py'),
		'--command', os.path.join(cwd, 'duk') + ' {{path}}'
	], dump_stdout=False, dump_stderr=True)
	test262_log = res['stdout']
	os.chdir(cwd)

	res = execute([
		'python2',
		os.path.join(cwd, 'util', 'filter_test262_log.py'),
		os.path.join(cwd, 'doc', 'test262-known-issues.yaml')
	], input=test262_log)

	# Test result plumbing a bit awkward but works for now.
	# Known and diagnosed issues are considered a "pass" for
	# Github status.
	return 'TEST262 SUCCESS\n' in res['stdout']

context_handlers = {
	# Linux

	'codepolicycheck': context_codepolicycheck,

	'linux-x64-qecmatest': context_linux_x64_qecmatest,
	'linux-x64-apitest': context_linux_x64_apitest,
	'linux-x64-test262test': context_linux_x64_test262test,

	'linux-x64-duk-clang': context_linux_x64_duk_clang,
	'linux-x64-duk-gxx': context_linux_x64_duk_gxx,

	'linux-x64-gcc-defsize-makeduk': context_linux_x64_gcc_defsize_makeduk,
	'linux-x64-gcc-defsize-fltoetc': context_linux_x64_gcc_defsize_fltoetc,
	'linux-x64-gcc-minsize-fltoetc': context_linux_x64_gcc_minsize_fltoetc,
	'linux-x86-gcc-minsize-fltoetc': context_linux_x86_gcc_minsize_fltoetc,
	'linux-x32-gcc-minsize-fltoetc': context_linux_x32_gcc_minsize_fltoetc,

	'linux-x64-cpp-exceptions': context_linux_x64_cpp_exceptions,

	'linux-x86-ajduk': context_linux_x86_ajduk,
	'linux-x86-ajduk-rombuild': context_linux_x86_ajduk_rombuild,

	# OS X: can currently share Linux handlers

	'osx-x64-qecmatest': context_linux_x64_qecmatest,
	'osx-x64-duk-clang': context_linux_x64_duk_clang,
	'osx-x64-duk-gxx': context_linux_x64_duk_gxx,
	'osx-x64-gcc-minsize-makeduk': context_linux_x64_gcc_defsize_makeduk
}

#
#  Main
#

def main():
	print('*** Running test %r on %s' % (context, datetime.datetime.utcnow().isoformat() + 'Z'))
	print('')
	print('repo_full_name: ' + repo_full_name)
	print('repo_clone_url: ' + repo_clone_url)
	print('commit_name: ' + commit_name)
	print('context: ' + context);

	if not os.path.isdir(temp_dir):
		raise Exception('missing or invalid temporary directory: %r' % temp_dir)

	m = re_reponame.match(repo_full_name)
	if m is None:
		raise Exception('invalid repo name: %r' % repo_full_name)

	if repo_full_name not in repo_whitelist:
		raise Exception('repo name is not whitelisted: %r' % repo_full_name)

	# Replace full repo forward slashes with platform separator
	repo_targz = apply(os.path.join, [ repo_snapshot_dir ] + (repo_full_name + '.tar.gz').split('/'))
	if repo_targz[0:len(repo_snapshot_dir)] != repo_snapshot_dir:
		raise Exception('internal error figuring out repo_targz: %r' % repo_targz)

	repo_dir = os.path.join(temp_dir, 'repo')
	os.chdir(temp_dir)
	os.mkdir(repo_dir)

	print('')
	print('*** Unpack repos and helpers')
	print('')

	os.chdir(repo_dir)
	unpack_targz(repo_targz)
	execute([ 'git', 'config', 'core.filemode', 'false' ])  # avoid perm issues on Windows

	for fn in [
		'alljoyn-72930212134129ae0464df93c526a6d110cb82f7.tar.gz',
		'ajtcl-cf47440914f31553a0064f3dabbbf337921ea357.tar.gz',
		'closure-20160317.tar.gz',
		'uglifyjs2-20160317.tar.gz',
		'runtests-node-modules-20160320.tar.gz'
	]:
		unpack_targz(os.path.join(repo_snapshot_dir, fn))

	execute([ 'git', 'clean', '-f' ])
	execute([ 'git', 'reset', '--hard' ])
	execute([ 'git', 'checkout', 'master' ])
	execute([ 'git', 'pull', '--rebase' ])
	execute([ 'git', 'clean', '-f' ])
	execute([ 'git', 'reset', '--hard' ])
	execute([ 'git', 'checkout', commit_name ])
	execute([ 'git', 'describe', '--always', '--dirty' ])

	fn = context_handlers.get(context)
	if fn is None:
		print('Unknown context %s, supported contexts:')
		for ctx in sorted(context_handlers.keys()):
			print('  ' + ctx)
		raise Exception('context unknown: ' + context)

	print('')
	print('*** Running test for context: ' + context)
	print('')

	success = fn()

	print('')
	print('*** Finished test for context: ' + context + ', success: ' + repr(success))
	print('')

	if output_description is not None:
		print('TESTRUNNER_DESCRIPTION: ' + output_description)

	if success == True:
		# Testcase successful
		print('Test succeeded')
		sys.exit(0)
	elif success == False:
		# Testcase failed, but no test script error (= don't rerun automatically)
		print('Test failed')
		sys.exit(1)
	else:
		raise Exception('context handler returned a non-boolean: %r' % success)

if __name__ == '__main__':
	try:
		main()
	except SystemExit:
		raise
	except:
		# Test script failed, automatic retry is useful
		print('')
		print('*** Test script failed')
		print('')
		traceback.print_exc()
		print('TESTRUNNER_DESCRIPTION: Test script error')
		sys.exit(2)
