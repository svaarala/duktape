#!/usr/bin/python
#
#  Generate a list of built-in strings required by Duktape code, output
#  duk_strings.h (defines) and duk_strings.c (string data).  Raw string
#  data is also written to duk_strings.bin.
#
#  These strings may be required by execution and/or compilation, or
#  built-in code.  Strings are included here when it benefits footprint.
#  These strings are currently interned although strings needed only by
#  the compiler do not strictly need to be.  Strings need to be ordered
#  so that reserved words are in a certain range (with strict reserved
#  words grouped together).
#
#  XXX: better compression
#  XXX: avoid debug-related strings in release build (same applies to
#       other configuration dependent strings, like traceback data)
#  XXX: reserved word stridx's could be made to match token numbers
#       directly so that a duk_stridx2token[] would not be needed
#  XXX: improve per string metadata, and sort strings within constraints
#  XXX: some Duktape internal strings could just reuse existing strings

import os, sys
import optparse
import dukutil

# Prefix for defines

define_prefix = 'DUK_HEAP_STRIDX_'

#
#  String lists
#
#  Some strings may appear in multiple lists and even in multiple roles.
#

# XXX: currently the keywords are not recorded; use them later to organize
# strings more optimally

def mkstr(x,
          section_b=False,
          browser_like=False,
          custom=False,
          internal=False,
          reserved_word=False,
          future_reserved_word=False,
          future_reserved_word_strict=False,
          special_literal=False):

	"Create a string object."

	# A 0xff prefix (never part of valid UTF-8) is used for internal properties.
	# It is encoded as 0x00 in generated init data for technical reasons: it
	# keeps lookup table elements 7 bits instead of 8 bits.

	if internal:
		x = '\x00' + x

	return x

# Standard built-in object related strings
standard_builtin_string_list = [
	# internal class values
	mkstr("Object"),
	mkstr("Function"),
	mkstr("Array"),
	mkstr("String"),
	mkstr("Boolean"),
	mkstr("Number"),
	mkstr("Date"),
	mkstr("RegExp"),
	mkstr("Error"),
	mkstr("Math"),
	mkstr("JSON"),
	mkstr("Arguments"),

	# built-in object names
	mkstr("Object"),
	mkstr("Function"),
	mkstr("Array"),
	mkstr("String"),
	mkstr("Boolean"),
	mkstr("Number"),
	mkstr("Date"),
	mkstr("RegExp"),
	mkstr("Error"),
	mkstr("EvalError"),
	mkstr("RangeError"),
	mkstr("ReferenceError"),
	mkstr("SyntaxError"),
	mkstr("TypeError"),
	mkstr("URIError"),
	mkstr("Math"),
	mkstr("JSON"),

	# Global object
	mkstr("eval"),
	mkstr("parseInt"),
	mkstr("parseFloat"),
	mkstr("isNaN"),
	mkstr("isFinite"),
	mkstr("decodeURI"),
	mkstr("decodeURIComponent"),
	mkstr("encodeURI"),
	mkstr("encodeURIComponent"),
	mkstr("escape", section_b=True),
	mkstr("unescape", section_b=True),
	mkstr("print", browser_like=True),
	mkstr("alert", browser_like=True),

	# Object constructor
	mkstr("length"),
	mkstr("prototype"),
	mkstr("getPrototypeOf"),
	mkstr("getOwnPropertyDescriptor"),
	mkstr("getOwnPropertyNames"),
	mkstr("create"),
	mkstr("defineProperty"),
	mkstr("defineProperties"),
	mkstr("seal"),
	mkstr("freeze"),
	mkstr("preventExtensions"),
	mkstr("isSealed"),
	mkstr("isFrozen"),
	mkstr("isExtensible"),
	mkstr("keys"),

	# Property descriptors
	mkstr("value"),
	mkstr("writable"),
	mkstr("configurable"),
	mkstr("enumerable"),
	mkstr("get"),
	mkstr("set"),

	# Object prototype
	mkstr("constructor"),
	mkstr("toString"),
	mkstr("toLocaleString"),
	mkstr("valueOf"),
	mkstr("hasOwnProperty"),
	mkstr("isPrototypeOf"),
	mkstr("propertyIsEnumerable"),

	# Object instances
	# no special properties

	# Function constructor
	mkstr("length"),
	mkstr("prototype"),

	# Function prototype
	mkstr("constructor"),
	mkstr("toString"),
	mkstr("apply"),
	mkstr("call"),
	mkstr("bind"),

	# Function instances
	mkstr("length"),
	mkstr("prototype"),
	mkstr("caller"),		# for bind() generated instances
	mkstr("arguments"),		# for bind() generated instances

	# Array constructor
	mkstr("length"),
	mkstr("prototype"),
	mkstr("isArray"),

	# Array prototype
	mkstr("constructor"),
	mkstr("toString"),
	mkstr("toLocaleString"),
	mkstr("concat"),
	mkstr("join"),
	mkstr("pop"),
	mkstr("push"),
	mkstr("reverse"),
	mkstr("shift"),
	mkstr("slice"),
	mkstr("sort"),
	mkstr("splice"),
	mkstr("unshift"),
	mkstr("indexOf"),
	mkstr("lastIndexOf"),
	mkstr("every"),
	mkstr("some"),
	mkstr("forEach"),
	mkstr("map"),
	mkstr("filter"),
	mkstr("reduce"),
	mkstr("reduceRight"),

	# Array instances
	mkstr("length"),

	# String constructor
	mkstr("length"),
	mkstr("prototype"),
	mkstr("fromCharCode"),

	# String prototype
	mkstr("constructor"),
	mkstr("toString"),
	mkstr("valueOf"),
	mkstr("charAt"),
	mkstr("charCodeAt"),
	mkstr("concat"),
	mkstr("indexOf"),
	mkstr("lastIndexOf"),
	mkstr("localeCompare"),
	mkstr("match"),
	mkstr("replace"),
	mkstr("search"),
	mkstr("slice"),
	mkstr("split"),
	mkstr("substring"),
	mkstr("toLowerCase"),
	mkstr("toLocaleLowerCase"),
	mkstr("toUpperCase"),
	mkstr("toLocaleUpperCase"),
	mkstr("trim"),
	mkstr("substr", section_b=True),

	# String instances
	mkstr("length"),

	# Boolean constructor
	mkstr("length"),
	mkstr("prototype"),

	# Boolean prototype
	mkstr("constructor"),
	mkstr("toString"),
	mkstr("valueOf"),

	# Boolean instances
	# no special properties

	# Number constructor
	mkstr("length"),
	mkstr("prototype"),
	mkstr("MAX_VALUE"),
	mkstr("MIN_VALUE"),
	mkstr("NaN"),
	mkstr("NEGATIVE_INFINITY"),
	mkstr("POSITIVE_INFINITY"),

	# Number prototype
	mkstr("constructor"),
	mkstr("toString"),
	mkstr("toLocaleString"),
	mkstr("valueOf"),
	mkstr("toFixed"),
	mkstr("toExponential"),
	mkstr("toPrecision"),

	# Number instances
	# no special properties

	# Date constructor
	mkstr("length"),
	mkstr("prototype"),
	mkstr("parse"),
	mkstr("UTC"),
	mkstr("now"),

	# Date prototype
	mkstr("constructor"),
	mkstr("toString"),
	mkstr("toDateString"),
	mkstr("toTimeString"),
	mkstr("toLocaleString"),
	mkstr("toLocaleDateString"),
	mkstr("toLocaleTimeString"),
	mkstr("valueOf"),
	mkstr("getTime"),
	mkstr("getFullYear"),
	mkstr("getUTCFullYear"),
	mkstr("getMonth"),
	mkstr("getUTCMonth"),
	mkstr("getDate"),
	mkstr("getUTCDate"),
	mkstr("getDay"),
	mkstr("getUTCDay"),
	mkstr("getHours"),
	mkstr("getUTCHours"),
	mkstr("getMinutes"),
	mkstr("getUTCMinutes"),
	mkstr("getSeconds"),
	mkstr("getUTCSeconds"),
	mkstr("getMilliseconds"),
	mkstr("getUTCMilliseconds"),
	mkstr("getTimezoneOffset"),
	mkstr("setTime"),
	mkstr("setMilliseconds"),
	mkstr("setUTCMilliseconds"),
	mkstr("setSeconds"),
	mkstr("setUTCSeconds"),
	mkstr("setMinutes"),
	mkstr("setUTCMinutes"),
	mkstr("setHours"),
	mkstr("setUTCHours"),
	mkstr("setDate"),
	mkstr("setUTCDate"),
	mkstr("setMonth"),
	mkstr("setUTCMonth"),
	mkstr("setFullYear"),
	mkstr("setUTCFullYear"),
	mkstr("toUTCString"),
	mkstr("toISOString"),
	mkstr("toJSON"),
	mkstr("getYear", section_b=True),
	mkstr("setYear", section_b=True),
	mkstr("toGMTString", section_b=True),

	# Date instances
	# no special properties

	# RegExp constructor
	mkstr("length"),
	mkstr("prototype"),

	# RegExp prototype
	mkstr("constructor"),
	mkstr("exec"),
	mkstr("test"),
	mkstr("toString"),

	# RegExp instances
	mkstr("source"),
	mkstr("global"),
	mkstr("ignoreCase"),
	mkstr("multiline"),
	mkstr("lastIndex"),
	mkstr("(?:)"),

	# RegExp exec() results
	mkstr("index"),
	mkstr("input"),

	# Error constructor
	mkstr("length"),
	mkstr("prototype"),

	# Error prototype
	mkstr("constructor"),
	mkstr("name"),
	mkstr("message"),
	mkstr("toString"),

	# Error instances
	# no special properties

	# Error prototype / error fields (apply to all native errors in the spec)
	mkstr("name"),
	mkstr("message"),

	# Math object
	mkstr("E"),
	mkstr("LN10"),
	mkstr("LN2"),
	mkstr("LOG2E"),
	mkstr("LOG10E"),
	mkstr("PI"),
	mkstr("SQRT1_2"),
	mkstr("SQRT2"),
	mkstr("abs"),
	mkstr("acos"),
	mkstr("asin"),
	mkstr("atan"),
	mkstr("atan2"),
	mkstr("ceil"),
	mkstr("cos"),
	mkstr("exp"),
	mkstr("floor"),
	mkstr("log"),
	mkstr("max"),
	mkstr("min"),
	mkstr("pow"),
	mkstr("random"),
	mkstr("round"),
	mkstr("sin"),
	mkstr("sqrt"),
	mkstr("tan"),

	# JSON object
	mkstr("parse"),
	mkstr("stringify"),
]

# Other standard related strings
standard_other_string_list = [
	# typeof - these produce unfortunate naming conflicts like "Object" vs "object"
	mkstr("undefined"),
	mkstr("boolean"),
	mkstr("number"),
	mkstr("string"),
	mkstr("object"),	# also returned for typeof null
	mkstr("function"),

	# type related
	mkstr("undefined"),
	mkstr("null"),
	mkstr("true"),
	mkstr("false"),

	# special values
	mkstr("length"),
	mkstr("NaN"),
	mkstr("Infinity"),
	mkstr("+Infinity"),
	mkstr("-Infinity"),
	mkstr("0"),
	mkstr("+0"),
	mkstr("-0"),
	mkstr(""),

	# arguments object (E5 Section 10.6)
	mkstr("arguments"),
	mkstr("callee"),
	mkstr("caller"),
]

# Duktape specific strings
duk_string_list = [
	# non-standard class values
	mkstr("global", custom=True),	# implementation specific but shared by e.g. smjs and V8

	# non-standard error object properties
	mkstr("fileName", custom=True),
	mkstr("lineNumber", custom=True),
	mkstr("isNative", custom=True),
	mkstr("code", custom=True),
	mkstr("cause", custom=True),
	mkstr("traceback", custom=True),
	mkstr("trunc", custom=True),

	# non-standard function instance properties
	mkstr("name", custom=True),	# function declaration/expression name (or empty)

	# typeof - these produce unfortunate naming conflicts like "Object" vs "object"
	mkstr("buffer", custom=True),
	mkstr("pointer", custom=True),

	# internal property for primitive value (Boolean, Number, String)
	mkstr("value", internal=True, custom=True),

	# internal properties for enumerator objects
	mkstr("target", internal=True, custom=True),
	mkstr("next", internal=True, custom=True),

	# internal properties for RegExp instances
	mkstr("bytecode", internal=True, custom=True),

	# internal properties for function objects
	mkstr("formals", internal=True, custom=True),
	mkstr("varmap", internal=True, custom=True),
	mkstr("lexenv", internal=True, custom=True),
	mkstr("varenv", internal=True, custom=True),
	mkstr("source", internal=True, custom=True),
	mkstr("name", internal=True, custom=True),		# FIXME: remove?
	mkstr("pc2line", internal=True, custom=True),
	mkstr("filename", internal=True, custom=True),

	# internal properties for thread objects

	# internal properties for bound function objects
	mkstr("target", internal=True, custom=True),	# [[TargetFunction]]
	mkstr("this", internal=True, custom=True),	# [[BoundThis]]
	mkstr("args", internal=True, custom=True),	# [[BoundArguments]]

	# internal properties for argument objects
	mkstr("map", internal=True, custom=True),
	mkstr("callee", internal=True, custom=True),

	# internal properties for general objects
	mkstr("metatable", internal=True, custom=True),
	mkstr("finalizer", internal=True, custom=True),

	# internal properties for declarative environment records
	mkstr("callee", internal=True, custom=True),	# to access varmap
	mkstr("thread", internal=True, custom=True),	# to identify valstack
	mkstr("regbase", internal=True, custom=True),	# to determine absolute valstack index

	# internal properties for object environment records
	mkstr("target", internal=True, custom=True),	# target object
	mkstr("this", internal=True, custom=True),	# implicit this binding value

	# __duk__ object
	mkstr("__duk__", custom=True),
	mkstr("version", custom=True),
	mkstr("build", custom=True),
	mkstr("addr", custom=True),
	mkstr("refc", custom=True),
	mkstr("spawn", custom=True),
	mkstr("yield", custom=True),
	mkstr("resume", custom=True),
	mkstr("curr", custom=True),
	mkstr("gc", custom=True),
	mkstr("print", custom=True),
	mkstr("time", custom=True),
	mkstr("setFinalizer", custom=True),
	mkstr("getFinalizer", custom=True),
	mkstr("enc", custom=True),
	mkstr("dec", custom=True),
	mkstr("hex", custom=True),      # enc/dec alg
	mkstr("base64", custom=True),   # enc/dec alg
]

# Standard reserved words (non-strict mode + strict mode)
# Note: order must match DUK_TOK_XXX reserved defines in duk_types.h
standard_reserved_words_list = [
	# E5 Section 7.6.1

	# Keyword

	mkstr("break", reserved_word=True),
	mkstr("case", reserved_word=True),
	mkstr("catch", reserved_word=True),
	mkstr("continue", reserved_word=True),
	mkstr("debugger", reserved_word=True),
	mkstr("default", reserved_word=True),
	mkstr("delete", reserved_word=True),
	mkstr("do", reserved_word=True),
	mkstr("else", reserved_word=True),
	mkstr("finally", reserved_word=True),
	mkstr("for", reserved_word=True),
	mkstr("function", reserved_word=True),
	mkstr("if", reserved_word=True),
	mkstr("in", reserved_word=True),
	mkstr("instanceof", reserved_word=True),
	mkstr("new", reserved_word=True),
	mkstr("return", reserved_word=True),
	mkstr("switch", reserved_word=True),
	mkstr("this", reserved_word=True),
	mkstr("throw", reserved_word=True),
	mkstr("try", reserved_word=True),
	mkstr("typeof", reserved_word=True),
	mkstr("var", reserved_word=True),
	mkstr("void", reserved_word=True),
	mkstr("while", reserved_word=True),
	mkstr("with", reserved_word=True),

	# Future reserved word

	mkstr("class", reserved_word=True, future_reserved_word=True),
	mkstr("const", reserved_word=True, future_reserved_word=True),
	mkstr("enum", reserved_word=True, future_reserved_word=True),
	mkstr("export", reserved_word=True, future_reserved_word=True),
	mkstr("extends", reserved_word=True, future_reserved_word=True),
	mkstr("import", reserved_word=True, future_reserved_word=True),
	mkstr("super", reserved_word=True, future_reserved_word=True),

	# E5 Section 7.8.1 and 7.8.2: special literals which the lexer
	# basically treats like keywords

	mkstr("null", special_literal=True),
	mkstr("true", special_literal=True),
	mkstr("false", special_literal=True),
	mkstr("get", special_literal=True),
	mkstr("set", special_literal=True),
]

# Standard reserved words (strict mode only)
# Note: order must match DUK_TOK_XXX reserved defines in duk_types.h
standard_reserved_words_strict_string_list = [
	# Future reserved word (additionally in strict mode)

	mkstr("implements", reserved_word=True, future_reserved_word_strict=True),
	mkstr("interface", reserved_word=True, future_reserved_word_strict=True),
	mkstr("let", reserved_word=True, future_reserved_word_strict=True),
	mkstr("package", reserved_word=True, future_reserved_word_strict=True),
	mkstr("private", reserved_word=True, future_reserved_word_strict=True),
	mkstr("protected", reserved_word=True, future_reserved_word_strict=True),
	mkstr("public", reserved_word=True, future_reserved_word_strict=True),
	mkstr("static", reserved_word=True, future_reserved_word_strict=True),
	mkstr("yield", reserved_word=True, future_reserved_word_strict=True),
]

#
#  Forced define names for specific strings for which automatic name generation
#  does a bad job.
#

special_define_names = {
	# typeof has name conflicts like "object" and "Object", broken with
	# these unfortunately hacky defines
	'object': 'LC_OBJECT',
	'Object': 'UC_OBJECT',
	'boolean': 'LC_BOOLEAN',
	'Boolean': 'UC_BOOLEAN',
	'number': 'LC_NUMBER',
	'Number': 'UC_NUMBER',
	'function': 'LC_FUNCTION',
	'Function': 'UC_FUNCTION',
	'string': 'LC_STRING',
	'String': 'UC_STRING',
	'arguments': 'LC_ARGUMENTS',
	'Arguments': 'UC_ARGUMENTS',

	'+Infinity': 'PLUS_INFINITY',
	'-Infinity': 'MINUS_INFINITY',
	'0': 'ZERO',
	'+0': 'PLUS_ZERO',
	'-0': 'MINUS_ZERO',
	'NaN': 'NAN',
	'isNaN': 'IS_NAN',	
	'MIN_VALUE': 'MIN_VALUE',
	'MAX_VALUE': 'MAX_VALUE',
	'NEGATIVE_INFINITY': 'NEGATIVE_INFINITY',
	'POSITIVE_INFINITY': 'POSITIVE_INFINITY',
	'decodeURIComponent': 'DECODE_URI_COMPONENT',
	'encodeURIComponent': 'ENCODE_URI_COMPONENT',
	'getUTCDate': 'GET_UTC_DATE',
	'getUTCDay': 'GET_UTC_DAY',
	'getUTCFullYear': 'GET_UTC_FULL_YEAR',
	'getUTCHours': 'GET_UTC_HOURS',
	'getUTCMilliseconds': 'GET_UTC_MILLISECONDS',
	'getUTCMinutes': 'GET_UTC_MINUTES',
	'getUTCMonth': 'GET_UTC_MONTH',
	'getUTCSeconds': 'GET_UTC_SECONDS',
	'setUTCDate': 'SET_UTC_DATE',
	'setUTCDay': 'SET_UTC_DAY',
	'setUTCFullYear': 'SET_UTC_FULL_YEAR',
	'setUTCHours': 'SET_UTC_HOURS',
	'setUTCMilliseconds': 'SET_UTC_MILLISECONDS',
	'setUTCMinutes': 'SET_UTC_MINUTES',
	'setUTCMonth': 'SET_UTC_MONTH',
	'setUTCSeconds': 'SET_UTC_SECONDS',
	'LOG10E': 'LOG10E',
	'LOG2E': 'LOG2E',
	'toISOString': 'TO_ISO_STRING',
	'toUTCString': 'TO_UTC_STRING',
	'toGMTString': 'TO_GMT_STRING',
	'URIError': 'URI_ERROR',
	'__duk__': 'DUK',
	'': 'EMPTY_STRING',

	'(?:)': 'ESCAPED_EMPTY_REGEXP',
}

#
#  String table generation
#

# Get a define name for a string
def get_define_name(x):
	if special_define_names.has_key(x):
		return define_prefix + special_define_names[x]

	is_internal = False
	if len(x) >= 1 and x[0] == '\x00':
		is_internal = True
		x = x[1:]

	res = ''
	if is_internal:
		res += 'INT_'

	prev_upper = False
	for i in x:
		if i.isupper():
			if len(res) > 0 and not prev_upper:
				res += '_'

		res += i.upper()
		prev_upper = i.isupper()

	return define_prefix + res

def gen_strings_data_bitpacked(strlist):
	be = dukutil.BitEncoder()

	freq = [0] * 256
	maxlen = 0
	maxval = 0
	for s, d in strlist:
		for c in s:
			freq[ord(c)] += 1
		if len(s) > maxlen:
			maxlen = len(s)
		for c in s:
			if ord(c) > maxval:
				maxval = ord(c)

	lookup = []
	invlookup = [0] * 256
	for i in xrange(256):
		if freq[i] != 0:
			lookup.append(i)
	for i in xrange(len(lookup)):
		x = lookup[i]
		invlookup[x] = i

	uniq = len(lookup)

	if uniq > 63:
		raise Exception('too many unique characters for current assumptions')
	if maxlen > 31:
		raise Exception('string too long for current assumptions')
	if maxval > 127:
		raise Exception('string maxval too high for current assumptions')

        databits = []

	# lookup table for chars (6 bits -> 7 bit value)
	# XXX: 55 bytes, can halve by encoding first value and then 3-bit skips,
	# but net benefit maybe 20 bytes.
	for i in xrange(uniq):
		be.bits(lookup[i], 7)

	# strings: 5-bit length, N*6-bit characters
	for s, d in strlist:
		be.bits(len(s), 5)
		for c in s:
			be.bits(invlookup[ord(c)], 6)
	# end marker not necessary, C code knows length from define

	res = be.getByteString()

	print '%d bytes of string init data, %d unique bytes in strings, %d maximum string length, %d maximum code point value' % \
		(len(res), uniq, maxlen, maxval)

	return res, uniq, maxlen, maxval

if __name__ == '__main__':
	parser = optparse.OptionParser()
	parser.add_option('--out-header', dest='out_header')
	parser.add_option('--out-source', dest='out_source')
	parser.add_option('--out-python', dest='out_python')
	parser.add_option('--out-bin', dest='out_bin')
	parser.add_option('--byte-order', dest='byte_order')	# currently unused
	(opts, args) = parser.parse_args()

	# Strings are ordered in the result as follows:
	#   1. Strings not in either of the following two categories
	#   2. Reserved words in strict mode only
	#   3. Reserved words in both non-strict and strict mode
	#
	# Strings that below in multiple categories (such as 'true') are
	# output in the earliest category.  The lexer needs to have the
	# reserved words in an easy-to-use order.  Strings needed by built-in
	# initialization should be in the beginning to ensure that a 1-byte
	# index suffices for them.
	#
	# XXX: this should be based on string metadata given to mkstr().

	strlist = []
	num_nonstrict_reserved = None
	num_strict_reserved = None
	num_all_reserved = None
	idx_start_reserved = None
	idx_start_strict_reserved = None

	def _add(x, append):
		n_str = x
		n_def = get_define_name(x)
		for o_str, o_def in strlist:
			if o_str == n_str and o_def == n_def:
				# same string, same define => no action
				return
			if o_str == n_str and o_def != n_def:
				# same string, different define => should not happen
				raise Exception('same string, different define for %s' % n_str)
			if o_str != n_str and o_def == n_def:
				# different string, same define => need custom defines
				raise Exception('different string, same define for %s' % n_str)
		# all ok, add
		if append:
			strlist.append((n_str, n_def))
		else:
			strlist.insert(0, (n_str, n_def))

	for i in standard_reserved_words_list:
		_add(i, True)
	num_nonstrict_reserved = len(strlist)

	for i in standard_reserved_words_strict_string_list:
		_add(i, True)
	num_all_reserved = len(strlist)
	num_strict_reserved = num_all_reserved - num_nonstrict_reserved

	for i in standard_builtin_string_list:
		_add(i, False)

	for i in standard_other_string_list:
		_add(i, False)

	for i in duk_string_list:
		_add(i, False)

	idx_start_reserved = len(strlist) - num_all_reserved
	idx_start_strict_reserved = len(strlist) - num_strict_reserved

	strdata, lookuplen, maxlen, maxval = gen_strings_data_bitpacked(strlist)

	# write raw data file
	f = open(opts.out_bin, 'wb')
	f.write(strdata)
	f.close()

	# write C source file
	genc = dukutil.GenerateC()
	genc.emitHeader('genstrings.py')
	genc.emitArray(strdata, 'duk_strings_data')  # FIXME: unsigned char?
	genc.emitLine('')
	genc.emitLine('/* to convert a heap stridx to a token number, subtract')
	genc.emitLine(' * DUK_HEAP_STRIDX_START_RESERVED and add DUK_TOK_START_RESERVED.')
	genc.emitLine(' */')
	f = open(opts.out_source, 'wb')
	f.write(genc.getString())
	f.close()

	# write C header file
	genc = dukutil.GenerateC()
	genc.emitHeader('genstrings.py')
	genc.emitLine('#ifndef __DUK_STRINGS_H')
	genc.emitLine('#define __DUK_STRINGS_H 1')
	genc.emitLine('')
	genc.emitLine('extern char duk_strings_data[];')  # FIXME: unsigned char?
	genc.emitLine('')
	genc.emitDefine('DUK_STRDATA_DATA_LENGTH', len(strdata))
	genc.emitDefine('DUK_STRDATA_LOOKUP_LENGTH', lookuplen)
	genc.emitDefine('DUK_STRDATA_MAX_STRLEN', maxlen)
	genc.emitLine('')
	idx = 0
	for s, d in strlist:
		genc.emitDefine(d, idx, repr(s))
		idx += 1
	genc.emitLine('')
	idx = 0
	for s, d in strlist:
		defname = d.replace('_HEAP_STRIDX','_HEAP_STRING')  # FIXME
		genc.emitDefine(defname + '(heap)', 'DUK_HEAP_GET_STRING((heap),%s)' % d)
		defname = d.replace('_HEAP_STRIDX', '_HTHREAD_STRING')
		genc.emitDefine(defname + '(thr)', 'DUK_HTHREAD_GET_STRING((thr),%s)' % d)
		idx += 1
	genc.emitLine('')
	genc.emitDefine('DUK_HEAP_NUM_STRINGS', idx)
	genc.emitLine('')
	genc.emitDefine('DUK_HEAP_STRIDX_START_RESERVED', idx_start_reserved)
	genc.emitDefine('DUK_HEAP_STRIDX_START_STRICT_RESERVED', idx_start_strict_reserved)
	genc.emitDefine('DUK_HEAP_STRIDX_END_RESERVED', len(strlist), comment='exclusive endpoint')
	genc.emitLine('')
	genc.emitLine('#endif  /* __DUK_STRINGS_H */')
	f = open(opts.out_header, 'wb')
	f.write(genc.getString())
	f.close()

	f.close()

	# generate Python file
	# XXX: helper
	f = open(opts.out_python, 'wb')
	f.write('# automatically generated by genstrings.py, do not edit\n')
	f.write('\n')
	f.write('define_name_to_index = {\n')
	idx = 0
	for s, d in strlist:
		f.write('\t%s: %d,\n' % (repr(d), idx))
		idx += 1
	f.write('}\n')
	f.write('\n')
	f.write('real_name_to_index = {\n')
	idx = 0
	for s, d in strlist:
		f.write('\t%s: %d,\n' % (repr(s), idx))
		idx += 1
	f.write('}\n')
	f.write('\n')
	f.write('index_to_define_name = {}\n')
	f.write('for k in define_name_to_index.keys():\n')
	f.write('\tindex_to_define_name[define_name_to_index[k]] = k\n')
	f.write('\n')
	f.write('index_to_real_name = {}\n')
	f.write('for k in real_name_to_index.keys():\n')
	f.write('\tindex_to_real_name[real_name_to_index[k]] = k\n')
	f.write('\n')
	
	f.close()

	sys.exit(0)


