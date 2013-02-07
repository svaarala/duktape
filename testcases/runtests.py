#!/usr/bin/python
#
#  Test case execution tool.
#
#  Executes one or multiple testcases with one or more Ecmascript engines,
#  outputting test case statistics.  Testcases are first executed using
#  multiple threads.  Once all threads have been finished, individual
#  result JSON documents are created for each testcase.  Finally, the JSON
#  documents are combined into a full test run JSON document, and results
#  are also summarized in a human readable form.
#
#  Test case format is described in doc/test-cases.txt.
#
#  JSON is used for test results because it will allow automatic build/test
#  scripts to process test results without grepping etc.  The current
#  structure is not very clean.
#

import os
import sys
import time
import textwrap
import re
import json
import md5
import atexit
import tempfile
import subprocess
import argparse
import threading

#-----------------------------------------------------------------------------

class Util:
	"Miscellaneous utilities."

	diff_cmd = None

	def get_autodelete_tempname():
		"Return a temporary filename with automatic deletion using atexit."

		name = tempfile.mktemp(suffix='-runtests')

		def _delete():
			try:
				os.unlink(name)
			except:
				sys.stderr.write('failed to delete: %s\n' % name)

		atexit.register(_delete)
		return name
	get_autodelete_tempname = staticmethod(get_autodelete_tempname)

	def which_command(cmd):
		"Resolve a plain command name to an absolute path using 'which'."

		name = Util.get_autodelete_tempname()
		f = None

		try:
			f = open(name, 'wb')
			rc = subprocess.call(['which', cmd], stdout=f, stderr=subprocess.PIPE)
			f.close()
			f = None

			f = open(name, 'rb')
			val = f.read().strip()
			f.close()
			f = None

			if not os.path.exists(val):
				# partial check only
				raise Exception('cannot find command %r' % cmd)
			return val
		finally:
			if f is not None:
				f.close()
	which_command = staticmethod(which_command)

	def diff_text(text1, text2):
		"Get a unified diff between two text values."

		MAX_STDOUT_OUTPUT = 10 * 1024 * 1024

		fname1 = Util.get_autodelete_tempname()
		fname2 = Util.get_autodelete_tempname()

		try:
			f = open(fname1, 'wb')
			f.write(text1)
			f.close()
		finally:
			if f is not None:
				f.close()

		try:
			f = open(fname2, 'wb')
			f.write(text2)
			f.close()
		finally:
			if f is not None:
				f.close()

		cmd_path = Util.diff_cmd
		if cmd_path is None:
			raise Exception('internal error: diff_cmd not set')
		cmd = "%s -u '%s' '%s'" % (cmd_path, fname1, fname2)
		#sys.stderr.write('executing: %s\n' % cmd)

		# XXX: use subprocess instead
		f = os.popen(cmd, 'r')
		stdout = f.read(MAX_STDOUT_OUTPUT)
		rc = f.close()
		# ignore rc (it is not zero)

		return stdout
	diff_text = staticmethod(diff_text)

Util.diff_cmd = Util.which_command('diff')

#-----------------------------------------------------------------------------

class Engine:
	"Configuration for a single Ecmascript engine."

	header = None
	footer = None
	name = None
	cmd_path = None

	def __init__(self, header='', footer='', name=None, cmd_path=None):
		self.header = header
		self.footer = footer
		self.name = name
		self.cmd_path = cmd_path

#-----------------------------------------------------------------------------

class TestCase:
	"Parses a single test case file."

	name = None
	filename = None
	testcase = None    # test case file contents
	metadata = None
	expected = None

	re_testname = re.compile(r'^(.*?)\.js$')

	def __init__(self, filename):
		f = None
		try:
			f = open(filename, 'rb')
			self.testcase = f.read()
			self.parse(self.testcase)
			m = self.re_testname.match(os.path.basename(filename))

			self.filename = filename
			self.name = m.group(1)
		finally:
			if f is not None:
				f.close()

	def parse(self, testcase):
		curridx = 0

		res = []
		metadata = None

		while True:
			try:
				idx1 = testcase.index('/*===', curridx)
				idx2 = testcase.index('===*/', curridx)
				tmp = testcase[idx1 + 5:idx2]
				curridx = idx2 + 5
			except ValueError:
				break

			part = []
			for i,line in enumerate(tmp.split('\n')):
				# strip first line if empty
				if line.strip() == '' and i == 0:
					continue	
				part.append(line)
			res.append('\n'.join(part))

		# Metadata is parsed as a JSON object from the first block delimited
		# with /*--- {...} ---*/
		try:
			idx1 = testcase.index('/*---', 0)
			idx2 = testcase.index('---*/', 0)
			tmp = testcase[idx1 + 5:idx2]
			metadata = json.loads(tmp)
		except ValueError:
			pass

		if metadata is None:
			metadata = {}

		self.expected = ''.join(res)
		self.metadata = metadata

	def generate_cooked_testfile(self, header, footer):
		name = Util.get_autodelete_tempname()

		f = None
		try:
			f = open(name, 'wb')
			f.write(header.encode('utf-8'))
			f.write(self.testcase)
			f.write(footer.encode('utf-8'))
		finally:
			if f is not None:
				f.close()
				f = None

		return name

#-----------------------------------------------------------------------------

class TestCaseRunner:
	"Executes a single test case with a single engine."

	cmdline = None
	stdout = None
	stderr = None
	rc = None
	time = None
	match_expected = None
	stdout_diff = None
	stdout_diff_lines = None

	def __init__(self):
		pass

	# FIXME: add valgrind with various checks, esp. memory leaks

	def run(self, testcase, engine):
		# 'timelimit' limits the maximum runtime of a test in a crude sort
		# of way.  Stdout read maximum size limits maximum output size,
		# also in a very crude way.  These are protections against runaway
		# loops and such.  The limits are rather large to allow testing to
		# complete even with slow devices (like Raspberry Pi).

		cooked = testcase.generate_cooked_testfile(engine.header, engine.footer)

		cmdname = engine.cmd_path
		killtime, warntime = 120, 90
		if testcase.metadata.has_key('slow') and testcase.metadata['slow']:
			killtime, warntime = 1200, 900

		cmdlist = []
		cmdlist += [ 'timelimit', '-T', str(killtime), '-t', str(warntime) ]
		#cmdlist += [ 'valgrind', '--tool=memcheck', '--xml=yes', '--xml-file=/tmp/vg.xml' ]
		cmdlist += [ cmdname, cooked ]

		cmdline = ' '.join(cmdlist)
		self.cmdline = cmdline

		f_stdout = None
		f_stderr = None
		n_stdout = Util.get_autodelete_tempname()
		n_stderr = Util.get_autodelete_tempname()
		rc = None

		try: 
			f_stdout = open(n_stdout, 'wb')
			f_stderr = open(n_stderr, 'wb')

			start_time = time.time()
			rc = subprocess.call(cmdlist, stdout=f_stdout, stderr=f_stderr)
		except:
			res_object['exception'] = True
		finally:
			if f_stdout is not None:
				f_stdout.flush()
				f_stdout.close()
				f_stdout = None
			if f_stderr is not None:
				f_stderr.flush()
				f_stderr.close()
				f_stderr = None
		end_time = time.time()

		self.stdout = None
		self.stderr = None

		f = None
		try:
			f = open(n_stdout, 'rb')
			self.stdout = f.read()
		finally:
			if f is not None:
				f.close()
				f = None

		f = None
		try:
			f = open(n_stderr, 'rb')
			self.stderr = f.read()
		finally:
			if f is not None:
				f.close()
				f = None

		if rc is None:
			rc = 0

		self.rc = rc
		self.time = end_time - start_time

		if self.stdout is None:
			self.match_expected = False
			self.stdout_diff = None
			self.stdout_diff_lines = None
		else:
			if self.stdout != testcase.expected:
				self.match_expected = False
				self.stdout_diff = Util.diff_text(testcase.expected, self.stdout)
				self.stdout_diff_lines = len(self.stdout_diff.split('\n')) - 1  # -1 for trailing newline
			else:
				self.match_expected = True
				self.stdout_diff = None
				self.stdout_diff_lines = None

#-----------------------------------------------------------------------------

class TestCaseTool:
	"Parses command line options, executes a set of tests, and writes a test log."

	args = None
	test_log_file = None
	num_threads = None

	engines = None
	testcases = None
	results = None
	lock = None
	worklist = None
	ongoing = None

	nodejs_header = textwrap.dedent("""\
		/* nodejs header begin */
		function print() {
		    // Note: Array.prototype.map() is required to support 'this' binding
		    // other than an array (arguments object here).
		    var tmp = Array.prototype.map.call(arguments, function (x) { return "" + x; });
		    var msg = tmp.join(' ');
		    console.log(msg);
		}
		//var print = console.log;
		/* nodejs header end */
	""")

	def __init__(self):
		self.testcases = {}   # testcase.name -> testcase
		self.results = {}     # testcase.name -> { result }
		self.lock = threading.Lock()
		self.worklist = []    # { 'testcase': testcase, 'engine': engine }
		self.ongoing = {}     # testcase.name + '-' + engine.name -> True

	def log(self, msg):
		if self.test_log_file is None:
			return
		self.test_log_file.write(msg)

	def init_engines(self):
		self.engines = []

		engines = self.engines
		args = self.args

		if args.run_duk:
			cmd_path = args.cmd_duk
			if cmd_path is None:
				cmd_path = Util.which_command('duk')
			eng = Engine(header='', footer='', name='duk', cmd_path=cmd_path)
			engines.append(eng)

		if args.run_nodejs:
			cmd_path = args.cmd_nodejs
			if cmd_path is None:
				cmd_path = Util.which_command('node')
			eng = Engine(header=self.nodejs_header, footer='', name='nodejs', cmd_path=cmd_path)
			engines.append(eng)

		if args.run_rhino:
			cmd_path = args.cmd_rhino
			if cmd_path is None:
				cmd_path = Util.which_command('rhino')
			eng = Engine(header='', footer='', name='rhino', cmd_path=cmd_path)
			engines.append(eng)

		if args.run_smjs:
			cmd_path = args.cmd_smjs
			if cmd_path is None:
				cmd_path = Util.which_command('smjs')
			eng = Engine(header='', footer='', name='smjs', cmd_path=cmd_path)
			engines.append(eng)

	def run_work_item(self):
		try:
			self.lock.acquire()
			if len(self.worklist) == 0:
				return False
			item = self.worklist.pop()

			testcase = item['testcase']
			engine = item['engine']
			ongoing_key = testcase.name + '-' + engine.name
			self.ongoing[ongoing_key] = True
		finally:
			self.lock.release()

		tr = TestCaseRunner()
		tr.run(testcase, engine)

		try:
			self.lock.acquire()

			del self.ongoing[ongoing_key]

			if not self.results.has_key(testcase.name):
				self.results[testcase.name] = {}
			if not self.results[testcase.name].has_key(engine.name):
				self.results[testcase.name][engine.name] = {}
			self.results[testcase.name][engine.name] = tr
	
			return True
		finally:
			self.lock.release()

	def generate_testcase_result_doc(self, testcase):
		"Generate a result object ready for programmatic use."

		txt = '*** TEST CASE: %s ***' % testcase.name
		msg = '\n' + ('*' * len(txt)) + '\n' + txt + '\n' + ('*' * len(txt)) + '\n'
		self.log(msg)

		ret = {}

		results = self.results[testcase.name]
		enginenames = results.keys()
		enginenames.sort()

		ret['file'] = testcase.filename
		ret['name'] = testcase.name
		ret['expected'] = {
			'len': len(testcase.expected),
			'md5': md5.md5(testcase.expected).hexdigest()
		}
		ret['metadata'] = testcase.metadata

		actual = {}
		for eng in enginenames:
			key = 'res_' + eng
			actual[eng] = results[eng].stdout
			ret[key] = {
				'stdout': {
					'len': len(results[eng].stdout),
					'md5': md5.md5(results[eng].stdout).hexdigest()
				},
				'stderr': {
					'len': len(results[eng].stderr),
					'md5': md5.md5(results[eng].stderr).hexdigest()
				},
				'cmdline': results[eng].cmdline,
				'time': results[eng].time,
				'rc': results[eng].rc
			}
			if results[eng].stdout_diff is not None:
				ret[key]['diff'] = results[eng].stdout_diff
				ret[key]['diff_lines'] = results[eng].stdout_diff_lines

			msg = '\n'
			msg += 'Return code from test %s with engine %s -> %r\n' % (testcase.name, eng, results[eng].rc)
			msg += '=== Stdout from test %s with engine %s ===\n' % (testcase.name, eng)
			msg += results[eng].stdout
			if len(results[eng].stdout) > 0 and results[eng].stdout[-1] != '\n':
				msg += '\n'
			msg += '=== Stdout from test %s with engine %s finished ===\n' % (testcase.name, eng)
			msg += '=== Stderr from test %s with engine %s ===\n' % (testcase.name, eng)
			msg += results[eng].stderr
			if len(results[eng].stderr) > 0 and results[eng].stderr[-1] != '\n':
				msg += '\n'
			msg += '=== Stderr from test %s with engine %s finished ===\n' % (testcase.name, eng)
			msg += '\n'
			if results[eng].stdout_diff is not None:
				msg += '=== Diff from test %s, expected -> %s ===\n' % (testcase.name, eng)
				msg += results[eng].stdout_diff
				if len(results[eng].stdout_diff) > 0 and results[eng].stdout_diff[-1] != '\n':
					msg += '\n'
				msg += '=== Diff from test %s, expected -> %s finished ===\n' % (testcase.name, eng)
				msg += '\n'

			self.log(msg)

		for eng in enginenames:
			if eng == 'duk':
				continue
			if 'duk' not in enginenames:
				continue
			if not actual.has_key('duk') or not actual.has_key(eng):
				continue
			if actual['duk'] == actual[eng]:
				continue

			diff_key = 'diff_' + eng
			diff = Util.diff_text(actual['duk'], actual[eng])
			ret['res_duk'][diff_key] = {
				'diff': diff,
				'lines': len(diff.split('\n')) - 1
			}

			msg = '\n'
			msg += '=== diff -u duk -> %s for test %s ===\n' % (eng, testcase.name)
			msg += diff
			msg += '=== end diff duk -> %s for test %s ===\n' % (eng, testcase.name)
			msg += '\n'
			self.log(msg)

		if testcase.metadata.has_key('skip') and testcase.metadata['skip']:
			ret['result'] = 'skip'
		elif actual.has_key('duk'):
			if actual['duk'] == testcase.expected:
				ret['result'] = 'success'

				diff_strs = []
				for eng in enginenames:
					if eng == 'duk':
						continue
					if testcase.metadata.has_key('custom') and testcase.metadata['custom']:
						# no point in comparing (test should be outside but would increase indent)
						continue
					diff_key = 'diff_' + eng
					if ret['res_duk'].has_key(diff_key):
						diff_strs.append('%s diff %d lines' % (eng, ret['res_duk'][diff_key]['lines']))

				if len(diff_strs) > 0:
					ret['notes'] = ', '.join(diff_strs)
			else:
				ret['result'] = 'failure'
		else:
			ret['result'] = 'n/a'

		return ret

	def run_tests(self):
		testcasenames = self.testcases.keys()
		testcasenames.sort()

		msg = '*** Starting a test run on %d tests... ***\n\n' % len(testcasenames)
		self.log(msg)

		slow_cases = []
		other_cases = []
		for testcasename in testcasenames:
			testcase = self.testcases[testcasename]
			for engine in self.engines:
				if testcase.metadata.has_key('slow') and testcase.metadata['slow']:
					slow_cases.append({ 'testcase': testcase, 'engine': engine })
				else:
					other_cases.append({'testcase': testcase, 'engine': engine })
		self.worklist = slow_cases + other_cases

		worklist_orig_len = len(self.worklist)

		def thread_entry():
			while self.run_work_item():
				pass

		self.threads = []
		for i in xrange(self.num_threads):
			thread = threading.Thread(target=thread_entry)
			self.threads.append(thread)

		for thread in self.threads:
			thread.start()

		wait_list = []
		for thread in self.threads:
			wait_list.append(thread)

		while len(wait_list) > 0:
			thread = wait_list.pop()
			while thread.isAlive():
				try:
					self.lock.acquire()
					print('Running tests: %d ongoing, %d in work list, %d total items (%d threads)' % \
					      (len(self.ongoing.keys()), len(self.worklist), worklist_orig_len, self.num_threads))
					#if len(self.worklist) == 0:
					#	print(' --> ' + ' '.join(self.ongoing.keys()))
				finally:
					self.lock.release()

				thread.join(5)

	def print_summary(self, json_doc, report_notes):
		print '\n----------------------------------------------------------------------------\n'

		num_skipped = 0
		num_success = 0
		num_failure = 0
		num_other = 0

		keys = json_doc.keys()
		keys.sort()
		for k in keys:
			res = json_doc[k]
			tmp = []
			tmp.append(res['result'])
			if res.has_key('notes'):
				tmp.append(', ' + res['notes'])

			if res['result'] == 'success':
				num_success += 1
			elif res['result'] == 'failure':
				num_failure += 1
			elif res['result'] == 'skip':
				num_skipped += 1
			else:
				num_other += 1

			if res['result'] == 'success' and res.has_key('notes'):
				if report_notes:
					print '%50s: success; %s' % (res['name'], res['notes'])
			elif res['result'] == 'failure':
				if res.has_key('res_duk') and res['res_duk'].has_key('diff_lines'):
					print '%50s: failed; diff %d lines' % (res['name'], res['res_duk']['diff_lines'])
				else:
					print '%50s: failed' % res['name']

		print '\n----------------------------------------------------------------------------\n'

		print 'SUMMARY: %d success, %d failure, %d skipped' % (num_success, num_failure, num_skipped)

	def main(self):
		parser = argparse.ArgumentParser(description='Run one or more test cases.')
		parser.add_argument('tests', metavar='TESTCASE', type=str, nargs='*', help='test case list')
		parser.add_argument('--run-duk', dest='run_duk', action='store_true', default=False, help='run test with duk')
		parser.add_argument('--run-rhino', dest='run_rhino', action='store_true', default=False, help='run test with Rhino')
		parser.add_argument('--run-smjs', dest='run_smjs', action='store_true', default=False, help='run test with smjs')
		parser.add_argument('--run-nodejs', dest='run_nodejs',action='store_true', default=False, help='run test with NodeJS (V8)')
		parser.add_argument('--cmd-duk', dest='cmd_duk', action='store', default=None, help='explicit command path')
		parser.add_argument('--cmd-rhino', dest='cmd_rhino', action='store', default=None, help='explicit command path')
		parser.add_argument('--cmd-smjs', dest='cmd_smjs', action='store', default=None, help='explicit command path')
		parser.add_argument('--cmd-nodejs', dest='cmd_nodejs', action='store', default=None, help='explicit command path')
		parser.add_argument('--test-log', dest='test_log', type=str, action='store', default=None, help='test log file (- == stdout)')
		parser.add_argument('--test-all', dest='test_all', action='store_true', default=False, help='run all testcases in cwd')
		parser.add_argument('--num-threads', dest='num_threads', action='store', default="4", help='number of testcase threads')
		parser.add_argument('--report-notes', dest='report_notes', action='store_true', default=False, help='report notes (e.g. diff from other engines) for successful tests')

		args = parser.parse_args()
		self.args = args

		test_log_file = None
		test_log_stdout = False
		testfiles = []
		testcases = {}

		self.init_engines()
		
		if args.test_log is not None:
			if args.test_log == '-':
				test_log_file = sys.stdout
				test_log_stdout = True
			else:
				test_log_file = open(args.test_log, 'wb')

		if args.test_all:
			for i in os.listdir('.'):
				if os.path.splitext(i)[1] == '.js':
					testfiles.append(i)

			testfiles.sort()
		else:
			for i in args.tests:
				testfiles.append(i)

		for testfile in testfiles:
			testcase = TestCase(testfile)
			testcases[testcase.name] = testcase

		self.testcases = testcases
		self.test_log_file = test_log_file
		self.num_threads = int(args.num_threads)

		self.run_tests()

		json_doc = {}
		for testcasename in self.testcases.keys():
			testcase = self.testcases[testcasename]
			doc = self.generate_testcase_result_doc(testcase)
			json_doc[testcase.name] = doc
			self.log(json.dumps(doc) + '\n')
		#self.log(json.dumps(json_doc))

		self.print_summary(json_doc, args.report_notes)

		if test_log_file is not None and not test_log_stdout:
			test_log_file.close()
			test_log_file = None

		exitcode = 0
		exit(exitcode)

#-----------------------------------------------------------------------------

if __name__ == '__main__':
	t = TestCaseTool()
	t.main()

