#!/usr/bin/python
#
#  Utility to strip C comments from a C file, used to strip the combined
#  duktape.c file to make it smaller and faster to work with in IDEs.
#

import sys
import re

re_strip = re.compile('/\*(.*?)\*/', re.MULTILINE | re.DOTALL)

def main():
	with open(sys.argv[1], 'rb') as f:
		data = f.read()

	def censor(x):
		# Only process multiline comments.  This also reduces the
		# chance we'll process a C-comment-like string literal
		# which we don't otherwise check for now.
		if '\n' not in x.group(1):
			return x.group(0)

		# Keep newlines to retain line numbering
		tmp = ''
		for c in x.group(1):
			if c == '\n':
				tmp += '\n'
		return '/* ' + tmp + ' */'

	stripped = re_strip.sub(censor, data)

	with open(sys.argv[2], 'wb') as f:
		f.write(stripped)

if __name__ == '__main__':
	main()
