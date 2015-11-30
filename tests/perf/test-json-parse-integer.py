import json
import random

def test():
	arr = []
	for i in xrange(10000):
		arr.append(int(random.random() * 1e9))

	jsondata = json.dumps(arr)
	print(len(jsondata))
	#print(jsondata)

	for i in xrange(100):
		ign = json.loads(jsondata)

test()
