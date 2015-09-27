def test():
	obj = { 'xxx1': 1, 'xxx2': 2, 'xxx3': 4, 'xxx4': 4, 'foo': 123 }
	i = 0
	while i < 1e7:
		obj['foo'] = 234
		obj['foo'] = 234
		obj['foo'] = 234
		obj['foo'] = 234
		obj['foo'] = 234
		obj['foo'] = 234
		obj['foo'] = 234
		obj['foo'] = 234
		obj['foo'] = 234
		obj['foo'] = 234
		i += 1

test()
