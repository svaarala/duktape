def test():
	for i in xrange(int(5e3)):
		t = []
		for j in xrange(int(1e4)):
			#t[j] = 'x'
			t.append('x')
		t = ''.join(t)

test()
