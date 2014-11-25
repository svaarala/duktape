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

#  XXX: integrate more tightly with genbuiltins.py
#  XXX: add code to indicate strings which are needed at runtime
#       (may be profile dependent); then detect which strings
#       genbuiltins.py needs, and finally log unused strings
#       Perhaps string lists need to be added programmatically and
#       may be omitted based on profile
#  XXX: avoid debug-related strings in release build (same applies to
#       other configuration dependent strings, like traceback data)
#  XXX: better compression
#  XXX: reserved word stridx's could be made to match token numbers
#       directly so that a duk_stridx2token[] would not be needed
#  XXX: improve per string metadata, and sort strings within constraints
#  XXX: some Duktape internal strings could just reuse existing strings

import os
import sys
import optparse
import dukutil

# Prefix for defines

define_prefix = 'DUK_STRIDX_'

#
#  String lists
#
#  Some strings may appear in multiple lists and even in multiple roles.
#

# XXX: currently the keywords are not recorded; use them later to organize
# strings more optimally

class BuiltinString:
	name = None
	section_b = None
	browser_like = None
	es6 = None
	custom = None
	internal = None
	reserved_word = None
	future_reserved_word = None
	future_reserved_word_strict = None
	special_literal = None
	class_name = None

	# computed
	req_8bit = None

	def __init__(self):
		pass

def mkstr(x,
          section_b=False,
          browser_like=False,
          es6=False,
          commonjs=False,
          custom=False,
          internal=False,
          reserved_word=False,
          future_reserved_word=False,
          future_reserved_word_strict=False,
          special_literal=False,
          class_name=False):

	"Create a string object."

	# A 0xff prefix (never part of valid UTF-8) is used for internal properties.
	# It is encoded as 0x00 in generated init data for technical reasons: it
	# keeps lookup table elements 7 bits instead of 8 bits.  The initial byte
	# of a Duktape internal string is always capitalized (e.g. \x00Value) so
	# that user code can use clean lowercase prefixes like "\xFFptr".

	if internal:
		if len(x) < 1 or not (ord(x[0]) >= ord('A') and ord(x[0]) <= ord('Z')):
			raise Exception('invalid internal key: %s' % repr(x))
		x = '\x00' + x

	ret = BuiltinString()
	ret.name = x
	ret.section_b = section_b
	ret.browser_like = browser_like
	ret.es6 = es6
	ret.commonjs = commonjs
	ret.custom = custom
	ret.internal = internal
	ret.reserved_word = reserved_word
	ret.future_reserved_word = future_reserved_word
	ret.future_reserved_word_strict = future_reserved_word_strict
	ret.special_literal = special_literal
	ret.class_name = class_name

	ret.req_8bit = False
	if class_name:
		ret.req_8bit = True

	return ret

# Standard built-in object related strings
standard_builtin_string_list = [
	# internal class values
	mkstr("Undefined", class_name=True),   # sort of
	mkstr("Null", class_name=True),        # sort of
	mkstr("Object", class_name=True),
	mkstr("Function", class_name=True),
	mkstr("Array", class_name=True),
	mkstr("String", class_name=True),
	mkstr("Boolean", class_name=True),
	mkstr("Number", class_name=True),
	mkstr("Date", class_name=True),
	mkstr("RegExp", class_name=True),
	mkstr("Error", class_name=True),
	mkstr("Math", class_name=True),
	mkstr("JSON", class_name=True),
	mkstr("Arguments", class_name=True),

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
	mkstr("", class_name=True),	# used as a class name for unused/invalid class
	mkstr(","),			# for array joining
	mkstr(" "),			# for print()
	mkstr("\n\t"),			# for tracebacks
	mkstr("[...]"),			# for tracebacks
	mkstr("Invalid Date"),		# for invalid Date instances
	# arguments object (E5 Section 10.6)
	mkstr("arguments"),
	mkstr("callee"),
	mkstr("caller"),

	# "set" and "get" are strings we need in object literals but they are not
	# ReservedWords.

	mkstr("get"),
	mkstr("set"),
]

# ES6 (draft) specific strings
es6_string_list = [
	mkstr("Proxy", es6=True),
	#mkstr("revocable", es6=True),

	# Proxy trap names
	mkstr("has", es6=True),
	mkstr("set", es6=True),
	mkstr("get", es6=True),
	mkstr("deleteProperty", es6=True),
	mkstr("enumerate", es6=True),
	mkstr("ownKeys", es6=True),

	mkstr("setPrototypeOf", es6=True),
	mkstr("__proto__", es6=True),
]

# CommonJS related strings
commonjs_string_list = [
	mkstr("require", commonjs=True),
	mkstr("id", commonjs=True),
]

# Duktape specific strings
duk_string_list = [
	# non-standard global properties
	mkstr("Duktape", custom=True),

	# non-standard class values
	mkstr("global", custom=True, class_name=True),	# implementation specific but shared by e.g. smjs and V8
	mkstr("ObjEnv", custom=True, class_name=True),
	mkstr("DecEnv", custom=True, class_name=True),
	mkstr("Buffer", custom=True, class_name=True),
	mkstr("Pointer", custom=True, class_name=True),
	mkstr("Thread", custom=True, class_name=True),
	mkstr("Logger", custom=True, class_name=True),

	# non-standard built-in object names
	mkstr("ThrowTypeError", custom=True),  # implementation specific, matches V8

	# non-standard error object (or Error.prototype) properties
	mkstr("stack", custom=True),
	mkstr("pc", custom=True),
	mkstr("fileName", custom=True),
	mkstr("lineNumber", custom=True),
	#mkstr("code", custom=True),
	mkstr("Tracedata", internal=True, custom=True),

	# non-standard function instance properties
	mkstr("name", custom=True), 	# function declaration/expression name (or empty)
	mkstr("fileName", custom=True), # filename associated with function (shown in tracebacks)

	# typeof - these produce unfortunate naming conflicts like "Object" vs "object"
	mkstr("buffer", custom=True),
	mkstr("pointer", custom=True),

	# internal property for primitive value (Boolean, Number, String)
	mkstr("Value", internal=True, custom=True),

	# internal properties for enumerator objects
	mkstr("Target", internal=True, custom=True),
	mkstr("Next", internal=True, custom=True),

	# internal properties for RegExp instances
	mkstr("Bytecode", internal=True, custom=True),

	# internal properties for function objects
	mkstr("Formals", internal=True, custom=True),
	mkstr("Varmap", internal=True, custom=True),
	mkstr("Lexenv", internal=True, custom=True),
	mkstr("Varenv", internal=True, custom=True),
	mkstr("Source", internal=True, custom=True),
	mkstr("Pc2line", internal=True, custom=True),

	# internal properties for thread objects

	# internal properties for bound function objects
	mkstr("Target", internal=True, custom=True),	# [[TargetFunction]]
	mkstr("This", internal=True, custom=True),	# [[BoundThis]]
	mkstr("Args", internal=True, custom=True),	# [[BoundArguments]]

	# internal properties for argument objects
	mkstr("Map", internal=True, custom=True),
	mkstr("Callee", internal=True, custom=True),

	# internal properties for general objects
	#mkstr("Metatable", internal=True, custom=True),
	mkstr("Finalizer", internal=True, custom=True),

	# internal properties for Proxy objects
	mkstr("Target", internal=True, custom=True),	# [[ProxyTarget]]
	mkstr("Handler", internal=True, custom=True),	# [[ProxyHandler]]

	# internal properties for declarative environment records
	mkstr("Callee", internal=True, custom=True),	# to access varmap
	mkstr("Thread", internal=True, custom=True),	# to identify valstack
	mkstr("Regbase", internal=True, custom=True),	# to determine absolute valstack index

	# internal properties for object environment records
	mkstr("Target", internal=True, custom=True),	# target object
	mkstr("This", internal=True, custom=True),	# implicit this binding value

	# fake filename for compiled functions
	mkstr("compile", custom=True),                  # used as a filename for functions created with Function constructor
	mkstr("input", custom=True),                    # used as a filename for eval temp function

	# Duktape object
	mkstr("errCreate", custom=True),
	mkstr("errThrow", custom=True),
	mkstr("modSearch", custom=True),
	mkstr("modLoaded", custom=True),
	mkstr("env", custom=True),
	mkstr("version", custom=True),
	mkstr("info", custom=True),
	mkstr("act", custom=True),
	mkstr("gc", custom=True),
	mkstr("fin", custom=True),
	mkstr("enc", custom=True),
	mkstr("dec", custom=True),
	mkstr("hex", custom=True),      # enc/dec alg
	mkstr("base64", custom=True),   # enc/dec alg
	mkstr("jx", custom=True),       # enc/dec alg
	mkstr("jc", custom=True),       # enc/dec alg
	mkstr("compact", custom=True),

	# Buffer constructor

	# Buffer prototype

	# Pointer constructor

	# Pointer prototype

	# Thread constructor
	mkstr("yield", custom=True),
	mkstr("resume", custom=True),
	mkstr("current", custom=True),

	# Thread prototype

	# Logger constructor

	# Logger prototype and logger instances
	mkstr("fmt", custom=True),
	mkstr("raw", custom=True),
	mkstr("trace", custom=True),
	mkstr("debug", custom=True),
	mkstr("info", custom=True),
	mkstr("warn", custom=True),
	mkstr("error", custom=True),
	mkstr("fatal", custom=True),
	mkstr("n", custom=True),
	mkstr("l", custom=True),

	# Auxiliary logger strings
	mkstr("clog", custom=True),  # C logger

	# for controlling log formatting of objects
	mkstr("toLogString", custom=True),

	# special literals for custom json encodings
	mkstr('{"_undef":true}', custom=True),
	mkstr('{"_nan":true}', custom=True),
	mkstr('{"_inf":true}', custom=True),
	mkstr('{"_ninf":true}', custom=True),
	mkstr('{"_func":true}', custom=True),
	mkstr('{_func:true}', custom=True),
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

	# "set" and "get" are *NOT* reserved words and there is even code
	# in the wild with statements like 'var set = 1;'.  They are thus
	# treated as ordinary identifiers and recognized by the compiler
	# as tokens in a special way.
	#mkstr("get"),
	#mkstr("set"),
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
	'undefined': 'LC_UNDEFINED',
	'Undefined': 'UC_UNDEFINED',
	'null': 'LC_NULL',
	'Null': 'UC_NULL',
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
	'buffer': 'LC_BUFFER',
	'Buffer': 'UC_BUFFER',
	'pointer': 'LC_POINTER',
	'Pointer': 'UC_POINTER',
	#'thread': 'LC_THREAD',
	'Thread': 'UC_THREAD',
	#'logger': 'LC_LOGGER',
	'Logger': 'UC_LOGGER',
	'n': 'LC_N',
	'l': 'LC_L',

	'error': 'LC_ERROR',
	'Error': 'UC_ERROR',

	# log levels
	'trace': 'LC_TRACE',
	#'Trace': 'UC_TRACE',
	'debug': 'LC_DEBUG',
	#'Debug': 'UC_DEBUG',
	'info': 'LC_INFO',
	#'Info': 'UC_INFO',
	'warn': 'LC_WARN',
	#'Warn': 'UC_WARN',
	#'error': 'LC_ERROR',  # already above
	#'Error': 'UC_ERROR',
	'fatal': 'LC_FATAL',
	#'Fatal': 'UC_FATAL',

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
	'(?:)': 'ESCAPED_EMPTY_REGEXP',
	'Invalid Date': 'INVALID_DATE',

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
	'Duktape': 'DUKTAPE',
	'': 'EMPTY_STRING',
	',': 'COMMA',
	' ': 'SPACE',
	'\n\t': 'NEWLINE_TAB',
	'[...]': 'BRACKETED_ELLIPSIS',

	'{"_undef":true}': 'JSON_EXT_UNDEFINED',
	'{"_nan":true}': 'JSON_EXT_NAN',
	'{"_inf":true}': 'JSON_EXT_POSINF',
	'{"_ninf":true}': 'JSON_EXT_NEGINF',
	'{"_func":true}': 'JSON_EXT_FUNCTION1',
	'{_func:true}': 'JSON_EXT_FUNCTION2',
}

#
#  String table generation
#

# Get a define name for a string
def get_define_name(x):
	x = x.name
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
	for idx, c in enumerate(x):
		if c.isupper():
			if (idx > 0 and not prev_upper):
				res += '_'

		res += c.upper()
		prev_upper = c.isupper()

	return define_prefix + res

def gen_strings_data_bitpacked(strlist):
	be = dukutil.BitEncoder()

	# Strings are encoded as follows: a string begins in lowercase
	# mode and recognizes the following 5-bit symbols:
	#
	#    0-25    'a' ... 'z'
	#    26	     '_'
	#    27      0x00 (actually decoded to 0xff, internal marker)
	#    28	     reserved
	#    29      switch to uppercase for one character
	#            (next 5-bit symbol must be in range 0-25)
	#    30      switch to uppercase
	#    31      read a 7-bit character verbatim
	#
	# Uppercase mode is the same except codes 29 and 30 switch to
	# lowercase.

	UNDERSCORE = 26
	ZERO = 27
	SWITCH1 = 29
	SWITCH = 30
	SEVENBIT = 31

	maxlen = 0
	n_optimal = 0
	n_switch1 = 0
	n_switch = 0
	n_sevenbit = 0

	for s, d in strlist:
		be.bits(len(s), 5)

		if len(s) > maxlen:
			maxlen = len(s)

		# 5-bit character, mode specific
		mode = 'lowercase'

		for idx, c in enumerate(s):
			# This encoder is not that optimal, but good enough for now.

			islower = (ord(c) >= ord('a') and ord(c) <= ord('z'))
			isupper = (ord(c) >= ord('A') and ord(c) <= ord('Z'))
			islast = (idx == len(s) - 1)
			isnextlower = False
			isnextupper = False
			if not islast:
				c2 = s[idx+1]
				isnextlower = (ord(c2) >= ord('a') and ord(c2) <= ord('z'))
				isnextupper = (ord(c2) >= ord('A') and ord(c2) <= ord('Z'))

			if c == '_':
				be.bits(UNDERSCORE, 5)
				n_optimal += 1
			elif c == '\x00':
				be.bits(ZERO, 5)
				n_optimal += 1
			elif islower and mode == 'lowercase':
				be.bits(ord(c) - ord('a'), 5)
				n_optimal += 1
			elif isupper and mode == 'uppercase':
				be.bits(ord(c) - ord('A'), 5)
				n_optimal += 1
			elif islower and mode == 'uppercase':
				if isnextlower:
					be.bits(SWITCH, 5)
					be.bits(ord(c) - ord('a'), 5)
					mode = 'lowercase'
					n_switch += 1
				else:
					be.bits(SWITCH1, 5)
					be.bits(ord(c) - ord('a'), 5)
					n_switch1 += 1
			elif isupper and mode == 'lowercase':
				if isnextupper:
					be.bits(SWITCH, 5)
					be.bits(ord(c) - ord('A'), 5)
					mode = 'uppercase'
					n_switch += 1
				else:
					be.bits(SWITCH1, 5)
					be.bits(ord(c) - ord('A'), 5)
					n_switch1 += 1
			else:
				assert(ord(c) >= 0 and ord(c) <= 127)
				be.bits(SEVENBIT, 5)
				be.bits(ord(c), 7)
				n_sevenbit += 1
				#print 'sevenbit for: %r' % c

	# end marker not necessary, C code knows length from define

	res = be.getByteString()

	print ('%d strings, %d bytes of string init data, %d maximum string length, ' + \
	       'encoding: optimal=%d,switch1=%d,switch=%d,sevenbit=%d') % \
		(len(strlist), len(res), maxlen, \
	         n_optimal, n_switch1, n_switch, n_sevenbit)

	return res, maxlen

def gen_string_list():
	# Strings are ordered in the result as follows:
	#   1. Strings not in either of the following two categories
	#   2. Reserved words in strict mode only
	#   3. Reserved words in both non-strict and strict mode
	#
	# Reserved words must follow an exact order because they are
	# translated to/from token numbers by addition/subtraction.
	# The remaining strings (in category 1) must be ordered so
	# that those strings requiring an 8-bit index must be in the
	# beginning.
	#
	# XXX: quite hacky, rework.

	strlist = []
	num_nonstrict_reserved = None
	num_strict_reserved = None
	num_all_reserved = None
	idx_start_reserved = None
	idx_start_strict_reserved = None

	def _add(x, append):
		n_str = x.name
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

	# Add reserved words in order of occurrence first.  The order matters
	# because the string indices must be convertible to token numbers by
	# addition/subtraction.

	for i in standard_reserved_words_list:
		_add(i, True)
	num_nonstrict_reserved = len(strlist)

	for i in standard_reserved_words_strict_string_list:
		_add(i, True)
	num_all_reserved = len(strlist)
	num_strict_reserved = num_all_reserved - num_nonstrict_reserved

	# Figure out, for the remaining strings, which strings need to be
	# in the 8-bit range.  Note that a certain string may appear multiple
	# times in different roles (e.g. as a class name and a built-in object
	# name) so check every occurrence.

	req_8bit = {}

	str_lists = [ standard_builtin_string_list,
	              standard_other_string_list,
	              es6_string_list,
	              commonjs_string_list,
	              duk_string_list ]

	for lst in str_lists:
		for i in lst:
			if i.req_8bit:
				req_8bit[i.name] = True

	# Prepend strings not requiring 8-bit indices first; then prepend
	# strings requiring 8-bit indices (as early as possible).

	for lst in str_lists:
		for i in lst:
			if req_8bit.has_key(i.name):
				continue
			_add(i, False)

	for lst in str_lists:
		for i in lst:
			_add(i, False)

	# Check that 8-bit string constraints are satisfied

	for i,v in enumerate(strlist):
		name, defname = v[0], v[1]
		if req_8bit.has_key(name):
			if i >= 256:
				raise Exception('8-bit string index not satisfied: ' + repr(v))

	#for i,v in enumerate(strlist):
	#	print(i,v)

	idx_start_reserved = len(strlist) - num_all_reserved
	idx_start_strict_reserved = len(strlist) - num_strict_reserved

	return strlist, idx_start_reserved, idx_start_strict_reserved

class GenStrings:
	strlist = None				# list of (name, define) pairs
	strdata = None				# bit packed initializer data
	idx_start_reserved = None		# start of reserved keywords
	idx_start_strict_reserved = None	# start of strict reserved keywords
	maxlen = None				# length of longest string
	string_to_index = None			# map of name -> index
	define_to_index = None			# map of define name -> index

	def __init__(self):
		pass

	def processStrings(self):
		self.strlist, self.idx_start_reserved, self.idx_start_strict_reserved = gen_string_list()
		self.strdata, self.maxlen = gen_strings_data_bitpacked(self.strlist)

		# initialize lookup maps
		self.string_to_index = {}
		self.define_to_index = {}
		idx = 0
		for s, d in self.strlist:
			self.string_to_index[s] = idx
			self.define_to_index[d] = idx
			idx += 1

	def stringToIndex(self, x):
		return self.string_to_index[x]

	def defineToIndex(self, x):
		return self.define_to_index[x]

	def hasString(self, x):
		return self.string_to_index.has_key(x)

	def hasDefine(self, x):
		return self.define_to_index.has_key(x)

	def emitStringsData(self, genc):
		genc.emitArray(self.strdata, 'duk_strings_data', visibility='DUK_INTERNAL', typename='duk_uint8_t', intvalues=True, const=True, size=len(self.strdata))
		genc.emitLine('')
		genc.emitLine('/* to convert a heap stridx to a token number, subtract')
		genc.emitLine(' * DUK_STRIDX_START_RESERVED and add DUK_TOK_START_RESERVED.')
		genc.emitLine(' */')

	def emitStringsHeader(self, genc):
		genc.emitLine('#if !defined(DUK_SINGLE_FILE)')
		genc.emitLine('DUK_INTERNAL_DECL const duk_uint8_t duk_strings_data[%d];' % len(self.strdata))
		genc.emitLine('#endif  /* !DUK_SINGLE_FILE */')
		genc.emitLine('')
		genc.emitDefine('DUK_STRDATA_DATA_LENGTH', len(self.strdata))
		genc.emitDefine('DUK_STRDATA_MAX_STRLEN', self.maxlen)
		genc.emitLine('')
		idx = 0
		for s, d in self.strlist:
			genc.emitDefine(d, idx, repr(s))
			idx += 1
		genc.emitLine('')
		idx = 0
		for s, d in self.strlist:
			defname = d.replace('_STRIDX','_HEAP_STRING')
			genc.emitDefine(defname + '(heap)', 'DUK_HEAP_GET_STRING((heap),%s)' % d)
			defname = d.replace('_STRIDX', '_HTHREAD_STRING')
			genc.emitDefine(defname + '(thr)', 'DUK_HTHREAD_GET_STRING((thr),%s)' % d)
			idx += 1
		genc.emitLine('')
		genc.emitDefine('DUK_HEAP_NUM_STRINGS', idx)
		genc.emitLine('')
		genc.emitDefine('DUK_STRIDX_START_RESERVED', self.idx_start_reserved)
		genc.emitDefine('DUK_STRIDX_START_STRICT_RESERVED', self.idx_start_strict_reserved)
		genc.emitDefine('DUK_STRIDX_END_RESERVED', len(self.strlist), comment='exclusive endpoint')

	def getStringList(self):
		strs = []
		strs_base64 = []
		for s, d in self.strlist:
			# The 'strs' list has strings as-is, with U+0000 marking the
			# internal prefix (it's not correct as runtime we use \xFF).
			#
			# The 'strs_base64' is byte exact to allow an application to
			# use it for e.g. external strings optimization.  The strings
			# are encoded to UTF-8, internal prefix is replaced with \xFF,
			# and the result is base-64 encoded to maintain byte exactness.

			t = s.encode('utf-8')
			if len(t) > 0 and t[0] == '\x00':
				t = '\xff' + t[1:]
			t = t.encode('base64')
			if len(t) > 0 and t[-1] == '\n':
				t = t[0:-1]
			strs.append(s)
			strs_base64.append(t)
		return strs, strs_base64
