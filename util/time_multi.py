#!/usr/bin/python
#
#  Small helper for perftest runs.
#

import os
import sys
import time
import optparse
import subprocess

def main():
	parser = optparse.OptionParser()
	parser.add_option('--count', type='int', dest='count', default=3)
	parser.add_option('--mode', dest='mode', default='min')
	parser.add_option('--sleep', type='float', dest='sleep', default=0.0)
	parser.add_option('--sleep-factor', type='float', dest='sleep_factor', default=0.0)
	parser.add_option('--rerun-limit', type='int', dest='rerun_limit', default=30)
	parser.add_option('--verbose', action='store_true', dest='verbose', default=False)
	(opts, args) = parser.parse_args()

	time_min = None
	time_max = None
	time_sum = 0.0
	time_list = []

	if opts.verbose:
		sys.stderr.write('Running:')
		sys.stderr.flush()

	for i in xrange(opts.count):
		time.sleep(opts.sleep)

		cmd = [
			'time',
			'-f', '%U',
			'--quiet'
		]
		cmd = cmd + args
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

		time_this = float(stderr)
		#print(i, time_this)

		if time_min is None:
			time_min = time_this
		else:
			time_min = min(time_min, time_this)
		if time_max is None:
			time_max = time_this
		else:
			time_max = max(time_max, time_this)
		time_sum += time_this

		if opts.verbose:
			sys.stderr.write(' %f' % time_this)
			sys.stderr.flush()

		time_list.append(time_this)

		# Sleep time dependent on test time is useful for thermal throttling.
		time.sleep(opts.sleep_factor * time_this)

		# If run takes too long, there's no point in trying to get an accurate
		# estimate.
		if time_this >= opts.rerun_limit:
			break

	if opts.verbose:
		sys.stderr.write('\n')
		sys.stderr.flush()

	time_avg = time_sum / float(len(time_list))

	# /usr/bin/time has only two digits of resolution
	if opts.mode == 'min':
		print('%.02f' % time_min)
	elif opts.mode == 'max':
		print('%.02f' % time_max)
	elif opts.mode == 'avg':
		print('%.02f' % time_avg)
	elif opts.mode == 'all':
		print('min=%.02f, max=%.02f, avg=%0.2f, count=%d: %r' % \
		      (time_min, time_max, time_avg, len(time_list), time_list))
	else:
		print('invalid mode: %r' % opts.mode)

	sys.exit(0)

if __name__ == '__main__':
	main()
