#!/usr/bin/python
#
#  Format a perftest text dump into a HTML table.

import os
import sys
import re

def main():
	# test-try-catch-throw.js       : duk.O2.alt0 40.70 duk.O2.alt0f 40.74 duk.O2.alt1 40.10 duk.O2.alt1a 39.91 duk.O2.alt2 40.10 duk.O2.alt3 39.77 duk.O2.master 40.01 duk.O2.130 38.08

	re_line = re.compile(r'^(\S+)\s*:\s*(.*?)$')
	re_part = re.compile(r'\S+')
	first = True

	with open(sys.argv[1], 'rb') as f_in, open(sys.argv[2], 'wb') as f_out:
		f_out.write('<!DOCTYPE html>\n')
		f_out.write('<html>\n')
		f_out.write('<head>\n')
		f_out.write("""\
<style>
th, td { margin: 0; padding: 6pt; text-align: right; }
tr:nth-child(odd) { background: #eeeeee; }
</style>
""")
		f_out.write('</head>\n')
		f_out.write('<body>\n')
		f_out.write('<table>\n')
		for line in f_in:
			line = line.strip()
			m = re_line.match(line)
			if m is None:
				continue

			testname = m.group(1)
			parts = re_part.findall(m.group(2))

			if first:
				first = False
				f_out.write('<tr>')
				f_out.write('<th></th>')
				for idx in xrange(0, len(parts), 2):
					f_out.write('<th>' + parts[idx] + '</th>')
				f_out.write('</tr>\n')

			f_out.write('<tr>')
			f_out.write('<td>' + testname + '</td>')
			for idx in xrange(1, len(parts), 2):
				f_out.write('<td>' + parts[idx] + '</td>')
			f_out.write('</tr>\n')

		f_out.write('</table>\n')
		f_out.write('</body>\n')
		f_out.write('</html>\n')

if __name__ == '__main__':
	main()
