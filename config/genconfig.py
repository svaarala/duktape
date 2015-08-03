#!/usr/bin/python
#
#  Process Duktape option metadata and produce various useful outputs:
#
#    - duk_config.h matching Duktape 1.x feature option model (DUK_OPT_xxx)
#    - duk_config.h for a selected platform, compiler, forced options, etc.
#    - option documentation for Duktape 1.x feature options (DUK_OPT_xxx)
#    - option documentation for Duktape 1.x/2.x config options (DUK_USE_xxx)
#
#  This script is intended to support a large number of platforms, so there
#  is a lot of platform specific clutter here.  The intent is NOT to support
#  every possible platform out there, but enough to make it reasonable to adapt
#  some existing configuration to an exotic platform manually.
#

# FIXME: validate use/opt def references (referenced define exists)
# FIXME: prune metadata and unnecessary snippets
# FIXME: use separate directories for platforms, compilers, etc?
# FIXME: for deprecated/obsolete checks, compare explicitly against current
#        version number instead of assuming presence <=> deprecated/obsolete
#        (would allow for e.g. planned obsoleting metadata)

import os
import sys
import re
import json
import yaml
import optparse
import tarfile
import tempfile
import atexit
import shutil
import StringIO

def get_auto_delete_tempdir():
	tmpdir = tempfile.mkdtemp(suffix='-genconfig')
	def _f(dirname):
		print 'Deleting temporary directory: %r' % dirname
		if os.path.isdir(dirname) and '-genconfig' in dirname:
			shutil.rmtree(dirname)
	atexit.register(_f, tmpdir)
	return tmpdir

def strip_comments_from_lines(lines):
	# Not exact but close enough.  Doesn't handle string literals etc,
	# but these are not a concrete issue for scanning preprocessor
	# #define references.
	#
	# Comment contents are stripped of any DUK_ prefixed text to avoid
	# incorrect requires/provides detection.  Other comment text is kept;
	# in particular a "/* redefine */" comment must remain intact here.
	#
	# Avoid Python 2.6 vs. Python 2.7 argument differences.

	def censor(x):
		return re.sub(re.compile('DUK_\w+', re.MULTILINE), 'xxx', x.group(0))

	tmp = '\n'.join(lines)
	tmp = re.sub(re.compile('/\*.*?\*/', re.MULTILINE | re.DOTALL), censor, tmp)
	tmp = re.sub(re.compile('//.*?$', re.MULTILINE), censor, tmp)
	return tmp.split('\n')

# Header snippet representation: lines, provides defines, requires defines.
re_line_provides = re.compile(r'^#(?:define|undef)\s+(\w+).*$')
re_line_requires = re.compile(r'(DUK_\w+)')
class Snippet:
	lines = None     # lines of text and/or snippets
	provides = None  # map from define to 'True' for now
	requires = None  # map from define to 'True' for now

	def __init__(self, lines, provides=None, requires=None, autoscan_requires=True, autoscan_provides=True):
		self.lines = []
		for line in lines:
			if isinstance(line, str):
				self.lines.append(line)
			elif isinstance(line, unicode):
				self.lines.append(line.encode('utf-8'))
			else:
				raise Exception('invalid line: %r' % line)
		self.provides = {}
		if provides is not None:
			for k in provides.keys():
				self.provides[k] = True
		self.requires = {}
		if requires is not None:
			for k in requires.keys():
				self.requires[k] = True

		stripped_lines = strip_comments_from_lines(lines)

		for line in stripped_lines:
			# Careful with order, snippet may self-reference its own
			# defines in which case there's no outward dependency.
			# (This is not 100% because the order of require/provide
			# matters and this is not handled now.)
			#
			# Also, some snippets may #undef/#define another define but
			# they don't "provide" the define as such.  For example,
			# DUK_F_CLANG.h.in undefines DUK_F_GCC defines if clang is
			# detected: DUK_F_CLANG.h.in is considered to require
			# DUK_F_GCC but doesn't provide it.  Such redefinitions are
			# marked "/* redefine */" in the snippets.  They're best
			# avoided, of course.

			if autoscan_provides:
				m = re_line_provides.match(line)
				if m is not None and '/* redefine */' not in line:
					#print('PROVIDES: %r' % m.group(1))
					self.provides[m.group(1)] = True
			if autoscan_requires:
				matches = re.findall(re_line_requires, line)
				for m in matches:
					if self.provides.has_key(m):
						# Snippet provides it's own require; omit
						pass
					else:
						#print('REQUIRES: %r' % m)
						self.requires[m] = True

	def fromFile(cls, filename):
		lines = []
		with open(filename, 'rb') as f:
			for line in f:
				if line[-1] == '\n':
					line = line[:-1]
				lines.append(line)
		return Snippet(lines, autoscan_requires=True, autoscan_provides=True)
	fromFile = classmethod(fromFile)

	def merge(cls, snippets):
		ret = Snippet([], [], [])
		for s in snippets:
			ret.lines += s.lines
			for k in s.provides.keys():
				ret.provides[k] = True
			for k in s.requires.keys():
				ret.requires[k] = True
		return ret
	merge = classmethod(merge)

# Helper for building a text file from individual lines, injected files, etc.
class FileBuilder:
	lines = None
	base_dir = None
	use_cpp_warning = False

	def __init__(self, base_dir=None, use_cpp_warning=False):
		self.lines = []
		self.base_dir = base_dir
		self.use_cpp_warning = use_cpp_warning

	def line(self, line):
		self.lines.append(line)

	def empty(self):
		self.lines.append('')

	def rst_heading(self, title, char, doubled=False):
		if doubled:
			self.lines.append(char * len(title))
		self.lines.append(title)
		self.lines.append(char * len(title))

	def file_raw(self, f):
		for line in f:
			if line[-1] == '\n':
				line = line[:-1]
			self.lines.append(line)

	def file_relative(self, fn):
		with open(os.path.join(self.base_dir, fn), 'rb') as f:
			self.file_raw(f)

	def file_absolute(self, fn):
		with open(fn, 'rb') as f:
			self.file_raw(f)

	def snippet_relative(self, fn):
		sn = Snippet.fromFile(os.path.join(self.base_dir, fn))
		self.lines.append(sn)

	def snippet_absolute(fn):
		sn = Snippet.fromFile(fn)
		self.lines.append(sn)

	def cpp_error(self, msg):
		# XXX: assume no newlines etc
		self.lines.append('#error %s' % msg)

	def cpp_warning(self, msg):
		# XXX: assume no newlines etc
		# XXX: support compiler specific warning mechanisms
		if self.use_cpp_warning:
			# C preprocessor '#warning' is often supported
			self.lines.append('#warning %s' % msg)
		else:
			self.lines.append('/* WARNING: %s */' % msg)

	def cpp_warning_or_error(self, msg, is_error=True):
		if is_error:
			self.cpp_error(msg)
		else:
			self.cpp_warning(msg)

	def join(self):
		# FIXME: handle non-string self.lines elements
		return '\n'.join(self.lines)

	def upgrade_to_snippets(self):
		self.lines = upgrade_text_snippets(self.lines)

	def fill_dependencies_for_snippets(self, idx_deps):
		fill_dependencies_for_snippets(self.lines, idx_deps)

helper_snippets = None
def scan_snippets(dirname):
	global helper_snippets
	helper_snippets = []

	for fn in os.listdir(dirname):
		if (fn[0:6] != 'DUK_F_'):
			continue
		#print('Autoscanning snippet: %s' % fn)
		helper_snippets.append(Snippet.fromFile(os.path.join(dirname, fn)))

# Assume these provides come from outside.
assumed_provides = {
	'DUK_SINGLE_FILE': True,         # compiling Duktape from a single source file (duktape.c) version
	'DUK_COMPILING_DUKTAPE': True,   # compiling Duktape (not user application)
	'DUK_CONFIG_H_INCLUDED': True,   # artifact, include guard
}

def upgrade_text_snippets(snippets):
	# Convert strings to tiny snippets so that their requires/provides
	# are automatically scanned.
	ret = []
	for sn in snippets:
		if isinstance(sn, str):
			sn = Snippet([ sn ])
		elif isinstance(sn, unicode):
			sn = Snippet([ sn ])
		ret.append(sn)
	return ret

def fill_dependencies_for_snippets(snippets, idx_deps):
	# graph[A] = [ B, ... ] <-> B, ... provide something A requires.
	graph = {}
	snlist = []
	resolved = []   # for printing only

	def add(sn):
		if sn in snlist:
			return  # already present
		snlist.append(sn)

		to_add = []

		for k in sn.requires.keys():
			if assumed_provides.has_key(k):
				continue

			found = False
			for sn2 in snlist:
				if sn2.provides.has_key(k):
					if not graph.has_key(sn):
						graph[sn] = []
					graph[sn].append(sn2)
					found = True  # at least one other node provides 'k'

			if not found:
				#print 'Resolving %r' % k
				resolved.append(k)

				# Find a header snippet which provides the missing define.
				# Some DUK_F_xxx files provide multiple defines, so we don't
				# necessarily know the snippet filename here.

				sn_req = None
				for sn2 in helper_snippets:
					if sn2.provides.has_key(k):
						sn_req = sn2
						break
				if sn_req is None:
					raise Exception('cannot resolve missing require: %r' % k)

				# Snippet may have further unresolved provides; add recursively
				to_add.append(sn_req)

				if not graph.has_key(sn):
					graph[sn] = []
				graph[sn].append(sn_req)

		for sn in to_add:
			add(sn)

	# Add original snippets.  This fills in the required nodes
	# recursively.
	for sn in snippets:
		add(sn)

	# Figure out fill-ins by looking for snippets not in original
	# list and without any unserialized dependent nodes.
	handled = {}
	for sn in snippets:
		handled[sn] = True
	keepgoing = True
	while keepgoing:
		keepgoing = False
		for sn in snlist:
			if handled.has_key(sn):
				continue

			success = True
			for dep in graph.get(sn, []):
				if not handled.has_key(dep):
					success = False
			if success:
				snippets.insert(idx_deps, sn)
				idx_deps += 1
				snippets.insert(idx_deps, Snippet([ '' ]))
				idx_deps += 1
				handled[sn] = True
				keepgoing = True
				break

	# XXX: detect and handle loops cleanly
	for sn in snlist:
		if handled.has_key(sn):
			continue
		print('UNHANDLED KEY')
		print('PROVIDES: %r' % sn.provides)
		print('REQUIRES: %r' % sn.requires)
		print('\n'.join(sn.lines))

#	print(repr(graph))
#	print(repr(snlist))
	print 'Resolved helper defines: %r' % resolved

def serialize_snippet_list(snippets):
	ret = []

	emitted_provides = {}
	for k in assumed_provides.keys():
		emitted_provides[k] = True

	for sn in snippets:
		ret += sn.lines
		for k in sn.provides.keys():
			emitted_provides[k] = True
		for k in sn.requires.keys():
			if not emitted_provides.has_key(k):
				# FIXME: conditional warning, happens in some normal cases
				#print('WARNING: define %r required, not provided so far' % k)
				pass

	return '\n'.join(ret)

def remove_duplicate_newlines(x):
	ret = []
	empty = False
	for line in x.split('\n'):
		if line == '':
			if empty:
				pass
			else:
				ret.append(line)
			empty = True
		else:
			empty = False
			ret.append(line)
	return '\n'.join(ret)

# Metadata to scan from config files.
use_defs = None
use_defs_list = None
opt_defs = None
opt_defs_list = None
use_tags = None
use_tags_list = None
tags_meta = None
required_use_meta_keys = [
	'define',
	'introduced',
	'default',
	'tags',
	'description'
]
allowed_use_meta_keys = [
	'define',
	'related_feature_defines',
	'introduced',
	'deprecated',
	'removed',
	'unused',
	'requires',
	'conflicts',
	'related',
	'default',
	'tags',
	'description',
]
required_opt_meta_keys = [
	'define',
	'introduced',
	'tags',
	'description'
]
allowed_opt_meta_keys = [
	'define',
	'introduced',
	'deprecated',
	'removed',
	'unused',
	'requires',
	'conflicts',
	'related',
	'tags',
	'description'
]

# Preferred tag order for option documentation.
doc_tag_order = [
	'portability',
	'memory',
	'lowmemory',
	'ecmascript',
	'execution',
	'debugger',
	'debug',
	'development'
]

# Preferred tag order for generated C header files.
header_tag_order = doc_tag_order

def scan_use_defs(dirname):
	global use_defs, use_defs_list
	use_defs = {}
	use_defs_list = []

	for fn in os.listdir(dirname):
		root, ext = os.path.splitext(fn)
		if not root.startswith('DUK_USE_') or ext != '.yaml':
			continue
		with open(os.path.join(dirname, fn), 'rb') as f:
			doc = yaml.load(f)
			if doc.get('example', False):
				continue
			if doc.get('unimplemented', False):
				print('WARNING: unimplemented: %s' % fn)
				continue
			dockeys = doc.keys()
			for k in dockeys:
				if not k in allowed_use_meta_keys:
					print('WARNING: unknown key %s in metadata file %s' % (k, fn))
			for k in required_use_meta_keys:
				if not k in dockeys:
					print('WARNING: missing key %s in metadata file %s' % (k, fn))

			use_defs[doc['define']] = doc

	keys = use_defs.keys()
	keys.sort()
	for k in keys:
		use_defs_list.append(use_defs[k])

def scan_opt_defs(dirname):
	global opt_defs, opt_defs_list
	opt_defs = {}
	opt_defs_list = []

	for fn in os.listdir(dirname):
		root, ext = os.path.splitext(fn)
		if not root.startswith('DUK_OPT_') or ext != '.yaml':
			continue
		with open(os.path.join(dirname, fn), 'rb') as f:
			doc = yaml.load(f)
			if doc.get('example', False):
				continue
			if doc.get('unimplemented', False):
				print('WARNING: unimplemented: %s' % fn)
				continue
			dockeys = doc.keys()
			for k in dockeys:
				if not k in allowed_opt_meta_keys:
					print('WARNING: unknown key %s in metadata file %s' % (k, fn))
			for k in required_opt_meta_keys:
				if not k in dockeys:
					print('WARNING: missing key %s in metadata file %s' % (k, fn))

			opt_defs[doc['define']] = doc

	keys = opt_defs.keys()
	keys.sort()
	for k in keys:
		opt_defs_list.append(opt_defs[k])

def scan_use_tags():
	global use_tags, use_tags_list
	use_tags = {}

	for doc in use_defs_list:
		for tag in doc.get('tags', []):
			use_tags[tag] = True

	use_tags_list = use_tags.keys()
	use_tags_list.sort()

def scan_tags_meta(filename):
	global tags_meta

	with open(filename, 'rb') as f:
		tags_meta = yaml.load(f)

def get_tag_title(tag):
	meta = tags_meta.get(tag, None)
	if meta is None:
		return tag
	else:
		return meta.get('title', tag)

def get_tag_description(tag):
	meta = tags_meta.get(tag, None)
	if meta is None:
		return None
	else:
		return meta.get('description', None)

def get_tag_list_with_preferred_order(preferred):
	tags = []

	# Preferred tags first
	for tag in preferred:
		if tag not in tags:
			tags.append(tag)

	# Remaining tags in alphabetic order
	for tag in use_tags_list:
		if tag not in tags:
			tags.append(tag)

	#print('Effective tag order: %r' % tags)
	return tags

def rst_format(text):
	# FIXME: placeholder, need to decide on markup conventions for YAML files
	ret = []
	for para in text.split('\n'):
		if para == '':
			continue
		ret.append(para)
	return '\n\n'.join(ret)

def cint_encode(x):
	if not isinstance(x, (int, long)):
		raise Exception('invalid input: %r' % x)

	# XXX: unsigned constants?
	if x > 0x7fffffff or x < -0x80000000:
		return '%dLL' % x
	elif x > 0x7fff or x < -0x8000:
		return '%dL' % x
	else:
		return '%d' % x

def cstr_encode(x):
	if isinstance(x, unicode):
		x = x.encode('utf-8')
	if not isinstance(x, str):
		raise Exception('invalid input: %r' % x)

	res = '"'
	term = False
	has_terms = False
	for c in x:
		if term:
			# Avoid ambiguous hex escapes
			res += '" "'
			term = False
			has_terms = True
		o = ord(c)
		if o < 0x20 or o > 0x7e or c in '"\\':
			res += '\\x%02x' % o
			term = True
		else:
			res += c
	res += '"'

	if has_terms:
		res = '(' + res + ')'

	return res

# Shared helper to generate DUK_OPT_xxx and DUK_USE_xxx documentation.
# FIXME: unfinished placeholder
def generate_option_documentation(opts, opt_list=None, rst_title=None, include_default=False):
	ret = FileBuilder(use_cpp_warning=opts.use_cpp_warning)

	tags = get_tag_list_with_preferred_order(doc_tag_order)

	title = rst_title
	ret.rst_heading(title, '=', doubled=True)

	handled = {}

	for tag in tags:
		ret.empty()
		ret.rst_heading(get_tag_title(tag), '=')

		tag_desc = get_tag_description(tag)
		if tag_desc is not None:
			ret.empty()
			ret.line(rst_format(tag_desc))

		for doc in opt_list:
			if tag != doc['tags'][0]:  # sort under primary tag
				continue
			dname = doc['define']
			desc = doc.get('description', None)

			if handled.has_key(dname):
				raise Exception('define handled twice, should not happen: %r' % dname)
			handled[dname] = True

			ret.empty()
			ret.rst_heading(dname, '-')

			if desc is not None:
				ret.empty()
				ret.line(rst_format(desc))

			if include_default:
				ret.empty()
				ret.line('Default: ``' + str(doc['default']) + '``')  # FIXME: rst format

	for doc in opt_list:
		dname = doc['define']
		if not handled.has_key(dname):
			raise Exception('unhandled define (maybe missing from tags list?): %r' % dname)

	ret.empty()
	return ret.join()

def generate_feature_option_documentation(opts):
	return generate_option_documentation(opts, opt_list=opt_defs_list, rst_title='Duktape feature options', include_default=False)

def generate_config_option_documentation(opts):
	return generate_option_documentation(opts, opt_list=use_defs_list, rst_title='Duktape config options', include_default=True)

def get_forced_options(opts):
	# Forced options, last occurrence wins (allows a base config file to be
	# overridden by a more specific one).
	forced_opts = {}
	for val in opts.force_options_yaml:
		doc = yaml.load(StringIO.StringIO(val))
		for k in doc.keys():
			if use_defs.has_key(k):
				pass  # key is known
			else:
				print 'WARNING: option override key %s not defined in metadata, ignoring' % k
			forced_opts[k] = doc[k]  # shallow copy

	print 'Overrides: %s' % json.dumps(forced_opts)

	return forced_opts

# Emit a default #define / #undef for an option based on
# a config option metadata node (parsed YAML doc).
# FIXME: argument names
def emit_default_from_config_meta(ret, doc, forced_opts, undef_done):
	defname = doc['define']
	defval = forced_opts.get(defname, doc['default'])

	if defval == True:
		ret.line('#define ' + defname)
	elif defval  == False:
		if not undef_done:
			ret.line('#undef ' + defname)
	elif isinstance(defval, (int, long)):
		# integer value
		ret.line('#define ' + defname + ' ' + cint_encode(defval))
	elif isinstance(defval, (str, unicode)):
		# verbatim value
		ret.line('#define ' + defname + ' ' + defval)
	elif isinstance(defval, dict):
		if defval.has_key('verbatim'):
			# verbatim text for the entire line
			ret.line(defval['verbatim'])
		elif defval.has_key('string'):
			# C string value
			ret.line('#define ' + defname + ' ' + cstr_encode(defval['string']))
		else:
			raise Exception('unsupported value for option %s: %r' % (defname, defval))
	else:
		raise Exception('unsupported value for option %s: %r' % (defname, defval))

# Add a header snippet for detecting presence of DUK_OPT_xxx feature
# options which will be removed in Duktape 2.x.
def add_legacy_feature_option_checks(opts, ret):
	ret.empty()
	ret.line('/*')
	ret.line(' *  Checks for legacy feature options (DUK_OPT_xxx)')
	ret.line(' */')

	defs = []
	for doc in opt_defs_list:
		if doc['define'] not in defs:
			defs.append(doc['define'])
	for doc in use_defs_list:
		for dname in doc.get('related_feature_defines', []):
			if dname not in defs:
				defs.append(dname)
	defs.sort()

	for optname in defs:
		suggested = []
		for doc in use_defs_list:
			if optname in doc.get('related_feature_defines', []):
				suggested.append(doc['define'])
		ret.empty()
		ret.line('#if defined(%s)' % optname)
		if len(suggested) > 0:
			ret.cpp_warning_or_error('unsupported legacy feature option %s used, consider options: %s' % (optname, ', '.join(suggested)), opts.sanity_strict)
		else:
			ret.cpp_warning_or_error('unsupported legacy feature option %s used' % optname, opts.sanity_strict)
		ret.line('#endif')

	ret.empty()

# Add a header snippet for checking consistency of DUK_USE_xxx config
# options, e.g. inconsistent options, invalid option values.
def add_config_option_checks(opts, ret):
	ret.empty()
	ret.line('/*')
	ret.line(' *  Checks for config option consistency (DUK_USE_xxx)')
	ret.line(' */')

	defs = []
	for doc in use_defs_list:
		if doc['define'] not in defs:
			defs.append(doc['define'])
	defs.sort()

	for optname in defs:
		doc = use_defs[optname]
		dname = doc['define']

		# XXX: more checks

		if doc.get('removed', None) is not None:
			ret.empty()
			ret.line('#if defined(%s)' % dname)
			ret.cpp_warning_or_error('unsupported config option used (option has been removed): %s' % dname, opts.sanity_strict)
			ret.line('#endif')
		elif doc.get('deprecated', None) is not None:
			ret.empty()
			ret.line('#if defined(%s)' % dname)
			ret.cpp_warning_or_error('unsupported config option used (option has been deprecated): %s' % dname, opts.sanity_strict)
			ret.line('#endif')

		for req in doc.get('requires', []):
			ret.empty()
			ret.line('#if defined(%s) && !defined(%s)' % (dname, req))
			ret.cpp_warning_or_error('config option %s requires option %s (which is missing)' % (dname, req), opts.sanity_strict)
			ret.line('#endif')

		for req in doc.get('conflicts', []):
			ret.empty()
			ret.line('#if defined(%s) && defined(%s)' % (dname, req))
			ret.cpp_warning_or_error('config option %s conflicts with option %s (which is also defined)' % (dname, req), opts.sanity_strict)
			ret.line('#endif')

	ret.empty()

# Add a header snippet for providing a __OVERRIDE_DEFINES__ section.
def add_override_defines_section(opts, ret):
	ret.empty()
	ret.line('/*')
	ret.line(' *  You may add overriding #define/#undef directives below for')
	ret.line(' *  customization.  You of course cannot un-#include or un-typedef')
	ret.line(' *  anything; these require direct changes above.')
	ret.line(' */')
	ret.empty()
	ret.line('/* __OVERRIDE_DEFINES__ */')
	ret.empty()

# Generate the default duk_config.h which provides automatic detection
# of platform, compiler, architecture, and features for major platforms.
# Initially this header will be Duktape 1.2 compatible and support
# DUK_OPT_xxx feature options; later the feature options will be removed
# and users can just override DUK_USE_xxx flags by modifying the header
# or generating a new one using genconfig.
def generate_autodetect_duk_config_header(opts, meta_dir):
	ret = FileBuilder(base_dir=os.path.join(meta_dir, 'header-snippets'), \
	                  use_cpp_warning=opts.use_cpp_warning)

	forced_opts = get_forced_options(opts)

	# XXX: initial version is Duktape 1.2 duk_features.h.in built
	# from pieces; rework later

	# XXX: add place for Git describe like in duktape.c and duktape.h

	ret.file_relative('comment_prologue.h.in')
	ret.empty()

	ret.line('#ifndef DUK_CONFIG_H_INCLUDED')
	ret.line('#define DUK_CONFIG_H_INCLUDED')
	ret.empty()

	# Compiler features, processor/architecture, OS, compiler
	ret.file_relative('compiler_features.h.in')
	ret.empty()
	ret.file_relative('rdtsc.h.in')  # XXX: move downwards
	ret.empty()
	ret.file_relative('platform1.h.in')
	ret.empty()

	# Feature selection, system include, Date provider
	# Most #include statements are here
	# FIXME: DUK_USE_xxx flags for Date providers, major source of portability problems
	ret.file_relative('platform2.h.in')
	ret.empty()

	# FIXME: DUK_F_ULL_CONSTS
	ret.file_relative('ullconsts.h.in')
	ret.empty()

	# FIXME: __uclibc__ needs stdlib.h, but it really is the only exception
	ret.file_relative('libc.h.in')
	ret.empty()

	# Number types
	# FIXME: some stuff is derived
	# FIXME: standard vs. hack types distinction
	# FIXME: standard types are NOT always available, even with C99
	# #include <inttypes.h>, conditional
	ret.file_relative('types1.h.in')
	ret.line('#if defined(DUK_F_HAVE_INTTYPES)')
	ret.line('/* C99 or compatible */')
	ret.empty()
	ret.file_relative('types_c99.h.in')
	ret.empty()
	ret.line('#else  /* C99 types */')
	ret.empty()
	ret.file_relative('types_legacy.h.in')
	ret.empty()
	ret.line('#endif  /* C99 types */')
	ret.empty()
	ret.file_relative('types2.h.in')
	ret.empty()

	# DUK_USE_64BIT_OPS
	ret.file_relative('64bitops.h.in')
	ret.empty()

	# DUK_USE_UNALIGNED_ACCESSES_POSSIBLE
	# DUK_USE_ALIGN_BY
	# DUK_USE_PACK_MSVC_PRAGMA
	# DUK_USE_PACK_GCC_ATTR
	# DUK_USE_PACK_CLANG_ATTR
	# DUK_USE_PACK_DUMMY_MEMBER
	# DUK_USE_HASHBYTES_UNALIGNED_U32_ACCESS
	ret.file_relative('alignment.h.in')
	ret.empty()

	# DUK_USE_HOBJECT_LAYOUT_1
	# DUK_USE_HOBJECT_LAYOUT_2
	# DUK_USE_HOBJECT_LAYOUT_3
	# FIXME: depend on DUK_USE_UNALIGNED_ACCESSES_POSSIBLE, DUK_USE_ALIGN_BY
	ret.file_relative('object_layout.h.in')
	ret.empty()

	# DUK_USE_BYTEORDER_FORCED
	# DUK_USE_INTEGER_LE
	# DUK_USE_INTEGER_BE
	# DUK_USE_DOUBLE_LE
	# DUK_USE_DOUBLE_ME
	# DUK_USE_DOUBLE_BE
	ret.file_relative('byteorder.h.in')
	ret.empty()

	# DUK_USE_PACKED_TVAL_POSSIBLE
	ret.file_relative('packed_tval.h.in')
	ret.empty()

	# FIXME: no use flags... sanity?
	ret.file_relative('reject_fast_math.h.in')
	ret.empty()

	# DUK_USE_COMPUTED_INFINITY
	# DUK_USE_COMPUTED_NAN
	ret.file_relative('double_const.h.in')
	ret.empty()

	# FIXME: math replacements, NetBSD workaround, paranoid math, paranoid Date computation
	# DUK_USE_REPL_FPCLASSIFY
	# DUK_USE_REPL_SIGNBIT
	# DUK_USE_REPL_ISFINITE
	# DUK_USE_REPL_ISNAN
	# DUK_USE_REPL_ISINF
	# DUK_USE_MATH_FMIN
	# DUK_USE_MATH_FMAX
	# DUK_USE_MATH_ROUND
	# DUK_USE_POW_NETBSD_WORKAROUND
	# DUK_USE_PARANOID_MATH -- FIXME?
	ret.file_relative('repl_math.h.in')
	ret.empty()

	# DUK_USE_PARANOID_DATE_COMPUTATION
	ret.file_relative('paranoid_date.h.in')
	ret.empty()

	ret.file_relative('repl_ansi.h.in')
	ret.empty()

	# DUK_USE_AVOID_PLATFORM_FUNCPTRS
	ret.file_relative('platform_funcptr.h.in')
	ret.empty()

	# FIXME: generic
	ret.file_relative('stringify.h.in')
	ret.empty()
	ret.file_relative('segfault.h.in')
	ret.empty()
	ret.file_relative('unreferenced.h.in')
	ret.empty()
	ret.file_relative('noreturn.h.in')
	ret.empty()
	ret.file_relative('unreachable.h.in')
	ret.empty()
	ret.file_relative('likely.h.in')
	ret.empty()
	ret.file_relative('noinline.h.in')
	ret.empty()
	ret.file_relative('visibility.h.in')
	ret.empty()
	ret.file_relative('file_line_func.h.in')
	ret.empty()
	ret.file_relative('byteswap.h.in')
	ret.empty()

	ret.file_relative('arch_string.h.in')
	ret.empty()
	ret.file_relative('os_string.h.in')
	ret.empty()
	ret.file_relative('compiler_string.h.in')
	ret.empty()
	ret.file_relative('longjmp.h.in')
	ret.empty()
	ret.file_relative('target_info.h.in')
	ret.empty()

	ret.file_relative('unsorted_flags.h.in')
	ret.empty()
	ret.file_relative('date_provider.h.in')
	ret.empty()

	ret.file_relative('user_declare.h.in')
	ret.empty()

	# Emit forced options.  If a corresponding option is already defined
	# by a snippet above, #undef it first.

	tmp = Snippet(ret.join().split('\n'))
	first_forced = True
	for doc in use_defs_list:
		defname = doc['define']

		if doc.get('removed', None) is not None and opts.omit_removed_config_options:
			continue
		if doc.get('deprecated', None) is not None and opts.omit_deprecated_config_options:
			continue
		if doc.get('unused', False) == True and opts.omit_unused_config_options:
			continue
		if not forced_opts.has_key(defname):
			continue

		if not doc.has_key('default'):
			raise Exception('config option %s is missing default value' % defname)

		if first_forced:
			ret.line('/*')
			ret.line(' *  Forced options')
			ret.line(' */')
			ret.empty()
			first_forced = False

		undef_done = False
		if tmp.provides.has_key(defname):
			ret.line('#undef ' + defname)
			undef_done = True

		emit_default_from_config_meta(ret, doc, forced_opts, True)

	ret.empty()

	# If manually-edited snippets don't #define or #undef a certain
	# config option, emit a default value here with a warning.  It's
	# easy to forget adding a manual snippet when adding a new config
	# metadata file.

	# FIXME:
	# NEED: DUK_USE_DATE_PRS_GETDATE
	# NEED: DUK_USE_INTEGER_ME
	# NEED: DUK_USE_JSON_STRINGIFY_FASTPATH

	tmp = Snippet(ret.join().split('\n'))
	need = {}
	for doc in use_defs_list:
		if doc.get('removed', None) is not None:
			continue
		need[doc['define']] = True
	for k in tmp.provides.keys():
		if need.has_key(k):
			del need[k]
	need_keys = sorted(need.keys())

	if len(need_keys) > 0:
		ret.line('/*')
		ret.line(' *  Autogenerated defaults')
		ret.line(' */')
		ret.empty()

		for k in need_keys:
			print('WARNING: config option %s not covered by manual snippets, emitting default automatically' % k)
			emit_default_from_config_meta(ret, use_defs[k], {}, False)

		ret.empty()

	ret.file_relative('custom_header.h.in')
	ret.empty()

	if len(opts.fixup_header_lines) > 0:
		ret.line('/*')
		ret.line(' *  Fixups')
		ret.line(' */')
		ret.empty()
		for line in opts.fixup_header_lines:
			ret.line(line)
		ret.empty()

	add_override_defines_section(opts, ret)

	# FIXME: use autogenerated sanity instead of sanity.h.in

	ret.file_relative('sanity.h.in')
	ret.empty()

	if opts.emit_legacy_feature_check:
		# FIXME: this doesn't really make sense for the autodetect header yet
		add_legacy_feature_option_checks(opts, ret)
	if opts.emit_config_sanity_check:
		add_config_option_checks(opts, ret)

	ret.line('#endif  /* DUK_CONFIG_H_INCLUDED */')
	ret.empty()  # for trailing newline
	return remove_duplicate_newlines(ret.join())

# Generate a barebones duk_config.h header for a specific platform, architecture,
# and compiler.  The header won't do automatic feature detection and does not
# support DUK_OPT_xxx feature options (which will be removed in Duktape 2.x).
# Users can then modify this barebones header for very exotic platforms and manage
# the needed changes either as a YAML file or by appending a fixup header snippet.
def generate_barebones_duk_config_header(opts, meta_dir):
	ret = FileBuilder(base_dir=os.path.join(meta_dir, 'header-snippets'), \
	                  use_cpp_warning=opts.use_cpp_warning)

	# XXX: add place for Git describe like in duktape.c and duktape.h
	# XXX: provide more defines from YAML config files so that such defines
	#      can be overridden more conveniently

	forced_opts = get_forced_options(opts)

	ret.line('/*')
	ret.line(' *  duk_config.h generated by genconfig.py for:')
	ret.line(' *      platform: %s' % opts.platform)
	ret.line(' *      compiler: %s' % opts.compiler)
	ret.line(' *      architecture: %s' % opts.architecture)
	ret.line(' */')
	ret.empty()
	ret.line('#ifndef DUK_CONFIG_H_INCLUDED')
	ret.line('#define DUK_CONFIG_H_INCLUDED')

	ret.empty()
	ret.line('/*')
	ret.line(' *  Intermediate helper defines')
	ret.line(' */')
	ret.empty()

	idx_deps = len(ret.lines)  # position where to emit dependencies

	ret.empty()
	ret.line('/*')
	ret.line(' *  Platform headers and typedefs')
	ret.line(' */')
	ret.empty()

	if opts.platform is None:
		raise Exception('no platform specified')

	fn = 'platform_%s.h.in' % opts.platform
	ret.snippet_relative(fn)
	ret.empty()
	ret.snippet_relative('types_c99.h.in')  # FIXME: C99 typedefs forced for now
	ret.snippet_relative('types2.h.in')     # FIXME: boilerplate type stuff

	ret.empty()
	ret.line('/*')
	ret.line(' *  Platform features')
	ret.line(' */')
	ret.empty()

	# double constants
	# replacement functions
	# inherit definitions (like '#define DUK_FFLUSH fflush') from a generic
	#   set of defaults, allow platform configs to override

	ret.snippet_relative('platform_generic.h.in')

	ret.empty()
	ret.line('/*')
	ret.line(' *  Compiler features')
	ret.line(' */')
	ret.empty()

	if opts.compiler is None:
		raise Exception('no compiler specified')

	fn = 'compiler_%s.h.in' % opts.compiler
	ret.snippet_relative(fn)

	# noreturn, vacopy, etc
	# visibility attributes

	ret.empty()
	ret.line('/*')
	ret.line(' *  Architecture features')
	ret.line(' */')
	ret.empty()

	if opts.architecture is None:
		raise Exception('no architecture specified')

	fn = 'architecture_%s.h.in' % opts.architecture
	ret.snippet_relative(fn)

	ret.empty()
	ret.line('/*')
	ret.line(' *  Config options')
	ret.line(' */')
	ret.empty()

	# if defines by compiler/platform etc, only define if override exists
	# otherwise fallback to default
	# ordering based on tags; empty lines in-between

	tags = get_tag_list_with_preferred_order(header_tag_order)

	handled = {}

	# Mark all defines 'provided' by the snippets so far as handled.
	# For example, if the system header provides a DUK_USE_OS_STRING,
	# we won't emit it again below with its default value (but will
	# emit an override value if specified).

	for sn in upgrade_text_snippets(ret.lines):
		for k in sn.provides.keys():
			handled[k] = True

	for tag in tags:
		ret.line('/* ' + get_tag_title(tag) + ' */')

		for doc in use_defs_list:
			defname = doc['define']

			if doc.get('removed', None) is not None and opts.omit_removed_config_options:
				continue
			if doc.get('deprecated', None) is not None and opts.omit_deprecated_config_options:
				continue
			if doc.get('unused', False) == True and opts.omit_unused_config_options:
				continue

			if tag != doc['tags'][0]:  # sort under primary tag
				continue

			if not doc.has_key('default'):
				raise Exception('config option %s is missing default value' % defname)

			undef_done = False

			if handled.has_key(defname):
				defval = forced_opts.get(defname, None)
				if defval is None:
					ret.line('/* %s already emitted above */' % defname)
					continue

				# Define already emitted by snippets above but
				# an explicit override wants to redefine it.
				# Undef first and then use shared handler to
				# setup the forced value.
				ret.line('#undef ' + defname)
				undef_done = True

			# FIXME: macro args; DUK_USE_USER_DECLARE vs. DUK_USE_USER_DECLARE()
			#        vs. DUK_USE_USER_DECLARE(x,y)

			handled[defname] = True
			emit_default_from_config_meta(ret, doc, forced_opts, undef_done)

		ret.empty()

	ret.empty()
	ret.line('/*')
	ret.line(' *  Date provider')
	ret.line(' */')
	ret.empty()

	# FIXME: this should probably happen inside Duktape? It's very much
	# boilerplate here.
	ret.snippet_relative('date_provider.h.in')

	if len(opts.fixup_header_lines) > 0:
		ret.empty()
		ret.line('/*')
		ret.line(' *  Fixups')
		ret.line(' */')
		ret.empty()
		for line in opts.fixup_header_lines:
			ret.line(line)

	add_override_defines_section(opts, ret)

	ret.upgrade_to_snippets()
	ret.fill_dependencies_for_snippets(idx_deps)

	# FIXME: ensure no define is unhandled at the end

	# Check for presence of legacy feature options (DUK_OPT_xxx),
	# and consistency of final DUK_USE_xxx options.
	#
	# These could also be emitted into Duktape source code, but it's
	# probably better that the checks can be easily disabled from
	# duk_config.h.

	if opts.emit_legacy_feature_check:
		add_legacy_feature_option_checks(opts, ret)
	if opts.emit_config_sanity_check:
		add_config_option_checks(opts, ret)

	ret.line('#endif  /* DUK_CONFIG_H_INCLUDED */')
	ret.empty()  # for trailing newline

	ret.upgrade_to_snippets()
	return remove_duplicate_newlines(serialize_snippet_list(ret.lines))  # XXX: refactor into FileBuilder

def main():
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

	commands = [
		'autodetect-header',
		'barebones-header',
		'feature-documentation',
		'config-documentation'
	]
	parser = optparse.OptionParser(
		usage='Usage: %prog [options] COMMAND',
		description='Generate a duk_config.h or config option documentation based on config metadata.',
		epilog='COMMAND can be one of: ' + ', '.join(commands) + '.'
	)
	parser.add_option('--metadata', dest='metadata', default=None, help='metadata directory or metadata tar.gz file')
	parser.add_option('--output', dest='output', default=None, help='output filename for C header or RST documentation file')
	parser.add_option('--platform', dest='platform', default=None, help='platform (for "barebones-header" command)')
	parser.add_option('--compiler', dest='compiler', default=None, help='compiler (for "barebones-header" command)')
	parser.add_option('--architecture', dest='architecture', default=None, help='architecture (for "barebones-header" command)')
	parser.add_option('--dll', dest='dll', action='store_true', default=False, help='dll build of Duktape, affects symbol visibility macros especially on Windows')  # FIXME: unimplemented
	parser.add_option('--emit-legacy-feature-check', dest='emit_legacy_feature_check', action='store_true', default=False, help='emit preprocessor checks to reject legacy feature options (DUK_OPT_xxx)')
	parser.add_option('--emit-config-sanity-check', dest='emit_config_sanity_check', action='store_true', default=False, help='emit preprocessor checks for config option consistency (DUK_OPT_xxx)')
	parser.add_option('--omit-removed-config-options', dest='omit_removed_config_options', action='store_true', default=False, help='omit removed config options from generated headers')
	parser.add_option('--omit-deprecated-config-options', dest='omit_deprecated_config_options', action='store_true', default=False, help='omit deprecated config options from generated headers')
	parser.add_option('--omit-unused-config-options', dest='omit_unused_config_options', action='store_true', default=False, help='omit unused config options from generated headers')
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

	meta_dir = opts.metadata
	if opts.metadata is None:
		if os.path.isfile(os.path.join('.', 'genconfig_metadata.tar.gz')):
			opts.metadata = 'genconfig_metadata.tar.gz'
		elif os.path.isdir(os.path.join('.', 'config-options')):
			opts.metadata = '.'

	if opts.metadata is not None and os.path.isdir(opts.metadata):
		meta_dir = opts.metadata
		print 'Using metadata directory: %r' % meta_dir
	elif opts.metadata is not None and os.path.isfile(opts.metadata) and tarfile.is_tarfile(opts.metadata):
		meta_dir = get_auto_delete_tempdir()
		tar = tarfile.open(name=opts.metadata, mode='r:*')
		tar.extractall(path=meta_dir)
		print 'Using metadata tar file %r, unpacked to directory: %r' % (opts.metadata, meta_dir)
	else:
		raise Exception('metadata source must be a directory or a tar.gz file')

	scan_snippets(os.path.join(meta_dir, 'header-snippets'))
	scan_use_defs(os.path.join(meta_dir, 'config-options'))
	scan_opt_defs(os.path.join(meta_dir, 'feature-options'))
	scan_use_tags()
	scan_tags_meta(os.path.join(meta_dir, 'tags.yaml'))
	print('Scanned %d DUK_OPT_xxx, %d DUK_USE_XXX, %d helper snippets' % \
		(len(opt_defs.keys()), len(use_defs.keys()), len(helper_snippets)))
	#print('Tags: %r' % use_tags_list)

	if len(args) == 0:
		raise Exception('missing command')
	cmd = args[0]

	if cmd == 'autodetect-header':
		# Generate a duk_config.h similar to Duktape 1.2 feature detection.
		result = generate_autodetect_duk_config_header(opts, meta_dir)
		with open(opts.output, 'wb') as f:
			f.write(result)
	elif cmd == 'barebones-header':
		# Generate a duk_config.h with default options for a specific platform,
		# compiler, and architecture.
		result = generate_barebones_duk_config_header(opts, meta_dir)
		with open(opts.output, 'wb') as f:
			f.write(result)
	elif cmd == 'feature-documentation':
		result = generate_feature_option_documentation(opts)
		with open(opts.output, 'wb') as f:
			f.write(result)
	elif cmd == 'config-documentation':
		result = generate_config_option_documentation(opts)
		with open(opts.output, 'wb') as f:
			f.write(result)
	else:
		raise Exception('invalid command: %r' % cmd)

if __name__ == '__main__':
	main()
