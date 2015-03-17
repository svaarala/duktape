def test():
	def f():
		return

	i = 0
	while i < 1e8:
		f()
		i += 1

test()
