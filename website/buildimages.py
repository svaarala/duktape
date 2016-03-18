#!/usr/bin/env python2
#
#  Build data URIs for images.  Results are manually embedded into CSS.
#
#  https://en.wikipedia.org/wiki/Data_URI_scheme
#

import base64

if __name__ == '__main__':
	for image in [
			'bg-c.png', 'bg-ecmascript.png',
			'bg-c-2.png', 'bg-ecmascript-2.png'
			]:
		f = open(image, 'rb')
		data = f.read()
		f.close()
		data_uri = 'data:image/png;base64,' + base64.b64encode(data)
		print('')
		print('%s -> %s' % (image, data_uri))
		print('')

