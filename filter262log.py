#!/usr/bin/python

import os
import sys
import json

def main():
	f = open(sys.argv[1], 'rb')
	known = {}
	diagnosed = {}
	for o in json.loads(f.read()):
		if o.has_key('test') and o.has_key('known'):
			known[o['test']] = o['known']
		if o.has_key('test') and o.has_key('diagnosed'):
			diagnosed[o['test']] = o['diagnosed']
	f.close()

	skipstrings = [
		'passed in strict mode',
		'passed in non-strict mode',
		'failed in strict mode as expected',
		'failed in non-strict mode as expected'
	]

	in_failed_tests = False

	for line in sys.stdin:
		if len(line) > 1 and line[-1] == '\n':
			line = line[:-1]

		# Skip success cases

		skip = False
		for sk in skipstrings:
			if sk in line:
				skip = True
		if skip:
			continue

		# Augment error list with "known bugs"

		if line == 'Failed tests':
			in_failed_tests = True
			print(line)
			continue

		if in_failed_tests and line == '':
			in_failed_tests = False
			print(line)
			continue

		if in_failed_tests:
			# "  intl402/ch12/12.2/12.2.3_c in non-strict mode"
			tmp = line.strip().split(' ')
			test = tmp[0]
			if known.has_key(test):
				print(line + '   // KNOWN: ' + known[test])
			elif diagnosed.has_key(test):
				print(line + '   // diagnosed: ' + diagnosed[test])
			else:
				print(line)
			continue

		# Otherwise print normally

		print(line)

if __name__ == '__main__':
	main()
