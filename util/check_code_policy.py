#!/usr/bin/env python2
#
#  Check various source code policy rules and issue warnings for offenders.
#
#  Usage:
#
#    $ python check_code_policy.py src-input/*.c
#

import os
import sys
import re
import optparse

class Problem:
    filename = None
    linenumber = None
    line = None
    reason = None

    def __init__(self, filename, linenumber, line, reason):
        self.filename = filename
        self.linenumber = linenumber
        self.line = line
        self.reason = reason

re_debuglog_callsite = re.compile(r'^.*?(DUK_D+PRINT).*?$')
re_trailing_ws = re.compile(r'^.*?\s$')
re_only_ws = re.compile(r'^\s*$')
re_nonleading_tab = re.compile(r'^.*?[^\t]\t.*?$')  # tabs are only used for indent
re_identifier = re.compile(r'[A-Za-z0-9_]+')
re_nonascii = re.compile(r'^.*?[\x80-\xff].*?$')
re_func_decl_or_def = re.compile(r'^(\w+)\s+(?:\w+\s+)*(\w+)\(.*?.*?$')  # may not finish on same line
re_cpp_comment = re.compile(r'^.*?//.*?$')
re_longlong_constant_dec = re.compile(r'[0-9]+(LL|ULL)')
re_longlong_constant_hex = re.compile(r'0x[0-9a-fA-F]+(LL|ULL)')
re_float_compare = re.compile(r'==\s*\d*\.\d+')

fixmeString = 'FIX' + 'ME'  # avoid triggering a code policy check warning :)

# These identifiers are wrapped in duk_config.h, and should only be used
# through the wrappers elsewhere.
rejected_plain_identifiers_list = [
    # math classification
    'fpclassify',
    'signbit',
    'isfinite',
    'isnan',
    'isinf',
    'FP_NAN',
    'FP_INFINITE',
    'FP_ZERO',
    'FP_SUBNORMAL',
    'FP_NORMAL',

    # math functions
    'fabs',
    'fmin',
    'fmax',
    'floor',
    'ceil',
    'fmod',
    'pow',
    'acos',
    'asin',
    'atan',
    'atan2',
    'sin',
    'cos',
    'tan',
    'exp',
    'log',
    'sqrt',
    'cbrt',

    # memory functions
    'malloc',
    'realloc',
    'calloc',
    'free',
    'memcpy',
    'memmove',
    'memcmp',
    'memset',

    # string functions
    'strlen',
    'strcmp',
    'strncmp',
    'printf',
    'fprintf',
    'sprintf',
    '_snprintf',
    'snprintf',
    'vsprintf',
    '_vsnprintf',
    'vsnprintf',
    'sscanf',
    'vsscanf',

    # streams
    'stdout',
    'stderr',
    'stdin',

    # file ops
    'fopen',
    'fclose',
    'fread',
    'fwrite',
    'fseek',
    'ftell',
    'fflush',
    'fputc',

    # misc
    'abort',
    'exit',
    'setjmp',
    'longjmp',

    # variable/argument names which have shadowing issues with platform headers
    # see e.g. https://github.com/svaarala/duktape/pull/810
    'index',
    'rindex',

    # for consistency avoid these too, use obj_idx rather than obj_index, etc
    'obj_index',
    'from_index',
    'to_index',
    'arr_index',
    'uindex',

    # avoid in internals, use duk_hthread exclusively (except in duktape.h.in)
    'duk_context',

    # avoid in internal, except for duk_util_memory.c
    'DUK_MEMCPY',
    'DUK_MEMMOVE',
    'DUK_MEMCMP',
    'DUK_MEMSET',
    'DUK_MEMZERO'
]
rejected_plain_identifiers = {}
for id in rejected_plain_identifiers_list:
    rejected_plain_identifiers[id] = True

debuglog_wrappers = {
    'DUK_DPRINT': 'DUK_D',
    'DUK_DDPRINT': 'DUK_DD',
    'DUK_DDDPRINT': 'DUK_DDD'
}

allowed_visibility_macros = [
    'DUK_EXTERNAL_DECL',
    'DUK_EXTERNAL',
    'DUK_INTERNAL_DECL',
    'DUK_INTERNAL',
    'DUK_LOCAL_DECL',
    'DUK_LOCAL'
]

problems = []

re_repl_c_comments = re.compile(r'/\*.*?\*/', re.DOTALL)
re_repl_cpp_comments = re.compile(r'//.*?\n', re.DOTALL)
re_repl_string_literals_dquot = re.compile(r'''\"(?:\\\"|[^\"])*\"''')
re_repl_string_literals_squot = re.compile(r'''\'(?:\\\'|[^\'])*\'''')
re_repl_expect_strings = re.compile(r'/\*===.*?===*?\*/', re.DOTALL)
re_not_newline = re.compile(r'[^\n]+', re.DOTALL)

def repl_c(m):
    tmp = re.sub(re_not_newline, '', m.group(0))
    if tmp == '':
        tmp = ' '  # avoid /**/
    return '/*' + tmp + '*/'
def repl_cpp(m):
    return '// removed\n'
def repl_dquot(m):
    return '"' + ('.' * (len(m.group(0)) - 2)) + '"'
def repl_squot(m):
    return "'" + ('.' * (len(m.group(0)) - 2)) + "'"

def removeLiterals(data):
    data = re.sub(re_repl_string_literals_dquot, repl_dquot, data)
    data = re.sub(re_repl_string_literals_squot, repl_squot, data)
    return data

def removeCCommentsAndLiterals(data):
    data = re.sub(re_repl_c_comments, repl_c, data)
    data = re.sub(re_repl_string_literals_dquot, repl_dquot, data)
    data = re.sub(re_repl_string_literals_squot, repl_squot, data)
    return data

def removeAnyCommentsAndLiterals(data):
    data = re.sub(re_repl_c_comments, repl_c, data)
    data = re.sub(re_repl_cpp_comments, repl_cpp, data)
    data = re.sub(re_repl_string_literals_dquot, repl_dquot, data)
    data = re.sub(re_repl_string_literals_squot, repl_squot, data)
    return data

def removeExpectStrings(data):
    def repl(m):
        tmp = re.sub(re_not_newline, '', m.group(0))
        if tmp == '':
            tmp = ' '  # avoid /*======*/
        return '/*===' + tmp + '===*/'

    data = re.sub(re_repl_expect_strings, repl, data)
    return data

def checkDebugLogCalls(lines, idx, filename):
    # Allowed debug log forms:
    #
    #     DUK_D(DUK_DPRINT(...))
    #     DUK_DD(DUK_DDPRINT(...))
    #     DUK_DDD(DUK_DDDPRINT(...))
    #
    # The calls may span multiple lines, but the wrapper (DUK_D)
    # and the log macro (DUK_DPRINT) must be on the same line.

    line = lines[idx]
    if 'DPRINT' not in line:
        return

    m = re_debuglog_callsite.match(line)
    if m is None:
        return

    log_macro = m.group(1)
    log_wrapper = debuglog_wrappers[log_macro]
    if log_wrapper + '(' in line:
        return

    # exclude '#define DUK_DPRINT...' macros in duk_debug.h
    if len(line) >= 1 and line[0] == '#':
        return

    # exclude a few comment lines in duk_debug.h
    if len(line) >= 3 and line[0:3] == ' * ':
        return

    raise Exception('invalid debug log call form')

def checkTrailingWhitespace(lines, idx, filename):
    line = lines[idx]
    if len(line) > 0 and line[-1] == '\n':
        line = line[:-1]

    m = re_trailing_ws.match(line)
    if m is None:
        return

    raise Exception('trailing whitespace')

def checkCarriageReturns(lines, idx, filename):
    line = lines[idx]
    if not '\x0d' in line:
        return

    raise Exception('carriage return')

def checkMixedIndent(lines, idx, filename):
    line = lines[idx]
    if not '\x20\x09' in line:
        return

    # Mixed tab/space are only allowed after non-whitespace characters
    idx = line.index('\x20\x09')
    tmp = line[0:idx]
    m = re_only_ws.match(tmp)
    if m is None:
        return

    raise Exception('mixed space/tab indent (idx %d)' % idx)

def checkTabIndent(lines, idx, filename):
    line = lines[idx]
    if not '\x09' in line:
        return

    # Now just checks for presence of TAB characters which is fine for Python
    # code (which this check is used for).

    raise Exception('tab indent (idx %d)' % idx)

def checkNonLeadingTab(lines, idx, filename):
    line = lines[idx]
    m = re_nonleading_tab.match(line)
    if m is None:
        return

    raise Exception('non-leading tab (idx %d)' % idx)

def checkFixme(lines, idx, filename):
    line = lines[idx]
    if not fixmeString in line:
        return

    raise Exception(fixmeString + ' on line')

def checkIdentifiers(lines, idx, filename):
    line = lines[idx]
    # XXX: this now executes for every line which is pointless
    bn = os.path.basename(filename)
    excludePlain = (bn[0:5] == 'test-')

    for m in re.finditer(re_identifier, line):
        if rejected_plain_identifiers.has_key(m.group(0)):
            if m.group(0) in [ 'duk_context' ] and bn == 'duktape.h.in':
                continue  # duk_context allowed in public API header
            if m.group(0) in [ 'DUK_MEMCPY', 'DUK_MEMMOVE', 'DUK_MEMCMP', 'DUK_MEMSET', 'DUK_MEMZERO' ] and \
               bn in [ 'duk_util_memory.c', 'duk_util.h' ]:
                continue
            if not excludePlain:
                raise Exception('invalid identifier %r (perhaps plain)' % m.group(0))

def checkNonAscii(lines, idx, filename):
    line = lines[idx]
    m = re_nonascii.match(line)
    if m is None:
        return

    bn = os.path.basename(filename)
    if bn == 'test-lex-utf8.js':
        # this specific file is intentionally exempt
        pass
    else:
        raise Exception('non-ascii character')

def checkNoSymbolVisibility(lines, idx, filename):
    line = lines[idx]

    # Workaround for DUK_ALWAYS_INLINE preceding a declaration
    # (e.g. "DUK_ALWAYS_INLINE DUK_LOCAL ...")
    if line.startswith('DUK_ALWAYS_INLINE '):
        line = line[18:]

    m = re_func_decl_or_def.match(line)
    if m is None:
        return

    bn = os.path.basename(filename)
    if not ((bn[-2:] == '.c' or bn[-2:] == '.h' or bn[-5:] == '.h.in') and bn[0:5] != 'test-'):
        # Apply to only specific files in src-input/
        return

    if m.group(1) in allowed_visibility_macros and \
       not ((m.group(1) != 'DUK_LOCAL' and m.group(1) != 'DUK_LOCAL_DECL') and 'duk__' in m.group(2)) and \
       not ((m.group(1) == 'DUK_LOCAL' or m.group(1) == 'DUK_LOCAL_DECL') and 'duk__' not in m.group(2)):
        return

    # Previous line may contain the declaration (alone)
    if idx > 0 and lines[idx - 1].strip() in allowed_visibility_macros:
        return

    # Special exceptions
    # (None now)

    raise Exception('missing symbol visibility macro')

def checkIfdefIfndef(lines, idx, filename):
    line = lines[idx]

    if not line.startswith('#if'):
        return
    if line.startswith('#ifdef'):
        raise Exception('#ifdef found, #if defined is preferred')
    if line.startswith('#ifndef'):
        raise Exception('#ifndef found, #if !defined is preferred')

def checkLongLongConstants(lines, idx, filename):
    line = lines[idx]

    if not 'LL' in line:
        return
    for m in re.finditer(re_longlong_constant_hex, line):
        raise Exception('plain longlong constant, use DUK_U64_CONSTANT or DUK_I64_CONSTANT: ' + str(m.group(0)))
    for m in re.finditer(re_longlong_constant_dec, line):
        raise Exception('plain longlong constant, use DUK_U64_CONSTANT or DUK_I64_CONSTANT: ' + str(m.group(0)))

def checkCppComment(lines, idx, filename):
    line = lines[idx]
    m = re_cpp_comment.match(line)
    if m is None:
        return

    raise Exception('c++ comment')

def checkFloatCompare(lines, idx, filename):
    line = lines[idx]
    for m in re.finditer(re_float_compare, line):
        raise Exception('float/double compare, use duk_{double,float}_equals()')

def processFile(filename, checkersRaw, checkersNoCommentsOrLiterals, checkersNoCCommentsOrLiterals, checkersNoExpectStrings):
    f = open(filename, 'rb')
    dataRaw = f.read()
    f.close()

    dataNoCommentsOrLiterals = removeAnyCommentsAndLiterals(dataRaw)   # no C/javascript comments, literals removed
    dataNoCCommentsOrLiterals = removeCCommentsAndLiterals(dataRaw)    # no C comments, literals removed
    dataNoExpectStrings = removeExpectStrings(dataRaw)                 # no testcase expect strings

    linesRaw = dataRaw.split('\n')
    linesNoCommentsOrLiterals = dataNoCommentsOrLiterals.split('\n')
    linesNoCCommentsOrLiterals = dataNoCCommentsOrLiterals.split('\n')
    linesNoExpectStrings = dataNoExpectStrings.split('\n')

    def f(lines, checkers):
        for linenumber in xrange(len(lines)):
            for fun in checkers:
                try:
                    fun(lines, linenumber, filename)  # linenumber is zero-based here
                except Exception as e:
                    problems.append(Problem(filename, linenumber + 1, lines[linenumber], str(e)))

    f(linesRaw, checkersRaw)
    f(linesNoCommentsOrLiterals, checkersNoCommentsOrLiterals)
    f(linesNoCCommentsOrLiterals, checkersNoCCommentsOrLiterals)
    f(linesNoExpectStrings, checkersNoExpectStrings)

    # Last line should have a newline, and there should not be an empty line.
    # The 'split' result will have one empty string as its last item in the
    # expected case.  For a single line file there will be two split results
    # (the line itself, and an empty string).

    if len(linesRaw) == 0 or \
       len(linesRaw) == 1 and linesRaw[-1] != '' or \
       len(linesRaw) >= 2 and linesRaw[-1] != '' or \
       len(linesRaw) >= 2 and linesRaw[-1] == '' and linesRaw[-2] == '':
        problems.append(Problem(filename, len(linesRaw), '(no line)', 'No newline on last line or empty line at end of file'))

    # First line should not be empty (unless it's the only line, len(linesRaw)==2)
    if len(linesRaw) > 2 and linesRaw[0] == '':
        problems.append(Problem(filename, 1, '(no line)', 'First line is empty'))

def asciiOnly(x):
    return re.sub(r'[\x80-\xff]', '#', x)

def main():
    parser = optparse.OptionParser()
    parser.add_option('--dump-vim-commands', dest='dump_vim_commands', default=False, help='Dump oneline vim command')
    parser.add_option('--check-debug-log-calls', dest='check_debug_log_calls', action='store_true', default=False, help='Check debug log call consistency')
    parser.add_option('--check-carriage-returns', dest='check_carriage_returns', action='store_true', default=False, help='Check carriage returns')
    parser.add_option('--check-fixme', dest='check_fixme', action='store_true', default=False, help='Check ' + fixmeString + ' tags')
    parser.add_option('--check-non-ascii', dest='check_non_ascii', action='store_true', default=False, help='Check non-ASCII characters')
    parser.add_option('--check-no-symbol-visibility', dest='check_no_symbol_visibility', action='store_true', default=False, help='Check for missing symbol visibility macros')
    parser.add_option('--check-rejected-identifiers', dest='check_rejected_identifiers', action='store_true', default=False, help='Check for rejected identifiers like plain "printf()" calls')
    parser.add_option('--check-trailing-whitespace', dest='check_trailing_whitespace', action='store_true', default=False, help='Check for trailing whitespace')
    parser.add_option('--check-mixed-indent', dest='check_mixed_indent', action='store_true', default=False, help='Check for mixed indent (space and tabs)')
    parser.add_option('--check-tab-indent', dest='check_tab_indent', action='store_true', default=False, help='Check for tab indent')
    parser.add_option('--check-nonleading-tab', dest='check_nonleading_tab', action='store_true', default=False, help='Check for non-leading tab characters')
    parser.add_option('--check-cpp-comment', dest='check_cpp_comment', action='store_true', default=False, help='Check for c++ comments ("// ...")')
    parser.add_option('--check-float-compare', dest='check_float_compare', action='store_true', default=False, help='Check for floating point comparisons (== 1.23)')
    parser.add_option('--check-ifdef-ifndef', dest='check_ifdef_ifndef', action='store_true', default=False, help='Check for #ifdef and #ifndef (prefer #if defined and #if !defined)')
    parser.add_option('--check-longlong-constants', dest='check_longlong_constants', action='store_true', default=False, help='Check for plain 123LL or 123ULL constants')
    parser.add_option('--fail-on-errors', dest='fail_on_errors', action='store_true', default=False, help='Fail on errors (exit code != 0)')

    (opts, args) = parser.parse_args()

    checkersRaw = []
    if opts.check_debug_log_calls:
        checkersRaw.append(checkDebugLogCalls)
    if opts.check_carriage_returns:
        checkersRaw.append(checkCarriageReturns)
    if opts.check_fixme:
        checkersRaw.append(checkFixme)
    if opts.check_non_ascii:
        checkersRaw.append(checkNonAscii)
    if opts.check_no_symbol_visibility:
        checkersRaw.append(checkNoSymbolVisibility)
    if opts.check_ifdef_ifndef:
        checkersRaw.append(checkIfdefIfndef)
    if opts.check_longlong_constants:
        checkersRaw.append(checkLongLongConstants)

    checkersNoCCommentsOrLiterals = []
    if opts.check_cpp_comment:
        checkersNoCCommentsOrLiterals.append(checkCppComment)
    if opts.check_float_compare:
        checkersNoCCommentsOrLiterals.append(checkFloatCompare)

    checkersNoCommentsOrLiterals = []
    if opts.check_rejected_identifiers:
        checkersNoCommentsOrLiterals.append(checkIdentifiers)

    checkersNoExpectStrings = []
    if opts.check_trailing_whitespace:
        checkersNoExpectStrings.append(checkTrailingWhitespace)
    if opts.check_mixed_indent:
        checkersNoExpectStrings.append(checkMixedIndent)
    if opts.check_tab_indent:
        checkersNoExpectStrings.append(checkTabIndent)
    if opts.check_nonleading_tab:
        checkersNoExpectStrings.append(checkNonLeadingTab)

    for filename in args:
        processFile(filename, checkersRaw, checkersNoCommentsOrLiterals, checkersNoCCommentsOrLiterals, checkersNoExpectStrings)

    if len(problems) > 0:
        for i in problems:
            tmp = 'vim +' + str(i.linenumber)
            while len(tmp) < 10:
                tmp = tmp + ' '
            tmp += ' ' + str(i.filename) + ' : ' + str(i.reason)
            while len(tmp) < 80:
                tmp = tmp + ' '
            tmp += ' - ' + asciiOnly(i.line.strip())
            print(tmp)

        print '*** Total: %d problems' % len(problems)

        if opts.dump_vim_commands:
            cmds = []
            for i in problems:
                cmds.append('vim +' + str(i.linenumber) + ' "' + i.filename + '"')
            print ''
            print('; '.join(cmds))

        if opts.fail_on_errors:
            sys.exit(1)

    sys.exit(0)

if __name__ == '__main__':
    main()
