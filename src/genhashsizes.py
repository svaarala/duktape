#!/usr/bin/python
#
#  Find a sequence of duk_hobject hash sizes which have a desired 'ratio'
#  and are primes.  Prime hash sizes ensure that all probe sequence values
#  (less than hash size) are relatively prime to hash size, i.e. cover the
#  entire hash.  Prime data is packed into about 1 byte/prime using a
#  prediction-correction model.
#
#  Also generates a set of probe steps which are relatively prime to every
#  hash size.

import sys
import math

def is_prime(n):
	if n == 0:
		return False
	if n == 1 or n == 2:
		return True

	n_limit = int(math.ceil(float(n) ** 0.5)) + 1
	n_limit += 100  # paranoia
	if n_limit >= n:
		n_limit = n - 1
	for i in xrange(2,n_limit + 1):
		if (n % i) == 0:
			return False
	return True

def next_prime(n):
	while True:
		n += 1
		if is_prime(n):
			return n

def generate_sizes(min_size, max_size, step_ratio):
	"Generate a set of hash sizes following a nice ratio."

	sizes = []
	ratios = []
	curr = next_prime(min_size)
	next = curr
	sizes.append(curr)

	step_ratio = float(step_ratio) / 1024

	while True:
		if next > max_size:
			break
		ratio = float(next) / float(curr)
		if ratio < step_ratio:
			next = next_prime(next)
			continue
		sys.stdout.write('.'); sys.stdout.flush()
		sizes.append(next)
		ratios.append(ratio)
		curr = next
		next = next_prime(int(next * step_ratio))

	sys.stdout.write('\n'); sys.stdout.flush()
	return sizes, ratios

def generate_corrections(sizes, step_ratio):
	"Generate a set of correction from a ratio-based predictor."

	# Generate a correction list for size list, assuming steps follow a certain
	# ratio; this allows us to pack size list into one byte per size

	res = []

	res.append(sizes[0])  # first entry is first size

	for i in xrange(1, len(sizes)):
		prev = sizes[i - 1]
		pred = int(prev * step_ratio) >> 10
		diff = int(sizes[i] - pred)
		res.append(diff)

		if diff < 0 or diff > 127:
			raise Exception('correction does not fit into 8 bits')

	res.append(-1)  # negative denotes last end of list
	return res

def generate_probes(count, sizes):
	res = []

	# Generate probe values which are guaranteed to be relatively prime to
	# all generated hash size primes.  These don't have to be primes, but
	# we currently use smallest non-conflicting primes here.

	i = 2
	while len(res) < count:
		if is_prime(i) and (i not in sizes):
			if i > 255:
				raise Exception('probe step does not fit into 8 bits')
			res.append(i)
			i += 1
			continue
		i += 1

	return res

# NB: these must match duk_hobject defines and code
step_ratio = 1177  # approximately (1.15 * (1 << 10))
min_size = 16
max_size = 2**32 - 1

sizes, ratios = generate_sizes(min_size, max_size, step_ratio)
corrections = generate_corrections(sizes, step_ratio)
probes = generate_probes(32, sizes)
print len(sizes)
print 'SIZES: ' + repr(sizes)
print 'RATIOS: ' + repr(ratios)
print 'CORRECTIONS: ' + repr(corrections)
print 'PROBES: ' + repr(probes)

# highest 32-bit prime
i = 2**32
while True:
	i -= 1
	if is_prime(i):
		print 'highest 32-bit prime is: %d (0x%08x)' % (i, i)
		break

