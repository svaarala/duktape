#!/usr/bin/python
#
#  Prepare the Node.js debugger Interface for duktape
#
# TODOs:
#  * Add more Sanity Checks
#  * Add Configuration Options.
#  * Test Python 3 Support.
#  * Add Mac OSX Support
#  * Add Linux Support
#  * Run "NPM Update" command.
#  * Run "NPM Install" command.

import sys, os, tempfile, logging
import optparse
import zipfile

if sys.version_info >= (3,):
    import urllib.request as urllib2
    import urllib.parse as urlparse
else:
    import urllib2
    import urlparse
enable_debug = False

def simple_resolve_file_path(file_path):
	file_directory = os.path.dirname(file_path)
	if(file_directory != ''):
		if not os.path.exists(file_directory):
			os.makedirs(file_directory)
			return 0
	return os.path.isfile(file_path)

def simple_download_file( out_path, web_url, referer = None ):
	global enable_debug
	if (simple_resolve_file_path(out_path)):
		return;
	print "Attempting to download \"%s\"" % (web_url)
	opener = urllib2.build_opener()
	opener.addheaders = [('User-agent', 'Mozilla/5.0')]
	if referer is not None:
		opener.addheaders = [('Referer', referer)]
	u = opener.open(web_url,timeout=10)
	meta = u.info()
	if enable_debug:
		print "Opener\n\n",opener
		print "meta\n\n",meta
		print meta.getheaders("Content-Length")[0]
	file_size = int(meta.getheaders("Content-Length")[0])
	if enable_debug:
		print "Downloading: %s Bytes: %s" % (out_path, file_size)
	f = open(out_path, 'wb')
	file_size_dl = 0
	block_sz = 8192
	if enable_debug:
		while True:
			buffer = u.read(block_sz)
			if not buffer:
				break
			file_size_dl += len(buffer)
			f.write(buffer)
			status = r"%10d  [%3.2f%%]" % (file_size_dl, file_size_dl * 100. / file_size)
			status = status + chr(8)*(len(status)+1)
			print status,
	else:
		while True:
			buffer = u.read(block_sz)
			if not buffer:
				break
			file_size_dl += len(buffer)
			f.write(buffer)
	f.close()

def simple_unzip_file(zipfile, file_member, out_file_path):
	global enable_debug
	if simple_resolve_file_path(out_file_path):
		return
	file_size = zipfile.getinfo(file_member).file_size
	if enable_debug:
		print "extracting: %s Bytes: %s" % (out_file_path, file_size)	
	u = zipfile.open(file_member)
	f = open(out_file_path, 'wb')
	zipfilepos = 0
	block_sz = 8192

	if enable_debug:
		while True:
			buffer = u.read(block_sz)
			if not buffer:
				break
			zipfilepos += len(buffer)
			f.write(buffer)
			status = r"%10d  [%3.2f%%]" % (zipfilepos, zipfilepos * 100. / file_size)
			status = status + chr(8)*(len(status)+1)
			print status,
	else:
		while True:
			buffer = u.read(block_sz)
			if not buffer:
				break
			zipfilepos += len(buffer)
			f.write(buffer)
	f.close()
	

def main():
	global enable_debug
	parser = optparse.OptionParser()
	parser.add_option('-d', dest='enableDebug',action='store_true', default=False, help='Enable Debug Print.')
	(opts, args) = parser.parse_args()
	enable_debug = opts.enableDebug;
	# Download and Install Node.js binary.
	if (os.name == 'nt') and not simple_resolve_file_path("node.exe"):
		simple_download_file("node.exe", "http://nodejs.org/dist/v0.12.7/node.exe")
	# Download and install the Node.js Package Manager.
	if (os.name == 'nt') and not simple_resolve_file_path("npm_v2.13.4.zip"):
		simple_download_file("npm_v2.13.4.zip",
			"http://github.com/npm/npm/archive/v2.13.4.zip", "http://github.com")
	if (simple_resolve_file_path("npm_v2.13.4.zip") and
		 not simple_resolve_file_path("bin/npm-cli.js")):
		fh = open("npm_v2.13.4.zip", "rb")
		z = zipfile.ZipFile(fh)
		for name in z.namelist():
			if(not os.path.basename(name)):
				continue
			if name.startswith("npm-2.13.4/bin"):
				simple_unzip_file(z, name, 'bin/%s'%(name[15:]))
			if name.startswith("npm-2.13.4/node_modules"):
				simple_unzip_file(z, name, 'node_modules/%s'%(name[24:]))
			if name.startswith("npm-2.13.4/lib"):
				simple_unzip_file(z, name, 'lib/%s'%(name[15:]))			
		fh.close()

	#Download and install Socket.IO.
	simple_download_file("static/socket.io-1.2.0.js", "http://cdn.socket.io/socket.io-1.2.0.js")

	# Download and Install JQuery 
	simple_download_file("static/jquery-1.11.1.min.js", "http://code.jquery.com/jquery-1.11.1.min.js")

	# Download and Install jQuery UI
	simple_download_file("jquery-ui-1.11.2.zip", "http://jqueryui.com/resources/download/jquery-ui-1.11.2.zip")
	if (simple_resolve_file_path("jquery-ui-1.11.2.zip") and
		 not simple_resolve_file_path("static/jquery-ui.min.js")):
		fh = open("jquery-ui-1.11.2.zip", "rb")
		z = zipfile.ZipFile(fh)
		simple_unzip_file(z, 'jquery-ui-1.11.2/jquery-ui.min.js', 'static/jquery-ui.min.js')
		simple_unzip_file(z, 'jquery-ui-1.11.2/jquery-ui.min.css', 'static/jquery-ui.min.css')
		#z.extract('jquery-ui-1.11.2/images', 'static/images')
		simple_resolve_file_path('static/images')
		for name in z.namelist():
			if name.startswith("jquery-ui-1.11.2/images"):
				simple_unzip_file(z, name, 'static/images/%s'%(os.path.basename(name)))
		fh.close()

	# Download and Install "jQuery Syntax Highlighter"
	# http://balupton.github.io/jquery-syntaxhighlighter/demo/
	simple_download_file("static/jquery.syntaxhighlighter.min.js",
		"http://balupton.github.com/jquery-syntaxhighlighter/scripts/jquery.syntaxhighlighter.min.js")

	# Download and Install Snippet
	# http://steamdev.com/snippet/
	simple_download_file("static/jquery.snippet.min.js",
		"http://steamdev.com/snippet/js/jquery.snippet.min.js")
	simple_download_file("static/jquery.snippet.min.css",
		"http://steamdev.com/snippet/css/jquery.snippet.min.css")

	# http://prismjs.com/
	# http://prismjs.com/plugins/line-highlight/
	#
	# XXX: prism download manually?

	# Download and Install "-prefix-free"
	# http://leaverou.github.io/prefixfree/
	# https://raw.github.com/LeaVerou/prefixfree/gh-pages/prefixfree.min.js
	simple_download_file("static/prefixfree.min.js",
		"http://raw.github.com/LeaVerou/prefixfree/gh-pages/prefixfree.min.js")

	# Download and install - CSS Tools: Reset CSS
	# http://meyerweb.com/eric/tools/css/reset/
	simple_download_file("static/reset.css",
		"http://meyerweb.com/eric/tools/css/reset/reset.css")

			
	# TODO: Run/Update Node.js
	# Run the following Commands in Debugger Folder.
	# node.exe bin\npm-cli.js update
	# node.exe bin\npm-cli.js install

	# Finally, The debugger should be ready for use:
	# node.exe duk_debug.js
if __name__ == "__main__":
    main()