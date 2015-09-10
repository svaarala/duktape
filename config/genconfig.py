#!/usr/bin/python
#
#  Process Duktape option metadata and produce various useful outputs:
#
#    - duk_config.h matching Duktape 1.x feature option model (DUK_OPT_xxx)
#    - duk_config.h for a selected platform, compiler, forced options, etc.
#    - option documentation for Duktape 1.x feature options (DUK_OPT_xxx)
#    - option documentation for Duktape 1.x/2.x config options (DUK_USE_xxx)
#
#  Genconfig tries to build all outputs based on modular metadata, so that
#  managing a large number of config options (which is hard to avoid given
#  the wide range of targets Duktape supports) remains maintainable.
#
#  Genconfig does *not* try to support all exotic platforms out there.
#  Instead, the goal is to allow the metadata to be extended, or to provide
#  a reasonable starting point for manual duk_config.h tweaking.
#
#  NOTE: For Duktape 1.3 release the main goal is to autogenerate a Duktape
#  1.2 compatible "autodetect" header from snippets.  Other outputs are still
#  experimental.
#

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

#
#  Globals holding scanned metadata, helper snippets, etc
#

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
	'feature_enables',
	'feature_disables',
	'feature_snippet',
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

# Helper headers snippets.
helper_snippets = None

# Assume these provides come from outside.
assumed_provides = {
	'DUK_SINGLE_FILE': True,         # compiling Duktape from a single source file (duktape.c) version
	'DUK_COMPILING_DUKTAPE': True,   # compiling Duktape (not user application)
	'DUK_CONFIG_H_INCLUDED': True,   # artifact, include guard
}

# Platform files must provide at least these (additional checks
# in validate_platform_file()).
platform_required_provides = [
	'DUK_USE_OS_STRING',
	'DUK_SETJMP', 'DUK_LONGJMP',
]

# Architecture files must provide at least these (additional checks
# in validate_architecture_file()).
architecture_required_provides = [
	'DUK_USE_ARCH_STRING',
	'DUK_USE_ALIGN_BY', 'DUK_USE_UNALIGNED_ACCESSES_POSSIBLE', 'DUK_USE_HASHBYTES_UNALIGNED_U32_ACCESS',
	'DUK_USE_PACKED_TVAL', 'DUK_USE_PACKED_TVAL_POSSIBLE'
]

# Compiler files must provide at least these (additional checks
# in validate_compiler_file()).
compiler_required_provides = [
	# XXX: incomplete, maybe a generic fill-in for missing stuff because
	# there's quite a lot of required compiler defines.

	'DUK_USE_COMPILER_STRING',

	'DUK_EXTERNAL_DECL', 'DUK_EXTERNAL',
	'DUK_INTERNAL_DECL', 'DUK_INTERNAL',
	'DUK_LOCAL_DECL', 'DUK_LOCAL',

	'DUK_FILE_MACRO', 'DUK_LINE_MACRO', 'DUK_FUNC_MACRO'
]

#
#  Miscellaneous helpers
#

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
re_line_requires = re.compile(r'(DUK_[A-Z0-9_]+)')  # uppercase only, don't match DUK_USE_xxx for example
class Snippet:
	lines = None     # lines of text and/or snippets
	provides = None  # map from define to 'True' for now
	requires = None  # map from define to 'True' for now

	def __init__(self, lines, provides=None, requires=None, autoscan_requires=True, autoscan_provides=True):
		self.lines = []
		if not isinstance(lines, list):
			raise Exception('Snippet constructor must be a list (not e.g. a string): %s' % repr(lines))
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
		# for line in stripped_lines: print(line)

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
				if m is not None and '/* redefine */' not in line and \
					len(m.group(1)) > 0 and m.group(1)[-1] != '_':
					# Don't allow e.g. DUK_USE_ which results from matching DUK_USE_xxx
					#print('PROVIDES: %r' % m.group(1))
					self.provides[m.group(1)] = True
			if autoscan_requires:
				matches = re.findall(re_line_requires, line)
				for m in matches:
					if len(m) > 0 and m[-1] == '_':
						# Don't allow e.g. DUK_USE_ which results from matching DUK_USE_xxx
						pass
					elif m[:7] == 'DUK_OPT':
						# DUK_OPT_xxx always come from outside
						pass
					elif m[:7] == 'DUK_USE':
						# DUK_USE_xxx are internal and they should not be 'requirements'
						pass
					elif self.provides.has_key(m):
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
# Inserted values are converted to Snippets so that their provides/requires
# information can be tracked.  When non-C outputs are created, these will be
# bogus but ignored.
class FileBuilder:
	vals = None  # snippet list
	base_dir = None
	use_cpp_warning = False

	def __init__(self, base_dir=None, use_cpp_warning=False):
		self.vals = []
		self.base_dir = base_dir
		self.use_cpp_warning = use_cpp_warning

	def line(self, line):
		self.vals.append(Snippet([ line ]))

	def lines(self, lines):
		if len(lines) > 0 and lines[-1] == '\n':
			lines = lines[:-1]  # strip last newline to avoid empty line
		self.vals.append(Snippet(lines.split('\n')))

	def empty(self):
		self.vals.append(Snippet([ '' ]))

	def rst_heading(self, title, char, doubled=False):
		tmp = []
		if doubled:
			tmp.append(char * len(title))
		tmp.append(title)
		tmp.append(char * len(title))
		self.vals.append(Snippet(tmp))

	def snippet_relative(self, fn):
		sn = Snippet.fromFile(os.path.join(self.base_dir, fn))
		self.vals.append(sn)

	def snippet_absolute(fn):
		sn = Snippet.fromFile(fn)
		self.vals.append(sn)

	def cpp_error(self, msg):
		# XXX: assume no newlines etc
		self.vals.append(Snippet([ '#error %s' % msg ]))

	def cpp_warning(self, msg):
		# XXX: assume no newlines etc
		# XXX: support compiler specific warning mechanisms
		if self.use_cpp_warning:
			# C preprocessor '#warning' is often supported
			self.vals.append(Snippet([ '#warning %s' % msg ]))
		else:
			self.vals.append(Snippet([ '/* WARNING: %s */' % msg ]))

	def cpp_warning_or_error(self, msg, is_error=True):
		if is_error:
			self.cpp_error(msg)
		else:
			self.cpp_warning(msg)

	def chdr_block_heading(self, msg):
		lines = []
		lines.append('')
		lines.append('/*')
		lines.append(' *  ' + msg)
		lines.append(' */')
		lines.append('')
		self.vals.append(Snippet(lines))

	def join(self):
		tmp = []
		for line in self.vals:
			if not isinstance(line, object):
				raise Exception('self.vals must be all snippets')
			for x in line.lines:  # x is a Snippet
				tmp.append(x)
		return '\n'.join(tmp)

	def fill_dependencies_for_snippets(self, idx_deps):
		fill_dependencies_for_snippets(self.vals, idx_deps)

# Insert missing define dependencies into index 'idx_deps' repeatedly
# until no unsatisfied dependencies exist.  This is used to pull in
# the required DUK_F_xxx helper defines without pulling them all in.
# The resolution mechanism also ensures dependencies are pulled in the
# correct order, i.e. DUK_F_xxx helpers may depend on each other (as
# long as there are no circular dependencies).
#
# XXX: this can be simplified a lot
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
					print(repr(sn.lines))
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
				# XXX: conditional warning, happens in some normal cases
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

def scan_snippets(dirname):
	global helper_snippets
	helper_snippets = []

	for fn in os.listdir(dirname):
		if (fn[0:6] != 'DUK_F_'):
			continue
		#print('Autoscanning snippet: %s' % fn)
		helper_snippets.append(Snippet.fromFile(os.path.join(dirname, fn)))

def validate_platform_file(filename):
	sn = Snippet.fromFile(filename)

	# XXX: move required provides/defines into metadata only
	for req in platform_required_provides:
		if req not in sn.provides:
			raise Exception('Platform %s is missing %s' % (filename, req))

	if not ('DUK_USE_SETJMP' in sn.provides or 'DUK_USE_UNDERSCORE_SETJMP' in sn.provides or
	        'DUK_USE_SIGSETJMP' in sn.provides):
		raise Exception('Platform %s is missing a setjmp provider' % filename)

def validate_architecture_file(filename):
	sn = Snippet.fromFile(filename)

	# XXX: move required provides/defines into metadata only
	for req in architecture_required_provides:
		if req not in sn.provides:
			raise Exception('Architecture %s is missing %s' % (filename, req))

def validate_compiler_file(filename):
	sn = Snippet.fromFile(filename)

	# XXX: move required provides/defines into metadata only
	for req in compiler_required_provides:
		if req not in sn.provides:
			raise Exception('Architecture %s is missing %s' % (filename, req))

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
	# XXX: placeholder, need to decide on markup conventions for YAML files
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

#
#  Autogeneration of option documentation
#

# Shared helper to generate DUK_OPT_xxx and DUK_USE_xxx documentation.
# XXX: unfinished placeholder
def generate_option_documentation(opts, opt_list=None, rst_title=None, include_default=False):
	ret = FileBuilder(use_cpp_warning=opts.use_cpp_warning)

	tags = get_tag_list_with_preferred_order(doc_tag_order)

	title = rst_title
	ret.rst_heading(title, '=', doubled=True)

	handled = {}

	for tag in tags:
		first = True

		for doc in opt_list:
			if tag != doc['tags'][0]:  # sort under primary tag
				continue
			dname = doc['define']
			desc = doc.get('description', None)

			if handled.has_key(dname):
				raise Exception('define handled twice, should not happen: %r' % dname)
			handled[dname] = True

			if first:  # emit tag heading only if there are subsections
				ret.empty()
				ret.rst_heading(get_tag_title(tag), '=')

				tag_desc = get_tag_description(tag)
				if tag_desc is not None:
					ret.empty()
					ret.line(rst_format(tag_desc))
				first = False

			ret.empty()
			ret.rst_heading(dname, '-')

			if desc is not None:
				ret.empty()
				ret.line(rst_format(desc))

			if include_default:
				ret.empty()
				ret.line('Default: ``' + str(doc['default']) + '``')  # XXX: rst or other format

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

#
#  Helpers for duk_config.h generation
#

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
def emit_default_from_config_meta(ret, doc, forced_opts, undef_done):
	defname = doc['define']
	defval = forced_opts.get(defname, doc['default'])

	if defval == True:
		ret.line('#define ' + defname)
	elif defval == False:
		if not undef_done:
			ret.line('#undef ' + defname)
		else:
			# Default value is false, and caller has emitted
			# an unconditional #undef, so don't emit a duplicate
			pass
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
	ret.chdr_block_heading('Checks for legacy feature options (DUK_OPT_xxx)')

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
	ret.chdr_block_heading('Checks for config option consistency (DUK_USE_xxx)')

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

# Add automatic DUK_OPT_XXX and DUK_OPT_NO_XXX handling for backwards
# compatibility with Duktape 1.2 and before.
def add_feature_option_handling(opts, ret, forced_opts):
	ret.chdr_block_heading('Feature option handling')

	for doc in use_defs_list:
		# If a related feature option exists, it can be used to force
		# enable/disable the target feature.  If neither feature option
		# (DUK_OPT_xxx or DUK_OPT_NO_xxx) is given, revert to default.

		config_define = doc['define']

		feature_define = None
		feature_no_define = None
		inverted = False
		if doc.has_key('feature_enables'):
			feature_define = doc['feature_enables']
		elif doc.has_key('feature_disables'):
			feature_define = doc['feature_disables']
			inverted = True
		else:
			pass

		if feature_define is not None:
			feature_no_define = 'DUK_OPT_NO_' + feature_define[8:]
			ret.line('#if defined(%s)' % feature_define)
			if inverted:
				ret.line('#undef %s' % config_define)
			else:
				ret.line('#define %s' % config_define)
			ret.line('#elif defined(%s)' % feature_no_define)
			if inverted:
				ret.line('#define %s' % config_define)
			else:
				ret.line('#undef %s' % config_define)
			ret.line('#else')
			undef_done = False
			emit_default_from_config_meta(ret, doc, forced_opts, undef_done)
			ret.line('#endif')
		elif doc.has_key('feature_snippet'):
			ret.lines(doc['feature_snippet'])
		else:
			pass

		ret.empty()

	ret.empty()

# Development time helper: add DUK_ACTIVE which provides a runtime C string
# indicating what DUK_USE_xxx config options are active at run time.  This
# is useful in genconfig development so that one can e.g. diff the active
# run time options of two headers.  This is intended just for genconfig
# development and is not available in normal headers.
def add_duk_active_defines_macro(ret):
	ret.chdr_block_heading('DUK_ACTIVE_DEFINES macro (development only)')

	idx = 0
	for doc in use_defs_list:
		defname = doc['define']

		ret.line('#if defined(%s)' % defname)
		ret.line('#define DUK_ACTIVE_DEF%d " %s"' % (idx, defname))
		ret.line('#else')
		ret.line('#define DUK_ACTIVE_DEF%d ""' % idx)
		ret.line('#endif')

		idx += 1

	tmp = []
	for i in xrange(idx):
		tmp.append('DUK_ACTIVE_DEF%d' % i)

	ret.line('#define DUK_ACTIVE_DEFINES ("Active: ["' + ' '.join(tmp) + ' " ]")')

#
#  duk_config.h generation
#

# Generate the default duk_config.h which provides automatic detection of
# platform, compiler, architecture, and features for major platforms.
# Use manually written monolithic header snippets from Duktape 1.2 for
# generating the header.  This header is Duktape 1.2 compatible and supports
# DUK_OPT_xxx feature options.  Later on the DUK_OPT_xxx options will be
# removed and users can override DUK_USE_xxx flags directly by modifying
# duk_config.h or by generating a new header using genconfig.
def generate_autodetect_duk_config_header(opts, meta_dir):
	ret = FileBuilder(base_dir=os.path.join(meta_dir, 'header-snippets'), \
	                  use_cpp_warning=opts.use_cpp_warning)

	forced_opts = get_forced_options(opts)

	ret.snippet_relative('comment_prologue.h.in')
	ret.empty()

	ret.line('#ifndef DUK_CONFIG_H_INCLUDED')
	ret.line('#define DUK_CONFIG_H_INCLUDED')
	ret.empty()

	# Compiler features, processor/architecture, OS, compiler
	ret.snippet_relative('compiler_features.h.in')
	ret.empty()
	ret.snippet_relative('rdtsc.h.in')  # XXX: move downwards
	ret.empty()
	ret.snippet_relative('platform1.h.in')
	ret.empty()

	# Feature selection, system include, Date provider
	# Most #include statements are here
	ret.snippet_relative('platform2.h.in')
	ret.empty()
	ret.snippet_relative('ullconsts.h.in')
	ret.empty()
	ret.snippet_relative('libc.h.in')
	ret.empty()

	# Number types
	ret.snippet_relative('types1.h.in')
	ret.line('#if defined(DUK_F_HAVE_INTTYPES)')
	ret.line('/* C99 or compatible */')
	ret.empty()
	ret.snippet_relative('types_c99.h.in')
	ret.empty()
	ret.line('#else  /* C99 types */')
	ret.empty()
	ret.snippet_relative('types_legacy.h.in')
	ret.empty()
	ret.line('#endif  /* C99 types */')
	ret.empty()
	ret.snippet_relative('types2.h.in')
	ret.empty()
	ret.snippet_relative('64bitops.h.in')
	ret.empty()

	# Alignment
	ret.snippet_relative('alignment.h.in')
	ret.empty()

	# Object layout
	ret.snippet_relative('object_layout.h.in')
	ret.empty()

	# Byte order
	ret.snippet_relative('byteorder.h.in')
	ret.empty()

	# Packed duk_tval
	ret.snippet_relative('packed_tval.h.in')
	ret.empty()

	# Detect 'fast math'
	ret.snippet_relative('reject_fast_math.h.in')
	ret.empty()

	# IEEE double constants
	ret.snippet_relative('double_const.h.in')
	ret.empty()

	# Math and other ANSI replacements, NetBSD workaround, paranoid math, paranoid Date
	ret.snippet_relative('repl_math.h.in')
	ret.empty()
	ret.snippet_relative('paranoid_date.h.in')
	ret.empty()
	ret.snippet_relative('repl_ansi.h.in')
	ret.empty()

	# Platform function pointers
	ret.snippet_relative('platform_funcptr.h.in')
	ret.empty()

	# General compiler stuff
	ret.snippet_relative('stringify.h.in')
	ret.empty()
	ret.snippet_relative('segfault.h.in')
	ret.empty()
	ret.snippet_relative('unreferenced.h.in')
	ret.empty()
	ret.snippet_relative('noreturn.h.in')
	ret.empty()
	ret.snippet_relative('unreachable.h.in')
	ret.empty()
	ret.snippet_relative('likely.h.in')
	ret.empty()
	ret.snippet_relative('inline.h.in')
	ret.empty()
	ret.snippet_relative('visibility.h.in')
	ret.empty()
	ret.snippet_relative('file_line_func.h.in')
	ret.empty()
	ret.snippet_relative('byteswap.h.in')
	ret.empty()

	# Arhitecture, OS, and compiler strings
	ret.snippet_relative('arch_string.h.in')
	ret.empty()
	ret.snippet_relative('os_string.h.in')
	ret.empty()
	ret.snippet_relative('compiler_string.h.in')
	ret.empty()

	# Target info
	ret.snippet_relative('target_info.h.in')
	ret.empty()

	# Longjmp handling
	ret.snippet_relative('longjmp.h.in')
	ret.empty()

	# Unsorted flags, contains almost all actual Duktape-specific
	# but platform independent features
	ret.snippet_relative('unsorted_flags.h.in')
	ret.empty()

	# User declarations
	ret.snippet_relative('user_declare.h.in')
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
			ret.chdr_block_heading('Forced options')
			first_forced = False

		undef_done = False
		if tmp.provides.has_key(defname):
			ret.line('#undef ' + defname)
			undef_done = True

		emit_default_from_config_meta(ret, doc, forced_opts, undef_done)

	ret.empty()

	# If manually-edited snippets don't #define or #undef a certain
	# config option, emit a default value here.  This is useful to
	# fill-in for new config options not covered by manual snippets
	# (which is intentional).

	tmp = Snippet(ret.join().split('\n'))
	need = {}
	for doc in use_defs_list:
		if doc.get('removed', None) is not None:  # XXX: check version
			continue
		need[doc['define']] = True
	for k in tmp.provides.keys():
		if need.has_key(k):
			del need[k]
	need_keys = sorted(need.keys())

	if len(need_keys) > 0:
		ret.chdr_block_heading('Autogenerated defaults')

		for k in need_keys:
			#print('config option %s not covered by manual snippets, emitting default automatically' % k)
			emit_default_from_config_meta(ret, use_defs[k], {}, False)

		ret.empty()

	ret.snippet_relative('custom_header.h.in')
	ret.empty()

	if len(opts.fixup_header_lines) > 0:
		ret.chdr_block_heading('Fixups')
		for line in opts.fixup_header_lines:
			ret.line(line)
		ret.empty()

	add_override_defines_section(opts, ret)

	# Date provider snippet is after custom header and overrides, so that
	# the user may define e.g. DUK_USE_DATE_NOW_GETTIMEOFDAY in their
	# custom header.
	ret.snippet_relative('date_provider.h.in')
	ret.empty()

	# Sanity checks
	# XXX: use autogenerated sanity checks instead
	ret.snippet_relative('sanity.h.in')
	ret.empty()

	if opts.emit_legacy_feature_check:
		# XXX: this doesn't really make sense for the autodetect
		# header yet, because sanity.h.in already covers these.
		add_legacy_feature_option_checks(opts, ret)
	if opts.emit_config_sanity_check:
		add_config_option_checks(opts, ret)
	if opts.add_active_defines_macro:
		add_duk_active_defines_macro(ret)

	ret.line('#endif  /* DUK_CONFIG_H_INCLUDED */')
	ret.empty()  # for trailing newline
	return remove_duplicate_newlines(ret.join())

# Generate a duk_config.h where platform, architecture, and compiler are
# all either autodetected or specified by user.  When autodetection is
# used, the generated header is based on modular snippets and metadata to
# be more easily maintainable than manually edited monolithic snippets.
#
# This approach will replace the legacy autodetect header in Duktape 1.4,
# and most likely the separate barebones header also.
#
# The generated header is Duktape 1.2 compatible for now, and supports
# DUK_OPT_xxx feature options.  Later on the DUK_OPT_xxx options will be
# removed and user code overrides DUK_USE_xxx flags directly by modifying
# duk_config.h manually or by generating a new header using genconfig.
def generate_autodetect_duk_config_header_modular(opts, meta_dir):
	ret = FileBuilder(base_dir=os.path.join(meta_dir, 'header-snippets'), \
	                  use_cpp_warning=opts.use_cpp_warning)

	forced_opts = get_forced_options(opts)

	platforms = None
	with open(os.path.join(meta_dir, 'platforms.yaml'), 'rb') as f:
		platforms = yaml.load(f)
	architectures = None
	with open(os.path.join(meta_dir, 'architectures.yaml'), 'rb') as f:
		architectures = yaml.load(f)
	compilers = None
	with open(os.path.join(meta_dir, 'compilers.yaml'), 'rb') as f:
		compilers = yaml.load(f)

	ret.line('/*')
	ret.line(' *  duk_config.h autodetect header generated by genconfig.py.')
	ret.line(' *')
	ret.line(' *  Git commit: %s' % opts.git_commit or 'n/a')
	ret.line(' *  Git describe: %s' % opts.git_describe or 'n/a')
	ret.line(' *')
	if opts.platform is not None:
		ret.line(' *  Platform: ' + opts.platform)
	else:
		ret.line(' *  Supported platforms:')
		for platf in platforms['autodetect']:
			ret.line(' *      - %s' % platf.get('name', platf.get('check')))
	ret.line(' *')
	if opts.architecture is not None:
		ret.line(' *  Architecture: ' + opts.architecture)
	else:
		ret.line(' *  Supported architectures:')
		for arch in architectures['autodetect']:
			ret.line(' *      - %s' % arch.get('name', arch.get('check')))
	ret.line(' *')
	if opts.compiler is not None:
		ret.line(' *  Compiler: ' + opts.compiler)
	else:
		ret.line(' *  Supported compilers:')
		for comp in compilers['autodetect']:
			ret.line(' *      - %s' % comp.get('name', comp.get('check')))
	ret.line(' *')
	ret.line(' */')
	ret.empty()
	ret.line('#ifndef DUK_CONFIG_H_INCLUDED')
	ret.line('#define DUK_CONFIG_H_INCLUDED')
	ret.empty()

	ret.chdr_block_heading('Intermediate helper defines')

	idx_deps = len(ret.vals)  # position where to emit dependencies

	# Feature selection, system include, Date provider
	# Most #include statements are here

	if opts.platform is not None:
		ret.chdr_block_heading('Platform: ' + opts.platform)

		ret.snippet_relative('platform_cppextras.h.in')
		ret.empty()

		# XXX: better to lookup platforms metadata
		include = 'platform_%s.h.in' % opts.platform
		validate_platform_file(os.path.join(meta_dir, 'header-snippets', include))
		ret.snippet_relative(include)
	else:
		ret.chdr_block_heading('Platform autodetection')

		ret.snippet_relative('platform_cppextras.h.in')
		ret.empty()

		for idx, platf in enumerate(platforms['autodetect']):
			check = platf.get('check', None)
			include = platf['include']
			validate_platform_file(os.path.join(meta_dir, 'header-snippets', include))

			if idx == 0:
				ret.line('#if defined(%s)' % check)
			else:
				if check is None:
					ret.line('#else')
				else:
					ret.line('#elif defined(%s)' % check)
			ret.snippet_relative(include)
		ret.line('#endif  /* autodetect platform */')

	ret.snippet_relative('platform_sharedincludes.h.in')
	ret.empty()

	if opts.architecture is not None:
		ret.chdr_block_heading('Architecture: ' + opts.architecture)

		# XXX: better to lookup architectures metadata
		include = 'architecture_%s.h.in' % opts.architecture
		validate_architecture_file(os.path.join(meta_dir, 'header-snippets', include))
		ret.snippet_relative(include)
	else:
		ret.chdr_block_heading('Architecture autodetection')

		for idx, arch in enumerate(architectures['autodetect']):
			check = arch.get('check', None)
			include = arch['include']
			validate_architecture_file(os.path.join(meta_dir, 'header-snippets', include))

			if idx == 0:
				ret.line('#if defined(%s)' % check)
			else:
				if check is None:
					ret.line('#else')
				else:
					ret.line('#elif defined(%s)' % check)
			ret.snippet_relative(include)
		ret.line('#endif  /* autodetect architecture */')

	if opts.compiler is not None:
		ret.chdr_block_heading('Compiler: ' + opts.compiler)

		# XXX: better to lookup compilers metadata
		include = 'compiler_%s.h.in' % opts.compiler
		validate_compiler_file(os.path.join(meta_dir, 'header-snippets', include))
		ret.snippet_relative(include)
	else:
		ret.chdr_block_heading('Compiler autodetection')

		for idx, comp in enumerate(compilers['autodetect']):
			check = comp.get('check', None)
			include = comp['include']
			validate_compiler_file(os.path.join(meta_dir, 'header-snippets', include))

			if idx == 0:
				ret.line('#if defined(%s)' % check)
			else:
				if check is None:
					ret.line('#else')
				else:
					ret.line('#elif defined(%s)' % check)
			ret.snippet_relative(include)
		ret.line('#endif  /* autodetect compiler */')

	# FIXME: The snippets below have some conflicts with the platform,
	# architecture, and compiler snippets files included above.  These
	# need to be resolved so that (a) each define is only provided from
	# one place or (b) the latter definition is a "fill-in" which is
	# only used when a certain define is missing from e.g. a compiler
	# snippet (useful for e.g. compiler defines which have sane, standard
	# defaults).

	# FIXME: __uclibc__ needs stdlib.h, but it really is the only exception
	ret.snippet_relative('libc.h.in')
	ret.empty()

	# Number types
	ret.snippet_relative('types1.h.in')
	ret.line('#if defined(DUK_F_HAVE_INTTYPES)')
	ret.line('/* C99 or compatible */')
	ret.empty()
	ret.snippet_relative('types_c99.h.in')
	ret.empty()
	ret.line('#else  /* C99 types */')
	ret.empty()
	ret.snippet_relative('types_legacy.h.in')
	ret.empty()
	ret.line('#endif  /* C99 types */')
	ret.empty()
	ret.snippet_relative('types2.h.in')
	ret.empty()
	ret.snippet_relative('64bitops.h.in')
	ret.empty()

	# Alignment
	ret.snippet_relative('alignment.h.in')
	ret.empty()

	# Object layout
	ret.snippet_relative('object_layout.h.in')
	ret.empty()

	# Byte order
	# FIXME: from the architecture snippet
	ret.snippet_relative('byteorder.h.in')
	ret.empty()

	# Packed duk_tval
	# FIXME: from the architecture snippet
	ret.snippet_relative('packed_tval.h.in')
	ret.empty()

	# Detect 'fast math'
	ret.snippet_relative('reject_fast_math.h.in')
	ret.empty()

	# IEEE double constants
	# FIXME: these should maybe be 'fill-ins' if previous headers
	# didn't provide something
	ret.snippet_relative('double_const.h.in')
	ret.empty()

	# Math and other ANSI replacements, NetBSD workaround, paranoid math, paranoid Date
	ret.snippet_relative('repl_math.h.in')
	ret.empty()
	ret.snippet_relative('paranoid_date.h.in')
	ret.empty()
	ret.snippet_relative('repl_ansi.h.in')
	ret.empty()

	# Platform function pointers
	ret.snippet_relative('platform_funcptr.h.in')
	ret.empty()

	# General compiler stuff
	ret.snippet_relative('stringify.h.in')
	ret.empty()
	ret.snippet_relative('segfault.h.in')
	ret.empty()
	ret.snippet_relative('unreferenced.h.in')
	ret.empty()
	ret.snippet_relative('noreturn.h.in')
	ret.empty()
	ret.snippet_relative('unreachable.h.in')
	ret.empty()
	ret.snippet_relative('likely.h.in')
	ret.empty()
	ret.snippet_relative('inline.h.in')
	ret.empty()
	ret.snippet_relative('visibility.h.in')
	ret.empty()
	ret.snippet_relative('file_line_func.h.in')
	ret.empty()
	ret.snippet_relative('byteswap.h.in')
	ret.empty()

	# These come directly from platform, architecture, and compiler
	# snippets.
	#ret.snippet_relative('arch_string.h.in')
	#ret.empty()
	#ret.snippet_relative('os_string.h.in')
	#ret.empty()
	#ret.snippet_relative('compiler_string.h.in')
	#ret.empty()
	#ret.snippet_relative('longjmp.h.in')
	#ret.empty()

	# Target info
	ret.snippet_relative('target_info.h.in')
	ret.empty()

	# Automatic DUK_OPT_xxx feature option handling
	# FIXME: platform setjmp/longjmp defines now conflict with this
	if True:
		# Unsorted flags, contains almost all actual Duktape-specific
		# but platform independent features
		#ret.snippet_relative('unsorted_flags.h.in')
		#ret.empty()

		add_feature_option_handling(opts, ret, forced_opts)

	ret.snippet_relative('user_declare.h.in')
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
			ret.chdr_block_heading('Forced options')
			first_forced = False

		undef_done = False
		if tmp.provides.has_key(defname):
			ret.line('#undef ' + defname)
			undef_done = True

		emit_default_from_config_meta(ret, doc, forced_opts, undef_done)

	ret.empty()

	# If manually-edited snippets don't #define or #undef a certain
	# config option, emit a default value here.  This is useful to
	# fill-in for new config options not covered by manual snippets
	# (which is intentional).

	tmp = Snippet(ret.join().split('\n'))
	need = {}
	for doc in use_defs_list:
		if doc.get('removed', None) is not None:  # XXX: check version
			continue
		need[doc['define']] = True
	for k in tmp.provides.keys():
		if need.has_key(k):
			del need[k]
	need_keys = sorted(need.keys())

	if len(need_keys) > 0:
		ret.chdr_block_heading('Autogenerated defaults')

		for k in need_keys:
			#print('config option %s not covered by manual snippets, emitting default automatically' % k)
			emit_default_from_config_meta(ret, use_defs[k], {}, False)

		ret.empty()

	ret.snippet_relative('custom_header.h.in')
	ret.empty()

	if len(opts.fixup_header_lines) > 0:
		ret.chdr_block_heading('Fixups')
		for line in opts.fixup_header_lines:
			ret.line(line)
		ret.empty()

	add_override_defines_section(opts, ret)

	# Date provider snippet is after custom header and overrides, so that
	# the user may define e.g. DUK_USE_DATE_NOW_GETTIMEOFDAY in their
	# custom header.
	ret.snippet_relative('date_provider.h.in')
	ret.empty()

	ret.fill_dependencies_for_snippets(idx_deps)

	# FIXME: use autogenerated sanity instead of sanity.h.in

	ret.snippet_relative('sanity.h.in')
	ret.empty()

	if opts.emit_legacy_feature_check:
		# FIXME: this doesn't really make sense for the autodetect header yet
		add_legacy_feature_option_checks(opts, ret)
	if opts.emit_config_sanity_check:
		add_config_option_checks(opts, ret)
	if opts.add_active_defines_macro:
		add_duk_active_defines_macro(ret)

	ret.line('#endif  /* DUK_CONFIG_H_INCLUDED */')
	ret.empty()  # for trailing newline
	return remove_duplicate_newlines(ret.join())

# Generate a barebones duk_config.h header for a specific platform, architecture,
# and compiler.  The header won't do automatic feature detection and does not
# support DUK_OPT_xxx feature options (which will be removed in Duktape 2.x).
# Users can then modify this barebones header for very exotic platforms and manage
# the needed changes either as a YAML file or by appending a fixup header snippet.
#
# XXX: to be replaced by generate_modular_duk_config_header().
def generate_barebones_duk_config_header(opts, meta_dir):
	ret = FileBuilder(base_dir=os.path.join(meta_dir, 'header-snippets'), \
	                  use_cpp_warning=opts.use_cpp_warning)

	# XXX: Provide more defines from YAML config files so that such
	#      defines can be overridden more conveniently (e.g. DUK_COS).

	forced_opts = get_forced_options(opts)

	ret.line('/*')
	ret.line(' *  duk_config.h generated by genconfig.py for:')
	ret.line(' *      platform: %s' % opts.platform)
	ret.line(' *      compiler: %s' % opts.compiler)
	ret.line(' *      architecture: %s' % opts.architecture)
	ret.line(' *')
	ret.line(' *  Git commit: %s' % opts.git_commit or 'n/a')
	ret.line(' *  Git describe: %s' % opts.git_describe or 'n/a')
	ret.line(' */')
	ret.empty()
	ret.line('#ifndef DUK_CONFIG_H_INCLUDED')
	ret.line('#define DUK_CONFIG_H_INCLUDED')

	ret.chdr_block_heading('Intermediate helper defines')

	idx_deps = len(ret.vals)  # position where to emit dependencies

	ret.chdr_block_heading('Platform headers and typedefs')

	if opts.platform is None:
		raise Exception('no platform specified')

	fn = 'platform_%s.h.in' % opts.platform
	ret.snippet_relative(fn)
	ret.empty()
	ret.snippet_relative('types_c99.h.in')  # XXX: C99 typedefs forced for now
	ret.snippet_relative('types2.h.in')     # XXX: boilerplate type stuff

	ret.chdr_block_heading('Platform features')

	# XXX: double constants
	# XXX: replacement functions
	# XXX: inherit definitions (like '#define DUK_FFLUSH fflush') from a
	#      generic set of defaults, allow platform configs to override

	ret.snippet_relative('platform_generic.h.in')

	ret.chdr_block_heading('Compiler features')

	if opts.compiler is None:
		raise Exception('no compiler specified')

	fn = 'compiler_%s.h.in' % opts.compiler
	ret.snippet_relative(fn)

	# noreturn, vacopy, etc
	# visibility attributes

	ret.chdr_block_heading('Architecture features')

	if opts.architecture is None:
		raise Exception('no architecture specified')

	fn = 'architecture_%s.h.in' % opts.architecture
	ret.snippet_relative(fn)

	ret.chdr_block_heading('Config options')

	tags = get_tag_list_with_preferred_order(header_tag_order)

	handled = {}

	# Mark all defines 'provided' by the snippets so far as handled.
	# For example, if the system header provides a DUK_USE_OS_STRING,
	# we won't emit it again below with its default value (but will
	# emit an override value if specified).

	for sn in ret.vals:
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

	if len(opts.fixup_header_lines) > 0:
		ret.chdr_block_heading('Fixups')
		for line in opts.fixup_header_lines:
			ret.line(line)

	add_override_defines_section(opts, ret)

	# Date provider snippet is after custom header and overrides, so that
	# the user may define e.g. DUK_USE_DATE_NOW_GETTIMEOFDAY in their
	# custom header.
	ret.empty()
	ret.snippet_relative('date_provider.h.in')
	ret.empty()

	ret.fill_dependencies_for_snippets(idx_deps)

	# XXX: ensure no define is unhandled at the end

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
	if opts.add_active_defines_macro:
		add_duk_active_defines_macro(ret)

	ret.line('#endif  /* DUK_CONFIG_H_INCLUDED */')
	ret.empty()  # for trailing newline

	return remove_duplicate_newlines(serialize_snippet_list(ret.vals))  # XXX: refactor into FileBuilder

#
#  Main
#

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
	parser.add_option('--git-commit', dest='git_commit', default=None, help='git commit hash to be included in header comments')
	parser.add_option('--git-describe', dest='git_describe', default=None, help='git describe string to be included in header comments')
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
		cmd = 'autodetect-header-legacy'

	if cmd == 'autodetect-header-legacy':
		# Generate a duk_config.h similar to Duktape 1.2 feature detection,
		# based on manually written monolithic snippets.
		# To be replaced by modular header.
		result = generate_autodetect_duk_config_header(opts, meta_dir)
		with open(opts.output, 'wb') as f:
			f.write(result)
	elif cmd == 'autodetect-header-modular':
		# Generate a duk_config.h similar to Duktape 1.2 feature detection.
		# Platform, architecture, and compiler can each be either autodetected
		# or specified by user.  Generated header is based on modular snippets
		# rather than a monolithic platform detection header.
		result = generate_autodetect_duk_config_header_modular(opts, meta_dir)
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
