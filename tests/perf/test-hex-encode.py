import math
import random

def test():
	tmp1 = []
	tmp2 = []

	print('build')
	for i in xrange(1024):
		tmp1.append(chr(int(math.floor(random.random() * 256))))
	tmp1 = ''.join(tmp1)
	for i in xrange(1024):
		tmp2.append(tmp1)
	tmp2 = ''.join(tmp2)

	print(len(tmp2))
	print('run')
	for i in xrange(5000):
		res = tmp2.encode('hex')

test()
