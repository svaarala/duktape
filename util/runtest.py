#!/usr/bin/env python2
#
#  Prepare a single ECMAScript testcase for execution and (optionally) execute
#  it with Duktape or another engine.  Interpret testcase results against the
#  expect string and known issues.
#
#  Currently no support for API testcases which require compilation and
#  linking.
#

# XXX: encoding issues
# XXX: external minifier
# XXX: use strict support; perhaps via test metadata
# XXX: figure out command line options
# XXX: options to control diff printing, other summary data
# XXX: use logging or just prints? debug prints?
# XXX: nodejs comparison

import os
import sys
import traceback
import re
import optparse
import subprocess
from threading import Timer
import atexit
import shutil
import time
import datetime
import tempfile
import platform
import md5
import json
import yaml
import xml.etree.ElementTree as ET

#
#  Platform detection
#

windows = platform.system() == 'Windows'
cygwin = 'CYGWIN' in platform.system()
istty = sys.stdout.isatty()
use_colors = istty and not windows

#
#  Global variables, RegExp patterns, etc
#

# Parse a metadata block from source file.
re_meta = re.compile(r'/\*---\n^((.|\r|\n)*?)^---\*/', re.MULTILINE)

# Parse an expect block from source file.  Initial newline is excluded but
# newline on last expect line is included.
re_expect = re.compile(r'/\*===\n^((?:.|\r|\n)*?)^===\*/|//>(.*?)$', re.MULTILINE)

# Parse a known issue: either a plain YAML file, or a YAML file separated by
# a '---' line, followed by expected output for the known issue.
re_knownissue = re.compile(r'((?:.|\n)*)\n---\n((?:.|\n)*)|((?:.|\n)*)', re.MULTILINE)

# Parse an include line.
re_include = re.compile('^/\*@include\s+(.*?)\s*@\*/$', re.MULTILINE)

# Parse a single line comment.  Doesn't account for e.g. Regexps that may
# contain two successive slashes, so careful when using the built-in hacky
# minifier.
re_singlelinecomment = re.compile('//(.*?)$', re.MULTILINE)

# Tempdir for test running, shutil.rmtree()'d at exit.
tempdir = None

# Entry CWD and script path for relative resolution.
entry_cwd = None
script_path = None

# Optparse args and opts shamelessly via a shared global to avoid plumbing.
opts = {}
args = []

# Testcase filename, used for relative resolution.
testcase_filename = None

#
#  ECMAScript test framework injected into ECMAScript test cases.
#

# Init code to run which allows the testcase to run on multiple engines.
ECMASCRIPT_TEST_FRAMEWORK=r'''
(function initTestFramework() {
    var Test = {};
    var G = new Function('return this')();
    if (typeof G.Duktape === 'object' && G.Duktape !== null) {
        Test.isDuktape = true;
        Test.engine = 'duktape';
    } else if (typeof G.Packages === 'object' && G.Packages !== null && String(Packages) === '[JavaPackage ]') {
        Test.isRhino = true;
        Test.engine = 'rhino';
    } else if (typeof G.process === 'object' && G.process !== null && typeof G.process.version === 'string') {
        Test.isV8 = true;  // not exact, detects via Node.js
        Test.engine = 'v8';
    } else {
        Test.engine = 'unknown';
    }
    Object.defineProperty(G, '__engine__', { value: Test.engine });  // XXX: to be removed, runtests compatibility

    if (typeof G.print !== 'function') {
        if (G.process && G.process.stdout && typeof G.process.stdout.write === 'function') {
            G.print = function print() {
                process.stdout.write(Array.prototype.map.call(arguments, String).join(' ') + '\n');
            };
        } else if (G.console && typeof G.console.log === 'function') {
            G.print = function print() {
                console.log(Array.prototype.map.call(arguments, String).join(' '));
            };
        }
    }

    if (Test.engine === 'duktape' && typeof G.console === 'undefined') {
        G.console = {
            log: print
        };
    }
})();
'''

#
#  File I/O helpers.
#

# Read file.
def read_file(fn):
    with open(fn, 'rb') as f:
        return f.read()

# Write file.
def write_file(fn, data):
    assert(isinstance(data, str))
    with open(fn, 'wb') as f:
        f.write(data)

# Convert to Windows path if necessary, used when running commands from Cygwin.
def path_to_platform(path):
    if not cygwin: return path
    return subprocess.check_output([ 'cygpath', '-w', path ]).strip()

#
#  Text processing helpers.
#

# Apply ANSI coloring.
# http://stackoverflow.com/questions/287871/print-in-terminal-with-colors-using-python
def ansi_color(text, color):
    if use_colors:
        return '\x1b[' + color + 'm' + text + '\x1b[0m'
    return text
def green(text):
    return ansi_color(text, '1;32;40')
def red(text):
    return ansi_color(text, '1;37;41')
def blue(text):
    return ansi_color(text, '0;37;44')
def yellow(text):
    return ansi_color(text, '0;33;40')
def grey(text):
    return ansi_color(text, '6;37;40')

# Parse lines.  Split a text input into lines using LF as the line separator.
# Assume last line is terminated with a LF and ignore an "empty line" that
# follows it.
def parse_lines(data):
    lines = data.split('\n')
    if lines[-1] == '':
        lines.pop()
    else:
        print('WARNING: last line of input did not contain a LF')
    return lines

# Combine lines into text.  Last line has trailing LF.
def combine_lines(lines):
    return '\n'.join(lines) + '\n'

# Count lines in text input.
def count_lines(data):
    return len(parse_lines(data))

# Indent lines.
def indent_lines(lines, count):
    prepend = ' ' * count
    return map(lambda x: prepend + x, lines)

# Clip text to maximum line count and column count.
def clip_lines(lines, start_idx, end_idx, column_limit=None):
    def clipline(x):
        if column_limit is not None and len(x) > column_limit:
            return x[0:column_limit] + ' [... %d more chars]' % (len(x) - column_limit)
        return x
    res = [clipline(x) for x in lines[start_idx:end_idx]]
    if len(lines) > end_idx:
        res.append('[... %d more lines]' % (len(lines) - end_idx))
    return res

# Remove carriage returns.
def remove_cr(data):
    return data.replace('\r', '')

#
#  Valgrind result processing
#

def parse_massif_result(f, res):
    # Allocated bytes.
    re_heap_b = re.compile(r'^mem_heap_B=(\d+)$')

    # Allocation overhead.  Matters for desktop environments, for efficient
    # zero overhead pool allocators this is not usually a concern (the waste
    # in a pool allocator behaves very differently than a libc allocator).
    re_heap_extra_b = re.compile(r'^mem_heap_extra_B=(\d+)$')

    # Stacks.
    re_stacks_b = re.compile(r'^mem_stacks_B=(\d+)$')

    peak_heap = 0
    peak_stack = 0

    for line in f:
        line = line.strip()
        #print(line)
        m1 = re_heap_b.match(line)
        m2 = re_heap_extra_b.match(line)
        m3 = re_stacks_b.match(line)

        heap = None
        if m1 is not None:
            heap = int(m1.group(1))
        stack = None
        if m3 is not None:
            stack = int(m3.group(1))

        if heap is not None:
            peak_heap = max(peak_heap, heap)
        if stack is not None:
            peak_stack = max(peak_stack, stack)

    res['massif_peak_heap_bytes'] = peak_heap
    res['massif_peak_stack_bytes'] = peak_stack

def parse_memcheck_result(f, res):
    try:
        tree = ET.parse(f)
    except ET.ParseError:
        res['errors'].append('memcheck-parse-failed')
        return
    root = tree.getroot()
    if root.tag != 'valgrindoutput':
        raise Exception('invalid valgrind xml format')

    def parse_error(node):
        err = {}
        for child in node.findall('kind'):
            err['kind'] = child.text
        for child in node.findall('xwhat'):
            for child2 in child.findall('text'):
                err['text'] = child2.text

        res['errors'].append(err['kind'])
        # XXX: make res['errors'] structured rather than text list?
        # 'err' is now ignored.

    for child in root.findall('error'):
        parse_error(child)

#
#  Test execution and result interpretation helpers.
#

# Get a unified diff between 'a' and 'b'.
def get_diff(a, b):
    if a == b:
        return ''

    fn_a = os.path.abspath(os.path.join(os.path.join(tempdir, 'diff-a')))
    fn_b = os.path.abspath(os.path.join(os.path.join(tempdir, 'diff-b')))
    write_file(fn_a, a)
    write_file(fn_b, b)

    cmd = None
    try:
        if windows:
            cmd = [ 'fc', path_to_platform(fn_a), path_to_platform(fn_b) ]
        else:
            cmd = [ 'diff', '--text', '-u', fn_a, fn_b ]

        #print('Executing: %r' % cmd)
        proc = subprocess.Popen(cmd, stdin=None, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        ret = proc.communicate(input=None)
        # ignore proc.returncode: diff returns 0 if inputs are same, nonzero otherwise.
        if len(ret[1]) > 0:
            print('Unexpected diff/fc stderr output:')
            print(ret[1])
        return ret[0]
    except:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        print('Command execution failed for %r:\n%s' % (cmd, traceback.format_exc(exc_traceback)))
        return '*** Failed to diff ***'

# Find testcase.  Testcase can be given with full (absolute or relative) path
# or just as 'test-dev-mandel2-func.js' for convenience, in which case we'll
# try to find it relative to the script path.
def find_testcase(name):
    if os.path.isfile(name):
        return name
    for dirname in [
        os.path.join(script_path, '..', 'tests', 'ecmascript'),
        os.path.join(entry_cwd, 'tests', 'ecmascript'),
        os.path.join(script_path, '..', 'tests', 'api'),
        os.path.join(entry_cwd, 'tests', 'api')
    ]:
        abs_fn = os.path.abspath(os.path.join(dirname, name))
        #print('Find testcase, try: %r' % abs_fn)
        if os.path.isfile(abs_fn):
            return abs_fn
    raise Exception('cannot find testcase: %r' % name)

# Find duktape command for convenience if not given explicitly.
def find_duktape():
    for fn in [
        os.path.join('.', 'duk'),
        os.path.join('.', 'duk.exe'),
        os.path.join(script_path, '..', 'duk'),
        os.path.join(script_path, '..', 'duk.exe'),
    ]:
        abs_fn = os.path.abspath(fn)
        #print('Find duk command, try: %r' % abs_fn)
        if os.path.exists(abs_fn):
            return abs_fn
    raise Exception('failed to locate "duk" command')

# Parse metadata from testcase file.
def parse_metadata(data):
    res = {}
    for m in re_meta.finditer(data):
        assert(m is not None)
        doc = yaml.load(m.group(1))  # YAML also accepts JSON
        for k in doc.keys():
            res[k] = doc[k]
    return res

# Parse expect string from testcase file.
def parse_expected_result(data):
    res = []
    for m in re_expect.finditer(data):
        assert(m is not None)
        if m.group(1) is not None:
            res.append(m.group(1))
        elif m.group(2) is not None:
            res.append(m.group(2) + '\n')  # Single-line shorthand
        else:
            raise Exception('internal error')
    return ''.join(res)

# Read include file, automatic path lookup.
def read_include_file(filename):
    abs_fn = os.path.abspath(os.path.join(os.path.dirname(testcase_filename), filename))
    #print('Include: %r -> %r' % (filename, abs_fn))
    data = read_file(abs_fn)
    return '/* Included: %r -> %r */ ' % (filename, abs_fn) + data

# Minify ECMAScript code either using an external minifier or a simple built-in
# minifier which replaces single line comments with /* */ comments and then
# replaces newlines with space.  This works in most cases, but assumes that
# semicolons are used in the source and that RegExps don't contain '//'
# sequences (slashes can be escaped).  The result is always a one-liner.
def minify_ecmascript(data):
    if '\n' not in data:
        return data

    fn_in = os.path.abspath(os.path.join(tempdir, 'minify-input'))
    fn_out = os.path.abspath(os.path.join(tempdir, 'minify-output'))
    write_file(fn_in, data)

    res = None
    if opts.minify_closure is not None:
        rc = subprocess.call([ 'java', '-jar', path_to_platform(opts.minify_closure),
                               '--js_output_file', path_to_platform(fn_out), path_to_platform(fn_in) ])
        if rc != 0:
            raise Exception('closure minify failed')
        res = read_file(fn_out)
        res = res.replace('\n', ' ')  # for some reason closure sometimes outputs newlines
    elif opts.minify_uglifyjs is not None:
        rc = subprocess.call([ opts.minify_uglifyjs, '-o',
                               path_to_platform(fn_out), path_to_platform(fn_in) ])
        if rc != 0:
            raise Exception('uglifyjs minify failed')
        res = read_file(fn_out)
    elif opts.minify_uglifyjs2 is not None:
        rc = subprocess.call([ opts.minify_uglifyjs2, '-o',
                               path_to_platform(fn_out), path_to_platform(fn_in) ])
        if rc != 0:
            raise Exception('uglifyjs2 minify failed')
        res = read_file(fn_out)
    else:
        #print('Input is not minified, no minifier given, using built-in simple minifier')
        def repl_comment(m):
            return '/* ' + m.group(1) + '*/'
        res = re.sub(re_singlelinecomment, repl_comment, data)
        res = res.replace('\n', ' ')

    res = res.strip()
    assert('\n' not in res)
    return res

# Inject utilities and other testing support functionality as one-liners
# into the testcase.  Using a one-liner avoids disturbing line numbers in
# the testcase.  The support code has ideally been already minified, but
# if not, try to minify it.  If there's no minifier, simply assume semicolons
# have been used correctly and replace newlines with spaces.
def prepare_ecmascript_testcase(data, meta):
    # Process includes.
    def repl_include(m):
        incfile = read_include_file(m.group(1))
        return minify_ecmascript(incfile)
    data = re.sub(re_include, repl_include, data)

    # Inject shared engine prefix.
    data = minify_ecmascript(ECMASCRIPT_TEST_FRAMEWORK) + ' ' + data

    # If the testcase needs to run strict program level code, prepend a
    # 'use strict' declaration once all the other preparations are done.
    if meta.get('use_strict', False):
        data = "'use strict'; " + data

    # Manually enabled Promise hack.
    if False:
        data = data + '\n' + "if (typeof Promise === 'function' && typeof Promise.runQueue === 'function') { Promise.runQueue(); }"

    return data

# Similar preparation for API testcases.
def prepare_api_testcase(data):
    raise Exception('not implemented')

# Parse a known issue file.
def parse_known_issue(data):
    m = re_knownissue.match(data)
    if m is None:
        raise Exception('failed to parse known issue file')
    elif m.group(1) is not None and m.group(2) is not None:
        meta = yaml.load(m.group(1))
        meta['output'] = m.group(2)  # add expected (known issue, i.e. buggy) output as .output
    elif m.group(3) is not None:
        meta = yaml.load(m.group(3))
    else:
        raise Exception('failed to parse known issue file')
    return meta

# Find known issues directory.
def find_known_issues():
    for dirname in [
        os.path.join(os.path.dirname(testcase_filename), '..', 'knownissues'),
        os.path.join(script_path, '..', 'tests', 'knownissues'),
        os.path.join(entry_cwd, 'tests', 'knownissues')
    ]:
        #print('Find known issues, try: %r' % dirname)
        if os.path.isdir(dirname):
            return dirname
    raise Exception('failed to locate known issues')

# Check known issues against output data.
def check_known_issues(dirname, output):
    output_md5 = md5.md5(output).digest().encode('hex')

    files = sorted(os.listdir(dirname))
    for fn in files:
        abs_fn = os.path.abspath(os.path.join(dirname, fn))
        #print('Check known issue: %r' % abs_fn)
        try:
            meta = parse_known_issue(read_file(abs_fn))
        except:
            print('Failed to parse known issue: %r' % abs_fn)
            meta = {}
        if meta.get('output', None) == output:
            return meta
        elif meta.get('md5', '').lower() == output_md5:
            return meta
    return None

#
#  Testcase execution.
#

# Execute Ecmscript testcase with optional timeout and valgrind wrapping.
# http://stackoverflow.com/questions/1191374/using-module-subprocess-with-timeout
# For Cygwin the command name should use Unix path but argument paths
# should be Windows converted.
def execute_ecmascript_testcase(res, data, name, polyfills):
    test_fn = os.path.abspath(os.path.join(tempdir, name))
    write_file(test_fn, data)

    valgrind_output = None

    cmd = []
    try:
        start_time = time.time()
        try:
            if opts.valgrind:
                res['valgrind'] = True
                res['valgrind_tool'] = opts.valgrind_tool
                cmd += [ 'valgrind' ]
                cmd += [ '--tool=' + opts.valgrind_tool ]

                valgrind_output = os.path.abspath(os.path.join(tempdir, 'valgrind.out'))
                if opts.valgrind_tool == 'massif':
                    cmd += [ '--massif-out-file=' + path_to_platform(valgrind_output) ]
                    #cmd += [ '--peak-inaccuracy=0.0' ]
                    #cmd += [ '--stacks=yes' ]
                elif opts.valgrind_tool == 'memcheck':
                    cmd += [ '--xml=yes', '--xml-file=' + path_to_platform(valgrind_output) ]
                else:
                    raise Exception('invalid valgrind tool %r' % opts.valgrind_tool)
                cmd += [ path_to_platform(os.path.abspath(opts.duk)) ]
            else:
                cmd += [ os.path.abspath(opts.duk) ]
            for fn in polyfills:
                cmd += [ path_to_platform(os.path.abspath(fn)) ]
            cmd += [ path_to_platform(os.path.abspath(test_fn)) ]
            res['command'] = cmd

            #print('Executing: %r' % cmd)
            proc = subprocess.Popen(cmd, stdin=None, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=os.path.abspath(tempdir))

            timeout_sec = opts.timeout
            def kill_proc(p):
                print('Killing testcase process due to timeout (%d seconds)' % timeout_sec)
                res['timeout'] = True
                p.kill()
            timer = Timer(timeout_sec, kill_proc, [proc])

            try:
                timer.start()
                ret = proc.communicate(input=None)
            finally:
                timer.cancel()

            res['stdout'] = remove_cr(ret[0])
            res['stderr'] = remove_cr(ret[1])
            res['returncode'] = proc.returncode

            if opts.valgrind:
                res['valgrind_output'] = ret[1]
                res['stderr'] = ''  # otherwise interpreted as an error
                if valgrind_output is not None and os.path.exists(valgrind_output):
                    with open(valgrind_output, 'rb') as f:
                        res['valgrind_output'] += f.read()
                    with open(valgrind_output, 'rb') as f:
                        if opts.valgrind_tool == 'massif':
                            parse_massif_result(f, res)
                        elif opts.valgrind_tool == 'memcheck':
                            parse_memcheck_result(f, res)
                else:
                    res['errors'].append('no-valgrind-output')
        except:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            print('Command execution failed for %r:\n%s' % (cmd, traceback.format_exc(exc_traceback)))
            res['stdout'] = traceback.format_exc(exc_traceback)
            res['stderr'] = traceback.format_exc(exc_traceback)
            res['returncode'] = 1
    finally:
        end_time = time.time()
        res['duration'] = end_time - start_time

# Execute API testcase with optional timeout and valgrind wrapping.
def execute_api_testcase(data):
    raise Exception('unimplemented')

# Interpret test results against expected result and known issues.
# Return a JSON-compatible object providing test results.
def interpret_test_result(doc, expect):
    meta = doc['metadata']

    errors = doc['errors']

    known_meta = check_known_issues(opts.known_issues, doc['stdout'])

    success = True
    if doc['stdout'] != expect:
        errors.append('expect-mismatch')
        success = False
    if doc['returncode'] != 0:
        if meta.get('intended_uncaught', False):
            # Test case is intended to throw an uncaught error.  This is
            # necessary to test some errors that occur at the program level.
            pass
        else:
            errors.append('returncode-nonzero')
            success = False
    if doc['stderr'] != '':
        if meta.get('intended_uncaught', False):
            pass
        else:
            errors.append('stderr-nonempty')
            success = False
    if doc['timeout']:
        errors.append('exec-timeout')
    if known_meta is not None:
        errors.append('known-issue')
        success = False
        doc['knownissue'] = known_meta.get('summary', 'no summary')
        doc['knownissue_meta'] = known_meta
    if len(errors) > 0:
        success = False

    doc['success'] = success
    doc['errors'] = errors
    doc['diff_expect'] = get_diff(expect, doc['stdout'])

#
#  Human readable summary.
#

# Print testcase summary from result JSON object.
def print_summary(doc):
    meta = doc['metadata']

    def fmt_time(x):
        if x >= 60:
            return '%.1f m' % (float(x) / 60.0)
        else:
            return '%.1f s' % float(x)

    def fmt_size(x):
        if x < 1024 * 1024:
            return '%.2f k' % (float(x) / 1024.0)
        else:
            return '%.2f M' % (float(x) / (1024.0 * 1024.0))

    parts = []
    issues = []
    test_result = '???'
    test_name = doc['testcase_name'].ljust(50)
    print_diff = True  # print diff if it is nonzero

    test_time = fmt_time(doc['duration'])
    test_time = '[%s]' % (test_time.rjust(6))

    if doc['skipped']:
        test_result = 'SKIPPED'
    elif doc['success']:
        if doc['timeout']:
            test_result = yellow('TIMEOUT')
            print_diff = False
        else:
            test_result = green('SUCCESS')
    else:
        if doc['timeout']:
            test_result = yellow('TIMEOUT')
            print_diff = False
        elif doc['knownissue'] != '':
            test_result = blue('KNOWN  ')
            print_diff = False
        else:
            test_result = red('FAILURE')

        issues += [ '[%d diff lines]' % count_lines(doc['diff_expect']) ]
        if doc['knownissue'] != '':
            issues += [ '[known: ' + doc['knownissue'] + ']' ]

    if len(doc['errors']) > 0:
        issues += [ '[errors: ' + ','.join(doc['errors']) + ']' ]

    parts += [ test_result, test_name ]

    if doc['duration'] >= 60.0:
        parts += [ blue(test_time) ]
    elif doc['duration'] >= 5.0:
        parts += [ yellow(test_time) ]
    else:
        parts += [ test_time ]

    if doc.has_key('massif_peak_heap_bytes'):
        tmp = []
        tmp += [ '%s heap' % fmt_size(doc['massif_peak_heap_bytes']) ]
        #tmp += [ '%s stack' % fmt_size(doc['massif_peak_stack_bytes']) ]
        parts += [ '[%s]' % (', '.join(tmp).rjust(14)) ]

    if doc.has_key('valgrind_tool'):
        parts += [ grey('[%s]' % doc['valgrind_tool']) ]

    parts += issues

    print(' '.join(parts))

    if doc['stderr'] != '' and not meta.get('intended_uncaught', False):
        if True:
            print('- Test wrote to stderr:')
            stderr_lines = parse_lines(doc['stderr'])
            stderr_lines = clip_lines(stderr_lines, 0, opts.clip_lines, opts.clip_columns)
            stderr_lines = indent_lines(stderr_lines, 4)
            sys.stdout.write(combine_lines(stderr_lines))
        else:
            pass

    if doc['diff_expect'] != '' and print_diff:
        if True:
            print('- Diff to expected result:')
            skip = 2  # skip a few uninteresting diff lines by default
            if windows: skip = 0  # but not 'fc'
            diff_lines = parse_lines(doc['diff_expect'])
            diff_lines = clip_lines(diff_lines, skip, skip + opts.clip_lines, opts.clip_columns)
            diff_lines = indent_lines(diff_lines, 4)
            sys.stdout.write(combine_lines(diff_lines))
        else:
            pass

#
#  Main program.
#

def main():
    global tempdir, args, opts, entry_cwd, script_path, testcase_filename

    exitcode = 0

    # Get script path and current CWD for relative resolution.  Plumbed
    # through globals to minimize argument passing.
    entry_cwd = os.getcwd()
    script_path = sys.path[0]  # http://stackoverflow.com/questions/4934806/how-can-i-find-scripts-directory-with-python

    # Parse options.
    parser = optparse.OptionParser(
        usage='Usage: %prog [options] testcase',
        description='Prepare an ECMAScript or API testcase for execution and (optionally) execute the testcase, print a summary, and write a JSON result file for further user.  Testcase can be given using a full path or using just the test name in which case it is looked up from ../tests/ecmascript/ relative to the runtest.py script.'
    )
    parser.add_option('--known-issues', dest='known_issues', default=None, help='Path to known issues directory, default is autodetect')
    parser.add_option('--ignore-skip', dest='ignore_skip', default=False, action='store_true', help='Ignore skip=true in metadata')
    parser.add_option('--minify-uglifyjs2', dest='minify_uglifyjs2', default=None, help='Path to UglifyJS2 to be used for minifying')
    parser.add_option('--minify-uglifyjs', dest='minify_uglifyjs', default=None, help='Path to UglifyJS to be used for minifying')
    parser.add_option('--minify-closure', dest='minify_closure', default=None, help='Path to Closure compiler.jar to be used for minifying')
    parser.add_option('--duk', dest='duk', default=None, help='Path to "duk" command, default is autodetect')
    parser.add_option('--timeout', dest='timeout', type='int', default=15*60, help='Test execution timeout (seconds), default 15min')
    parser.add_option('--polyfill', dest='polyfills', default=[], action='append', help='Polyfill script(s) for duk')
    parser.add_option('--valgrind', dest='valgrind', action='store_true', default=False, help='Run test inside valgrind')
    parser.add_option('--valgrind-tool', dest='valgrind_tool', default=None, help='Valgrind tool to use (implies --valgrind)')
    parser.add_option('--memcheck', dest='memcheck', default=False, action='store_true', help='Shorthand for --valgrind-tool memcheck')
    parser.add_option('--massif', dest='massif', default=False, action='store_true', help='Shorthand for --valgrind-tool massif')
    parser.add_option('--prepare-only', dest='prepare_only', action='store_true', default=False, help='Only prepare a testcase without running it')
    parser.add_option('--clip-lines', dest='clip_lines', type='int', default=15, help='Number of lines for stderr/diff summaries')
    parser.add_option('--clip-columns', dest='clip_columns', type='int', default=160, help='Number of columns for stderr/diff summaries')
    parser.add_option('--output-prepared', dest='output_prepared', default=None, help='Filename for prepared testcase')
    parser.add_option('--output-result', dest='output_result', default=None, help='Filename for result JSON file')
    parser.add_option('--output-stdout', dest='output_stdout', default=None, help='Filename for writing verbatim test stdout')
    parser.add_option('--output-stderr', dest='output_stderr', default=None, help='Filename for writing verbatim test stderr')
    parser.add_option('--output-diff', dest='output_diff', default=None, help='Filename for writing testcase expect-to-actual diff')
    parser.add_option('--output-valgrind', dest='output_valgrind', default=None, help='Filename for writing valgrind output')
    (opts, args) = parser.parse_args()

    # Some option defaulting.
    if opts.duk is None:
        opts.duk = find_duktape()
        #print('Autodetect "duk" command: %r' % opts.duk)
    testcase_filename = find_testcase(args[0])
    if opts.known_issues is None:
        opts.known_issues = find_known_issues()
        #print('Autodetect known issues directory: %r' % opts.known_issues)
    if opts.memcheck:
        opts.valgrind = True
        opts.valgrind_tool = 'memcheck'
    if opts.massif:
        opts.valgrind = True
        opts.valgrind_tool = 'massif'
    if opts.valgrind_tool is not None:
        opts.valgrind = True
    if opts.valgrind and opts.valgrind_tool is None:
        opts.valgrind_tool = 'memcheck'

    # Create a temporary directory for anything test related, automatic
    # atexit deletion.  Plumbed through globals to minimize argument passing.
    tempdir = tempfile.mkdtemp(prefix='tmp-duk-runtest-')
    atexit.register(shutil.rmtree, tempdir)
    #print('Using temporary directory: %r' % tempdir)

    # Read testcase, scan metadata and expected result.
    data = remove_cr(open(testcase_filename, 'rb').read())
    name = os.path.basename(testcase_filename)
    meta = parse_metadata(data)
    expect = parse_expected_result(data)

    # Prepare runnable testcase by injecting an ECMAScript test framework
    # and processing @include lines.
    data = prepare_ecmascript_testcase(data, meta)
    if opts.output_prepared is not None:
        write_file(opts.output_prepared, data)
        print('Wrote prepared testcase to: %r' % opts.output_prepared)

    # Initialize result object, filling fields with defaults so that calling
    # code can (at least mostly) rely on all fields being present.
    res = {}
    res['testcase_file'] = os.path.abspath(testcase_filename)
    res['testcase_name'] = name
    res['expect'] = expect
    res['metadata'] = meta
    res['skipped'] = False
    res['success'] = True
    res['errors'] = []
    res['diff_expect'] = ''
    res['knownissue'] = ''
    res['knownissue_meta'] = None
    res['skipped'] = True
    res['command'] = []
    res['valgrind'] = False
    res['valgrind_output'] = ''
    res['stdout'] = ''
    res['stderr'] = ''
    res['returncode'] = 0
    res['timeout'] = False
    res['duration'] = 0

    # Execute testcase unless requested to just prepare the testcase.
    # Execution result is a JSON-compatible object which can be written
    # out for further processing by the caller (e.g. to process results
    # of running multiple tests).  Print a maximally helpful, human readable
    # summary based on the same JSON-compatible result object.
    if not opts.prepare_only:
        if meta.get('skip', False) and not opts.ignore_skip:
            res['skipped'] = True
        else:
            res['skipped'] = False
            execute_ecmascript_testcase(res, data, name, opts.polyfills)
            interpret_test_result(res, expect)

        print_summary(res)

        if not res['success']:
            exitcode = 1
    else:
        pass

    # Write out requested output files: test result JSON, test raw
    # stdout/stderr, etc.
    if opts.output_result is not None:
        write_file(opts.output_result, json.dumps(res, indent=4, ensure_ascii=True).encode('utf-8') + '\n')
        print('Wrote test result JSON data to: %r' % opts.output_result)
    if opts.output_stdout is not None:
        write_file(opts.output_stdout, res['stdout'])
        print('Wrote test stdout to: %r' % opts.output_stdout)
    if opts.output_stderr is not None:
        write_file(opts.output_stderr, res['stderr'])
        print('Wrote test stderr to: %r' % opts.output_stderr)
    if opts.output_diff is not None:
        write_file(opts.output_diff, res['diff_expect'])
        print('Wrote test expect diff to: %r' % opts.output_diff)
    if opts.output_valgrind is not None:
        write_file(opts.output_valgrind, res['valgrind_output'])
        print('Wrote test valgrind output to: %r' % opts.output_valgrind)

    # Exit with 0 if test was considered success, non-zero otherwise.
    sys.exit(exitcode)

if __name__ == '__main__':
    main()
