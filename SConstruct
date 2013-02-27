#
#  SConstruct for the Duktape project, including sources, documentation, etc.
#

# FIXME: target detection is just a placeholder

import os
import sys
import subprocess

# helper to get a command's stdout (executed during tree build)
def get_stdout(args):
	p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	stdout_data, stderr_data = p.communicate()
	rc = p.wait()
	if rc != 0:
		raise Exception('failed to run %r -> %r' % (args, rc))
	return stdout_data.strip()

# detect host features and assume (for now) that it is also the target
machine_infos = {
	'i386': { 'name': 'x86 (i386)', 'supports_packed': True, 'byte_order': 'little' },
	'i486': { 'name': 'x86 (i486)', 'supports_packed': True, 'byte_order': 'little' },
	'i586': { 'name': 'x86 (i586)', 'supports_packed': True, 'byte_order': 'little' },
	'i686': { 'name': 'x86 (i686)', 'supports_packed': True, 'byte_order': 'little' },
	'x86_64': { 'name': 'x86-64', 'supports_packed': False, 'byte_order': 'little' },
	'default': { 'name': 'unknown', 'supports_packed': True, 'byte_order': 'little' },  # just a guess
}
machine_type = get_stdout(['uname', '-m'])
if machine_infos.has_key(machine_type):
	machine = machine_infos[machine_type]
else:
	machine = machine_infos['default']
print('Detected host (=target) machine: %r' % machine)

# build info
buildinfo_date = get_stdout(['date', '+%Y-%m-%d'])  # more stable than full datetime -> better for incremental builds
buildinfo_uname = get_stdout(['uname', '-a'])
try:
	buildinfo_githash = get_stdout(['git', 'rev-parse', 'HEAD'])
except:
	buildinfo_githash = 'exported'
duk_buildinfo = '%s; %s; %s' % (buildinfo_date, buildinfo_uname, buildinfo_githash)

# duk version
duk_version = 1

# profile selection, may depend on target capabilities
duk_profiles = [
	{ 'number': 100, 'packed': True, 'debug': False },
	{ 'number': 101, 'packed': True, 'debug': True },
	{ 'number': 200, 'packed': True, 'debug': False },
	{ 'number': 201, 'packed': True, 'debug': True },
	{ 'number': 300, 'packed': True, 'debug': False },
	{ 'number': 301, 'packed': True, 'debug': True },
	{ 'number': 400, 'packed': False, 'debug': False },
	{ 'number': 401, 'packed': False, 'debug': True },
	{ 'number': 500, 'packed': False, 'debug': False },
	{ 'number': 501, 'packed': False, 'debug': True },
]

# Preferred compiler and options.  Note: GCC version has significant impact
# on -Os code size, e.g. gcc-4.6 is way worse than gcc-4.5.

# FIXME: not used now
PREF_CC = 'gcc'
#PREF_CC = '/usr/bin/gcc-4.3'
#PREF_CC = '/usr/bin/gcc-4.4'
#PREF_CC = '/usr/bin/gcc-4.5'
#PREF_CC = '/usr/bin/gcc-4.6'
#PREF_CC = '/usr/bin/gcc-4.7'

# FIXME: this is a hack for x86-64, where we want to build both 32-bit and 64-bit
# files for better testing; needs a separate variant
FORCE_32BIT = False
#if machine_type == 'x86_64':
#	FORCE_32BIT = True

ccopts_shared = [
	'-pedantic',
	'-ansi',
	'-std=c99',
	'-Wall',
	'-fstrict-aliasing',
	'-D_POSIX_C_SOURCE=200809L',
#	'-D_GNU_SOURCE',		# FIXME: needed for e.g. getdate_r
	'-D_XOPEN_SOURCE',		# strptime
	'-DDUK_OPT_DPRINT_RDTSC=1',
]
if FORCE_32BIT:
	ccopts_shared += [ '-m32' ]

ccopts_release = ccopts_shared + [
	'-fomit-frame-pointer',
	'-Os',
	'-g',
	'-ggdb',
]
ccopts_debug = ccopts_shared + [
	'-O0',
	'-g',
	'-ggdb',
]

ldflags_shared = []
if FORCE_32BIT:
	ldflags_shared += [ '-m32' ]

ldflags_release = ldflags_shared
ldflags_debug = ldflags_shared

env_base = Environment()
env_release = env_base.Clone(CCFLAGS=ccopts_release, LINKFLAGS=ldflags_release)
env_debug = env_base.Clone(CCFLAGS=ccopts_debug, LINKFLAGS=ldflags_debug)

for prof in duk_profiles:
	if prof['debug']:
		env_build = env_debug.Clone()
	else:
		env_build = env_release.Clone()

	if prof['packed'] and not machine['supports_packed'] and not FORCE_32BIT:
		print('skip packed profile (target does not support packed): %r' % prof)
		continue

	SConscript('src/SConscript',
	           variant_dir='build/' + str(prof['number']),
	           exports={ 'duk_profile': str(prof['number']),
	                     'duk_version': duk_version,
	                     'duk_buildinfo': duk_buildinfo,
	                     'is_debug': prof['debug'],
	                     'byte_order': machine['byte_order'],
	                     'env': env_build })

	Clean('.', ['build/' + str(prof['number'])])

Clean('.', ['build'])

SConscript('doc/SConscript')

