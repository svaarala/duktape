def test():
	def f():
		return

	i = 0
	while i < 1e7:
		f()
		f()
		f()
		f()
		f()
		f()
		f()
		f()
		f()
		f()
		i += 1

test()
