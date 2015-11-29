import math
import random

def test():
	tmp1 = []
	tmp2 = []

	print('build')
	for i in xrange(1024):
		tmp1.append('%x' % math.floor(random.random() * 16))
	tmp1 = ''.join(tmp1)
	for i in xrange(1024):
		tmp2.append(tmp1)
	tmp2 = ''.join(tmp2)
	tmp2 = tmp2.encode('base64')

	tmp3 = []
	i = 0
	while i < len(tmp2):
		tmp3.append(tmp2[i:i+77])
		i += 77
	tmp2 = '\n'.join(tmp3) + '\n'

	print(len(tmp2))
	print('run')
	for i in xrange(2000):
		res = tmp2.decode('base64')

test()
