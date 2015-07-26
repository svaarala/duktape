#!/usr/bin/python
#
#  Throwaway utility to dump Ditz issues for grooming.
#

import os
import sys
import yaml

def main():
	def issueConstructor(loader, node):
		return node

	yaml.add_constructor('!ditz.rubyforge.org,2008-03-06/issue', issueConstructor)

	for fn in os.listdir(sys.argv[1]):
		if fn[0:6] != 'issue-':
			continue
		with open(os.path.join(sys.argv[1], fn), 'rb') as f:
			doc = yaml.load(f)
			tmp = {}
			for k,v in doc.value:
				tmp[k.value] = v.value
			if tmp.get('status', '') != ':closed':
				print('*** ' + fn)
				print(tmp.get('title', u'NOTITLE').encode('utf-8') + '\n')
				print(tmp.get('desc', u'').encode('utf-8') + '\n')

if __name__ == '__main__':
	main()
