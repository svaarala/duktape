#!/usr/bin/env python2
#
#  One-time tool to convert from old custom API document format to YAML.
#
#  $ for i in api/*.txt; do python api2yaml.py $i ${i%%.txt}.yaml; done
#

import os
import sys

def main(f_in, f_out, funcname):
	parts = {}
	curr = None
	partname = None

	def quoted(line):
		if line.strip() == '':
			# Don't print whitespace indent for empty lines
			f_out.write('\n')
		else:
			f_out.write('  %s\n' % line)

	for line in f_in:
		if len(line) > 0 and line[-1] == '\n':
			line = line[:-1]
		if len(line) > 0 and line[0] == '=':
			partname = line[1:]
			curr = []
			parts[partname] = curr
			continue

		curr.append(line)

	# Although the key order in the YAML output doesn't matter,
	# we want it to be in a specific order to make manual edits
	# nicer.  Do this by emitting the YAML manually.

	#print(repr(parts))

	for key in parts.keys():
		part = parts[key]
		while len(part) > 0 and part[-1] == '':
			part = part[:-1]
		parts[key] = part

	for key in parts.keys():
		part = parts[key]
		if len(part) == 0:
			del parts[key]

	f_out.write('name: %s\n' % funcname)

	assert(parts.has_key('proto'))

	f_out.write('\n')
	f_out.write('proto: |\n')
	for p in parts['proto']:
		quoted(p)

	if parts.has_key('stack'):
		f_out.write('\n')
		f_out.write('stack: |\n')
		for p in parts['stack']:
			quoted(p)

	assert(parts.has_key('summary'))

	f_out.write('\n')
	f_out.write('summary: |\n')
	for p in parts['summary']:
		quoted(p)

	assert(parts.has_key('example'))

	f_out.write('\n')
	f_out.write('example: |\n')
	for p in parts['example']:
		quoted(p)

	if parts.has_key('tags'):
		f_out.write('\n')
		f_out.write('tags:\n')
		for p in parts['tags']:
			f_out.write('  - %s\n' % p)

	if parts.has_key('seealso'):
		f_out.write('\n')
		f_out.write('seealso:\n')
		for p in parts['seealso']:
			f_out.write('  - %s\n' % p)

	if parts.has_key('introduced'):
		assert(len(parts['introduced']) == 1)
		f_out.write('\n')
		f_out.write('introduced: %s\n' % parts['introduced'][0])

	if parts.has_key('fixme'):
		f_out.write('fixme: |\n')
		for p in parts['fixme']:
			quote(p)

if __name__ == '__main__':
	with open(sys.argv[1], 'rb') as f_in, \
	     open(sys.argv[2], 'wb') as f_out:
		fn = os.path.basename(sys.argv[1])
		fn_plain = os.path.splitext(fn)[0]
		main(f_in, f_out, fn_plain)
