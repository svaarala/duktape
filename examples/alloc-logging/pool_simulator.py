#!/usr/bin/python
#
#  Simulate pool allocator behavior against a memory allocation log written
#  by duk_alloc_logging.c or in matching format.  Provide commands to provide
#  statistics and graphs, and to optimize pool counts for single or multiple
#  application profiles.
#
#  The pool allocator simulator incorporates quite basic pool features
#  including "borrowing" from larger pool sizes.  The behavior matches
#  AllJoyn.js ajs_heap.c allocator:
#
#    https://git.allseenalliance.org/cgit/core/alljoyn-js.git/tree/ajs_heap.c
#
#  If your pool allocator has different behavior (e.g. ability to split and
#  merge pool entries) you'll need to modify the simulator to properly
#  optimize pool counts.
#
#  Pool configuration and pool state are both expressed in JSON compatible
#  form internally so that they can read/written from/to files easily.
#

import os
import sys
import math
import json
import optparse

#---------------------------------------------------------------------------
#
#  Various helpers
#

def dprint(x):
	sys.stderr.write('%s\n' % x)
	sys.stderr.flush()

def readJson(fn):
	f = open(fn, 'rb')
	d = f.read()
	f.close()
	return json.loads(d)

def readFile(fn):
	f = open(fn, 'rb')
	d = f.read()
	f.close()
	return d

def writeJson(fn, val):
	f = open(fn, 'wb')
	f.write(json.dumps(val, indent=4, ensure_ascii=True, sort_keys=True))
	f.close()

def writeFile(fn, val):
	f = open(fn, 'wb')
	f.write(val)
	f.close()

# Clone a pool config (state), with all runtime fields intact
def clonePool(pool):
	return json.loads(json.dumps(pool))

# Clone a pool config, but clean it of any runtime fields
def clonePoolCleaned(pool):
	p = json.loads(json.dumps(pool))
	for k in [ 'entries', 'ajs_use', 'ajs_hwm', 'ajs_min', 'ajs_max' ]:
		if p.has_key(k):
			del p[k]
	return p

#---------------------------------------------------------------------------
#
#  Pool allocator simulator
#

# Pointers are represented simply as running numbers; 0 is NULL, and other
# numbers are simply alloc/realloc count.

nullPtr = 0
nextPtr = 1

HUGE = 0x100000000  # used for min()

class AllocFailedException(Exception):
	pass

class PoolSimulator:
	state = None
	config = None
	allow_borrow = True         # matches ajs_heap.c
	auto_extend = True          # for getting hwm w/o borrowing
	ignore_zero_alloc = False   # matches ajs_heap.c

	def __init__(self, config, borrow=True, extend=False):
		global nextPtr

		self.allow_borrow = borrow
		self.auto_extend = extend
		self.state = { 'pools': [] }
		self.config = json.loads(json.dumps(config))  # verify and clone

		for cfg in config['pools']:
			st = json.loads(json.dumps(cfg))
			st['entries'] = []
			st['ajs_use'] = 0      # entries in use
			st['ajs_hwm'] = 0      # max entries in use
			#st['ajs_min'] = None  # min alloc size
			#st['ajs_max'] = None  # max alloc size
			st['heap_index'] = st.get('heap_index', 0)  # ajs specific
			for i in xrange(cfg['count']):
				ent = { 'alloc_size': None,
				        'entry_size': st['size'],
				        'borrowed': False }  # free
				ent['pointer'] = nextPtr
				nextPtr += 1
				st['entries'].append(ent)
			self.state['pools'].append(st)

	def alloc(self, size):
		global nextPtr

		#print('alloc %d' % size)

		if size == 0 and self.ignore_zero_alloc:
			return nullPtr

		borrowed = False

		def alloc_match(e):
			e['alloc_size'] = size
			e['borrowed'] = borrowed
			p['ajs_use'] += 1
			p['ajs_hwm'] = max(p['ajs_use'], p['ajs_hwm'])
			p['ajs_min'] = min(p.get('ajs_min', HUGE), size)
			p['ajs_max'] = max(p.get('ajs_max', 0), size)
			return e['pointer']

		for p in self.state['pools']:
			if p['size'] < size:
				continue
			for e in p['entries']:
				if e['alloc_size'] is not None:
					continue
				return alloc_match(e)

			# Auto extend for measuring pool hwm without borrowing
			if self.auto_extend:
				ent = { 'alloc_size': None,
				        'entry_size': p['size'],
				        'borrowed': False,
				        'extended': True }
				ent['pointer'] = nextPtr
				nextPtr += 1
				p['entries'].append(ent)
				return alloc_match(ent)

			if not self.allow_borrow or not p['borrow']:
				raise AllocFailedException('alloc failure for size %d: pool full, no borrow' % size)
			borrowed = True

		raise AllocFailedException('alloc failure for size %d: went through all pools, no space' % size)

	def realloc(self, ptr, size):
		#print('realloc %d %d' % (ptr, size))

		if ptr == nullPtr:
			return self.alloc(size)

		if size == 0:
			self.free(ptr)
			return nullPtr

		# ptr != NULL and size != 0 here

		for idx in xrange(len(self.state['pools'])):
			p = self.state['pools'][idx]
			prev_p = None
			if idx >= 0:
				prev_p = self.state['pools'][idx - 1]

			for e in p['entries']:
				if e['pointer'] == ptr:
					if e['alloc_size'] is None:
						raise AllocFailedException('realloc failure for pointer %d: entry not allocated (double free)' % ptr)

					fits_current = (size <= p['size'])
					fits_previous = (prev_p is not None and size <= prev_p['size'])

					if fits_current and not fits_previous:
						# New alloc size fits current pool and won't fit into
						# previous pool (so it could be shrunk).

						p['ajs_max'] = max(p.get('ajs_max', 0), size)
						return ptr

					# Reallocate entry (smaller or larger).
					# Note: when shrinking, ajs_heap.c doesn't make sure
					# there's actually a free entry in the smaller pool.
					# This affects only some corner cases, but match
					# that behavior here.

					newPtr = self.alloc(size)
					self.free(ptr)
					return newPtr

		raise AllocFailedException('free failure for pointer %d: cannot find pointer' % ptr)

	def free(self, ptr):
		#print('free %d' % ptr)

		if ptr == nullPtr:
			return

		for p in self.state['pools']:
			for e in p['entries']:
				if e['pointer'] == ptr:
					if e['alloc_size'] is None:
						raise AllocFailedException('free failure for pointer %d: entry not allocated (double free)' % ptr)
					e['alloc_size'] = None
					e['borrowed'] = False
					p['ajs_use'] -= 1
					return

		raise AllocFailedException('free failure for pointer %d: cannot find pointer' % ptr)

	# Get a list of pool byte sizes.
	def getSizes(self):
		res = []
		for p in self.state['pools']:
			res.append(p['size'])
		return res

	# Get stats from current allocation state.
	def stats(self):
		alloc_bytes = 0
		waste_bytes = 0
		free_bytes = 0

		ajs_hwm_bytes = 0     # these correspond to runtime values from ajs_heap.c
		ajs_use_bytes = 0     # and are approximate
		ajs_waste_bytes = 0

		by_pool = []

		for p in self.state['pools']:
			alloc_bytes_pool = 0
			waste_bytes_pool = 0
			free_bytes_pool = 0

			for e in p['entries']:
				if e['alloc_size'] is None:
					free_bytes_pool += e['entry_size']
				else:
					alloc_bytes_pool += e['alloc_size']
					waste_bytes_pool += e['entry_size'] - e['alloc_size']

			ajs_use_count_pool = p['ajs_use']
			ajs_hwm_count_pool = p['ajs_hwm']
			ajs_min_bytes_pool = p.get('ajs_min', 0)
			ajs_max_bytes_pool = p.get('ajs_max', 0)
			ajs_hwm_bytes_pool = p['ajs_hwm'] * p['size']
			ajs_use_bytes_pool = p['ajs_use'] * p['size']
			ajs_waste_bytes_pool = p['ajs_hwm'] * (p['size'] - p.get('ajs_max', 0))

			by_pool.append({
				'size': p['size'],
				'alloc': alloc_bytes_pool,
				'waste': waste_bytes_pool,
				'free': free_bytes_pool,
				'ajs_use_count': ajs_use_count_pool,
				'ajs_hwm_count': ajs_hwm_count_pool,
				'ajs_min_bytes': ajs_min_bytes_pool,
				'ajs_max_bytes': ajs_max_bytes_pool,
				'ajs_hwm_bytes': ajs_hwm_bytes_pool,
				'ajs_use_bytes': ajs_use_bytes_pool,
				'ajs_waste_bytes': ajs_waste_bytes_pool
			})

			alloc_bytes += alloc_bytes_pool
			waste_bytes += waste_bytes_pool
			free_bytes += free_bytes_pool

			ajs_hwm_bytes += ajs_hwm_bytes_pool
			ajs_use_bytes += ajs_use_bytes_pool
			ajs_waste_bytes += ajs_waste_bytes_pool

		return {
			'alloc_bytes': alloc_bytes,
			'waste_bytes': waste_bytes,
			'free_bytes': free_bytes,

			'ajs_hwm_bytes': ajs_hwm_bytes,
			'ajs_use_bytes': ajs_use_bytes,
			'ajs_waste_bytes': ajs_waste_bytes,

			'byPool': by_pool
		}

	# Get "tight" pool config based on hwm of each pool size.
	def getTightHwmConfig(self):
		pools = []
		cfg = { 'pools': pools }
		total_bytes = 0
		for p in self.state['pools']:
			pool = clonePoolCleaned(p)
			pool['count'] = p['ajs_hwm']
			pools.append(pool)
			total_bytes += pool['size'] * pool['count']
		cfg['total_bytes'] = total_bytes
		return cfg

#---------------------------------------------------------------------------
#
#  Simulation: replay an allocation log
#

xIndex = 0

def processAllocLog(ps, f_log, out_dir, throw_on_oom=True, emit_files=True):
	# map native pointer to current simulator pointer
	ptrmap = {}

	def writeFile(fn, line):
		f = open(fn, 'ab')
		f.write(line + '\n')
		f.close()

	def emitStats():
		global xIndex

		if not emit_files:
			return

		stats = ps.stats()
		writeFile(os.path.join(out_dir, 'alloc_bytes_all.txt'), '%d %d' % (xIndex, stats['alloc_bytes']))
		writeFile(os.path.join(out_dir, 'waste_bytes_all.txt'), '%d %d' % (xIndex, stats['waste_bytes']))
		writeFile(os.path.join(out_dir, 'free_bytes_all.txt'), '%d %d' % (xIndex, stats['free_bytes']))
		writeFile(os.path.join(out_dir, 'ajs_hwm_bytes_all.txt'), '%d %d' % (xIndex, stats['ajs_hwm_bytes']))
		writeFile(os.path.join(out_dir, 'ajs_use_bytes_all.txt'), '%d %d' % (xIndex, stats['ajs_use_bytes']))
		writeFile(os.path.join(out_dir, 'ajs_waste_bytes_all.txt'), '%d %d' % (xIndex, stats['ajs_waste_bytes']))

		for p in stats['byPool']:
			writeFile(os.path.join(out_dir, 'alloc_bytes_%d.txt' % p['size']), '%d %d' % (xIndex, p['alloc']))
			writeFile(os.path.join(out_dir, 'waste_bytes_%d.txt' % p['size']), '%d %d' % (xIndex, p['waste']))
			writeFile(os.path.join(out_dir, 'free_bytes_%d.txt' % p['size']), '%d %d' % (xIndex, p['free']))
			writeFile(os.path.join(out_dir, 'ajs_use_count_%d.txt' % p['size']), '%d %d' % (xIndex, p['ajs_use_count']))
			writeFile(os.path.join(out_dir, 'ajs_hwm_count_%d.txt' % p['size']), '%d %d' % (xIndex, p['ajs_hwm_count']))
			writeFile(os.path.join(out_dir, 'ajs_min_bytes_%d.txt' % p['size']), '%d %d' % (xIndex, p['ajs_min_bytes']))
			writeFile(os.path.join(out_dir, 'ajs_max_bytes_%d.txt' % p['size']), '%d %d' % (xIndex, p['ajs_max_bytes']))
			writeFile(os.path.join(out_dir, 'ajs_hwm_bytes_%d.txt' % p['size']), '%d %d' % (xIndex, p['ajs_hwm_bytes']))
			writeFile(os.path.join(out_dir, 'ajs_use_bytes_%d.txt' % p['size']), '%d %d' % (xIndex, p['ajs_use_bytes']))
			writeFile(os.path.join(out_dir, 'ajs_waste_bytes_%d.txt' % p['size']), '%d %d' % (xIndex, p['ajs_waste_bytes']))
		xIndex += 1

	def emitSnapshot(count):
		if not emit_files:
			return

		f = open(os.path.join(out_dir, 'state_%d.json' % count), 'wb')
		f.write(json.dumps(ps.state, indent=4))
		f.close()

		stats = ps.stats()
		for p in stats['byPool']:
			logsize = math.log(p['size'], 2)
			writeFile(os.path.join(out_dir, 'alloc_bypool_%d.txt' % count), '%f %d   # size=%d' % (logsize, p['alloc'], p['size']))
			writeFile(os.path.join(out_dir, 'waste_bypool_%d.txt' % count), '%f %d   # size=%d' % (logsize, p['waste'], p['size']))
			writeFile(os.path.join(out_dir, 'free_bypool_%d.txt' % count), '%f %d   # size=%d' % (logsize, p['free'], p['size']))
			writeFile(os.path.join(out_dir, 'ajs_use_count_bypool_%d.txt' % count), '%f %d   # size=%d' % (logsize, p['ajs_use_count'], p['size']))
			writeFile(os.path.join(out_dir, 'ajs_hwm_count_bypool_%d.txt' % count), '%f %d   # size=%d' % (logsize, p['ajs_hwm_count'], p['size']))
			writeFile(os.path.join(out_dir, 'ajs_min_bytes_bypool_%d.txt' % count), '%f %d   # size=%d' % (logsize, p['ajs_min_bytes'], p['size']))
			writeFile(os.path.join(out_dir, 'ajs_max_bytes_bypool_%d.txt' % count), '%f %d   # size=%d' % (logsize, p['ajs_max_bytes'], p['size']))
			writeFile(os.path.join(out_dir, 'ajs_hwm_bytes_bypool_%d.txt' % count), '%f %d   # size=%d' % (logsize, p['ajs_hwm_bytes'], p['size']))
			writeFile(os.path.join(out_dir, 'ajs_use_bytes_bypool_%d.txt' % count), '%f %d   # size=%d' % (logsize, p['ajs_use_bytes'], p['size']))
			writeFile(os.path.join(out_dir, 'ajs_waste_bytes_bypool_%d.txt' % count), '%f %d   # size=%d' % (logsize, p['ajs_waste_bytes'], p['size']))

	sys.stdout.write('Simulating...')
	sys.stdout.flush()

	success = False

	try:
		count = 0
		for line in f_log:
			count += 1
			if (count % 1000) == 0:
				sys.stdout.write('.')
				sys.stdout.flush()
				emitSnapshot(count)

			emitStats()

			line = line.strip()
			parts = line.split(' ')

			# A ptr/NULL/FAIL size
			# F ptr/NULL size
			# R ptr/NULL oldsize ptr/NULL/FAIL newsize

			if len(parts) < 1:
				pass  # ignore
			elif parts[0] == 'A':
				if parts[1] == 'FAIL':
					pass
				elif parts[1] == 'NULL':
					ps.alloc(nullPtr)
				else:
					ptrmap[parts[1]] = ps.alloc(long(parts[2]))
			elif parts[0] == 'F':
				if parts[1] == 'NULL':
					ps.free(nullPtr)
				else:
					ptr = ptrmap[parts[1]]
					ps.free(ptr)
					del ptrmap[parts[1]]
			elif parts[0] == 'R':
				# oldsize is not needed; don't use because e.g. ajduk
				# log stats don't provide it

				if parts[1] == 'NULL':
					oldptr = nullPtr
				else:
					oldptr = ptrmap[parts[1]]

				if parts[3] == 'FAIL':
					pass
				else:
					newsize = long(parts[4])
					newptr = ps.realloc(oldptr, newsize)
					if newptr == nullPtr and newsize > 0:
						# Failed/freed, don't update pointers
						pass
					else:
						if parts[1] != 'NULL' and ptrmap.has_key(parts[1]):
							del ptrmap[parts[1]]
						if parts[3] != 'NULL':
							ptrmap[parts[3]] = newptr
			else:
				pass  # ignore

		sys.stdout.write(' done\n')
		sys.stdout.flush()
		success = True
	except AllocFailedException:
		sys.stdout.write(' failed, out of memory\n')
		sys.stdout.flush()
		if throw_on_oom:
			raise Exception('out of memory')

	emitSnapshot(count)
	emitStats()

	return success

#---------------------------------------------------------------------------
#
#  Gnuplot helper
#

def gnuplotGraphs(ps, out_dir):
	def plot(files, out_fn):
		f = open('/tmp/gnuplot-commands', 'wb')
		f.write('set terminal dumb\n')
		for idx, fn in enumerate(files):
			full_fn = os.path.join(out_dir, fn)
			cmd = 'plot'
			if idx > 0:
				cmd = 'replot'
			f.write('%s "%s" with lines\n' % (cmd, full_fn))
			#f.write('%s "%s" with boxes\n' % (cmd, full_fn))
		f.write('set terminal pngcairo size 1024,768\n')
		f.write('set output "%s"\n' % os.path.join(out_dir, out_fn))
		f.write('replot\n')
		f.close()

		os.system('gnuplot </tmp/gnuplot-commands >/dev/null 2>/dev/null')

	plot([ 'alloc_bytes_all.txt',
	       'waste_bytes_all.txt',
	       'free_bytes_all.txt' ], 'alloc_waste_free_all.png')
	plot([ 'alloc_bytes_all.txt',
	       'waste_bytes_all.txt',
	       'free_bytes_all.txt',
	       'ajs_hwm_bytes_all.txt',
	       'ajs_use_bytes_all.txt',
	       'ajs_waste_bytes_all.txt' ], 'alloc_waste_free_withajs_all.png')
	plot([ 'alloc_bytes_all.txt',
	       'waste_bytes_all.txt' ], 'alloc_waste_all.png')
	plot([ 'alloc_bytes_all.txt',
	       'waste_bytes_all.txt',
	       'ajs_hwm_bytes_all.txt',
	       'ajs_use_bytes_all.txt',
	       'ajs_waste_bytes_all.txt' ], 'alloc_waste_withajs_all.png')

	for sz in ps.getSizes():
		plot([ 'alloc_bytes_%d.txt' % sz,
		       'waste_bytes_%d.txt' % sz,
		       'free_bytes_%d.txt' % sz ], 'alloc_waste_free_%d.png' % sz)
		plot([ 'alloc_bytes_%d.txt' % sz,
		       'waste_bytes_%d.txt' % sz,
		       'free_bytes_%d.txt' % sz,
		       'ajs_hwm_bytes_%d.txt' % sz,
		       'ajs_use_bytes_%d.txt' % sz,
		       'ajs_waste_bytes_%d.txt' % sz ], 'alloc_waste_free_withajs_%d.png' % sz)
		plot([ 'alloc_bytes_%d.txt' % sz,
		       'waste_bytes_%d.txt' % sz ], 'alloc_waste_%d.png' % sz)
		plot([ 'alloc_bytes_%d.txt' % sz,
		       'waste_bytes_%d.txt' % sz,
		       'ajs_hwm_bytes_%d.txt' % sz,
		       'ajs_use_bytes_%d.txt' % sz,
		       'ajs_waste_bytes_%d.txt' % sz ], 'alloc_waste_withajs_%d.png' % sz)

	# plots containing all pool sizes in a timeline
	for name in [ 'alloc', 'waste' ]:
		files = []
		for sz in ps.getSizes():
			files.append('%s_bytes_%d.txt' % (name, sz))
		plot(files, '%s_bytes_allpools.png' % name)

	# autoplot for all data files
	for fn in os.listdir(out_dir):
		fn_txt = os.path.join(out_dir, fn)
		if not fn_txt.endswith('.txt'):
			continue
		fn_png = os.path.splitext(fn_txt)[0] + '.png'
		if os.path.exists(fn_png):
			continue

		plot([ fn ], fn_png)

	# XXX: plots for snapshots

#---------------------------------------------------------------------------
#
#  Pool optimization helpers
#

# Summary a pool config into a one-line string.
def configOneLiner(cfg):
	total_bytes = 0
	res = ''
	for i in xrange(len(cfg['pools'])):
		p1 = cfg['pools'][i]
		total_bytes += p1['size'] * p1['count']
		res += ' %r=%r' % (p1['size'], p1['count'])

	res = ('total %d:' % total_bytes) + res
	return res

# Convert a pool config into an ajs_heap.c AJS_HeapConfig initializer.
def configToAjsHeader(cfg):
	ind = '    '
	cfgName = 'heapConfig'

	res = []
	res.append('/* optimized using pool_simulator.py */')
	res.append('static const AJS_HeapConfig %s[] = {' % cfgName)
	res.append('%s/* %d bytes total */' % (ind, cfg['total_bytes']))
	for i in xrange(len(cfg['pools'])):
		p = cfg['pools'][i]
		if p['count'] == 0:
			continue
		borrow = '0'
		if p.get('borrow', False):
			borrow = 'AJS_POOL_BORROW'
		comma = ','   # could remove, need to know which line is last (zero counts affect it)
		res.append('%s{ %-7d, %-5d, %-16s, %d }%s   /* %7d bytes */' % \
		           (ind, p['size'], p['count'], borrow, p.get('heap_index', 0), comma,
		           p['size'] * p['count']))
	res.append('};')
	return '\n'.join(res) + '\n'

# Recompute 'total_bytes' of the pool (useful after modifications).
def recomputePoolTotal(cfg):
	total_bytes = 0
	for i in xrange(len(cfg['pools'])):
		p1 = cfg['pools'][i]
		total_bytes += p1['size'] * p1['count']
	cfg['total_bytes'] = total_bytes
	return cfg  # in-place

# Create a new pool config with pool counts added together.
def addPoolCounts(cfg1, cfg2):
	pools = []
	cfg = { 'pools': pools }

	if len(cfg1['pools']) != len(cfg2['pools']):
		raise Exception('incompatible pool configs')
	for i in xrange(len(cfg1['pools'])):
		p1 = cfg1['pools'][i]
		p2 = cfg2['pools'][i]
		if p1['size'] != p2['size']:
			raise Exception('incompatible pool configs')
		p3 = clonePoolCleaned(p1)
		p3['count'] = p1['count'] + p2['count']
		pools.append(p3)
	recomputePoolTotal(cfg)
	return cfg

# Create a new pool config with pool counts subtracts (result = cfg1 - cfg2).
def subtractPoolCounts(cfg1, cfg2):
	pools = []
	cfg = { 'pools': pools }

	if len(cfg1['pools']) != len(cfg2['pools']):
		raise Exception('incompatible pool configs')
	for i in xrange(len(cfg1['pools'])):
		p1 = cfg1['pools'][i]
		p2 = cfg2['pools'][i]
		if p1['size'] != p2['size']:
			raise Exception('incompatible pool configs')
		p3 = clonePoolCleaned(p1)
		p3['count'] = p1['count'] - p2['count']
		if p3['count'] < 0:
			print 'Warning: pool count went negative, replace with zero'
			p3['count'] = 0
			#raise Exception('pool count went negative')
		pools.append(p3)
	recomputePoolTotal(cfg)
	return cfg

# Create a new pool config with pool count being the maximum of all input
# configs (for each pool size).
def maxPoolCounts(cfglist):
	cfg1 = json.loads(json.dumps(cfglist[0]))  # start from clone of first config

	for cfg2 in cfglist:
		if len(cfg1['pools']) != len(cfg2['pools']):
			raise Exception('incompatible pool configs')
		for i in xrange(len(cfg1['pools'])):
			p1 = cfg1['pools'][i]
			p2 = cfg2['pools'][i]
			if p1['size'] != p2['size']:
				raise Exception('incompatible pool configs')
			p1['count'] = max(p1['count'], p2['count'])
	recomputePoolTotal(cfg1)
	return cfg1

# Scale pool counts with a factor, leaving pool counts fractional.
def scalePoolCountsFractional(cfg1, factor):
	pools = []
	cfg = { 'pools': pools }

	for i in xrange(len(cfg1['pools'])):
		p1 = cfg1['pools'][i]
		p2 = clonePoolCleaned(p1)
		p2['count'] = factor * p1['count']   # fractional
		pools.append(p2)
	recomputePoolTotal(cfg)
	return cfg

# Round pool counts to integer values with a configurable threshold.
def roundPoolCounts(cfg1, threshold):
	pools = []
	cfg = { 'pools': pools }

	for i in xrange(len(cfg1['pools'])):
		p1 = cfg1['pools'][i]
		count = math.floor(p1['count'])
		if p1['count'] - count > threshold:
			count += 1
		p2 = clonePoolCleaned(p1)
		p2['count'] = int(count)
		pools.append(p2)
	recomputePoolTotal(cfg)
	return cfg

def optimizePoolCountsForMemory(cfg_duktape, cfg_apps, target_memory):
	print('Duktape baseline: %s' % configOneLiner(cfg_duktape))

	# Subtract Duktape baseline from app memory usage
	for i in xrange(len(cfg_apps)):
		print('App with Duktape baseline: %s' % configOneLiner(cfg_apps[i]))
		cfg = subtractPoolCounts(cfg_apps[i], cfg_duktape)
		cfg_apps[i] = cfg
		print('App minus Duktape baseline: %s' % configOneLiner(cfg))

	# Normalize app memory usage
	normalized_memory = 1024.0 * 1024.0  # number doesn't really matter, fractions used
	for i in xrange(len(cfg_apps)):
		cfg = cfg_apps[i]
		factor = normalized_memory / cfg['total_bytes']
		cfg = scalePoolCountsFractional(cfg, factor)
		cfg_apps[i] = cfg
		print('Scaled app %d: %s' % (i, configOneLiner(cfg)))

	# Establish a representative profile over normalized application
	# profiles (over Duktape baseline).
	cfg_rep = maxPoolCounts(cfg_apps)
	print('Representative: %s' % configOneLiner(cfg_rep))

	# Scale (fractionally) to total bytes
	factor = (target_memory - cfg_duktape['total_bytes']) / cfg_rep['total_bytes']
	cfg_res = scalePoolCountsFractional(cfg_rep, factor)
	cfg_res = addPoolCounts(cfg_duktape, cfg_res)
	print('Fractional result: %s' % configOneLiner(cfg_res))

	# Round to integer pool counts with a sliding rounding
	# threshold so that we meet target memory as closely
	# as possible
	round_threshold = 1.0
	round_step = 0.0001
	round_threshold += round_step
	while True:
		cfg_tmp = roundPoolCounts(cfg_res, round_threshold - round_step)
		#print('rounding... %f -> %d total bytes' % (round_threshold, cfg_tmp['total_bytes']))
		if cfg_tmp['total_bytes'] > target_memory:
			# previous value was good
			break

		round_threshold -= round_step
		if round_threshold < 0.0:
			print('should not happen')
			round_threshold = 0.0
			break

	print('Final round threshold: %f' % round_threshold)
	cfg_final = roundPoolCounts(cfg_res, round_threshold)

	# XXX: negative pool counts

	print('Final pools: %s' % configOneLiner(cfg_final))
	return cfg_final

#---------------------------------------------------------------------------
#
#  Main program
#

# Simulate an allocation log and write out a lot of statistics and graphs.
def cmd_simulate(opts, args):
	dprint('Init pool simulator')
	ps = PoolSimulator(readJson(opts.pool_config), borrow=True, extend=False)

	dprint('Process allocation log')
	f = open(opts.alloc_log)
	processAllocLog(ps, f, opts.out_dir)
	f.close()

	dprint('Write tight pool config based on hwm')
	cfg = ps.getTightHwmConfig()
	f = open(os.path.join(opts.out_dir, 'config_tight.json'), 'wb')
	f.write(json.dumps(cfg, indent=4))
	f.close()

	dprint('Plot graphs (gnuplot)')
	gnuplotGraphs(ps, opts.out_dir)

	dprint('Finished, output is in: ' + str(opts.out_dir))

	#print(json.dumps(ps.state))

# Simulate an allocation log and optimize pool counts to tight values.
#
# If borrow_optimize=False, match pool count to high water mark with no
# borrowing.  Input pool counts are ignored and pools are extended as needed.
#
# If borrow_optimize=True, match pool counts initially to high water mark
# as before, but then reduce pool counts iteratively to minimum values which
# still allow the allocation log to be replayed without out-of-memory.  This
# results in pool counts which should be close to minimum values when
# borrowing behavior is taken into account.
def cmd_tight_counts(opts, args, borrow_optimize):
	# Get hwm profile with "autoextend", i.e. no borrowing

	print('Get hwm pool count profile with autoextend enabled (= no borrowing)')
	ps = PoolSimulator(readJson(opts.pool_config), borrow=False, extend=True)
	f = open(opts.alloc_log)
	processAllocLog(ps, f, opts.out_dir, throw_on_oom=True, emit_files=False)
	f.close()

	cfg = ps.getTightHwmConfig()
	print('Tight config based on hwm, no borrowing: %s' % configOneLiner(cfg))
	f = open(os.path.join(opts.out_dir, 'config_tight.json'), 'wb')
	f.write(json.dumps(cfg, indent=4))
	f.close()

	if not borrow_optimize:
		return cfg

	# Optimize pool counts taking borrowing into account.  Not very
	# optimal but step resizing ensures there shouldn't be pathological
	# cases (which might happen if step was -1).

	print('Optimizing pool counts taking borrowing into account (takes a while)...')

	for i in xrange(len(cfg['pools']) - 1, -1, -1):
		p = cfg['pools'][i]

		step = 1
		while step < p['count']:
			step *= 2
		highest_fail = -1

		while p['count'] > 0 and step > 0:
			prev_count = p['count']
			p['count'] -= step
			print('Reduce count for pool size %d bytes from %r to %r and resimulate' % (p['size'], prev_count, p['count']))

			# XXX: emits unused snapshots, optimize

			if p['count'] <= highest_fail:
				# we know this will fail
				success = False
			else:
				ps = PoolSimulator(cfg, borrow=True, extend=False)
				f = open(opts.alloc_log)
				success = processAllocLog(ps, f, opts.out_dir, throw_on_oom=False, emit_files=False)
				f.close()

			if not success:
				highest_fail = max(highest_fail, p['count'])
				p['count'] = prev_count
				step /= 2

		print('Pool config after size %d: %s' % (p['size'], configOneLiner(cfg)))

	print('Tight config based on hwm and optimizing borrowing: %s' % configOneLiner(cfg))
	return cfg

# Main entry point.
def main():
	parser = optparse.OptionParser()
	parser.add_option('--out-dir', dest='out_dir')
	parser.add_option('--pool-config', dest='pool_config')
	parser.add_option('--alloc-log', dest='alloc_log')
	parser.add_option('--out-pool-config', dest='out_pool_config')
	parser.add_option('--out-ajsheap-config', dest='out_ajsheap_config', default=None)
	(opts, args) = parser.parse_args()

	if not os.path.isdir(opts.out_dir):
		raise Exception('--out-dir argument is not a directory')
	if len(os.listdir(opts.out_dir)) > 0:
		raise Exception('--out-dir argument is not empty')

	def writeOutputs(cfg):
		writeJson(opts.out_pool_config, cfg)
		if opts.out_ajsheap_config is not None:
			writeFile(opts.out_ajsheap_config, configToAjsHeader(cfg))

	cmd = args[0]
	if cmd == 'simulate':
		cmd_simulate(opts, args)
	elif cmd == 'tight_counts_noborrow':
		cfg = cmd_tight_counts(opts, args, False)
		writeOutputs(cfg)
	elif cmd == 'tight_counts_borrow':
		cfg = cmd_tight_counts(opts, args, True)
		writeOutputs(cfg)
	elif cmd == 'subtract_pool_counts':
		# XXX: unused
		cfg1 = readJson(args[1])
		cfg2 = readJson(args[2])
		cfg3 = subtractPoolCounts(cfg1, cfg2)
		writeOutputs(cfg3)
	elif cmd == 'max_pool_counts':
		# XXX: unused
		# Not very useful without normalization.
		cfg = maxPoolCounts(args[1:])
		writeOutputs(cfg)
	elif cmd == 'pool_counts_for_memory':
		target_memory = long(args[1])
		cfg_duktape = readJson(args[2])
		print('Duktape baseline: %d bytes' % cfg_duktape['total_bytes'])
		cfg_apps = []
		for arg in args[3:]:
			cfg = readJson(arg)
			cfg_apps.append(cfg)
			print('Application: %d bytes' % cfg['total_bytes'])
		cfg = optimizePoolCountsForMemory(cfg_duktape, cfg_apps, target_memory)
		writeOutputs(cfg)
	else:
		raise Exception('invalid command ' + str(cmd))

if __name__ == '__main__':
	main()
