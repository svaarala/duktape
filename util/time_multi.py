#!/usr/bin/python
#
#  Small helper for perftest runs.
#

import os
import sys
import subprocess

def main():
	count = int(sys.argv[1])

	time_min = None
	for i in xrange(count):
		cmd = [
			'time',
			'-f', '%U',
			'--quiet',
			sys.argv[2],   # cmd
			sys.argv[3]    # testcase
		]
		#print(repr(cmd))
		p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		stdout, stderr = p.communicate()
		retval = p.wait()
		#print(i, retval, stdout, stderr)

		if retval == 139:
			print 'segv'
			sys.exit(1)
		elif retval != 0:
			print 'n/a'
			sys.exit(1)

		time = float(stderr)
		#print(i, time)

		if time_min is None:
			time_min = time
		else:
			time_min = min(time_min, time)

	# /usr/bin/time has only two digits of resolution
	print('%.02f' % time_min)
	sys.exit(0)

if __name__ == '__main__':
	main()
