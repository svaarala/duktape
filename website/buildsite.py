#!/usr/bin/python
#
#  Build Duktape website.  Must be run with cwd in the website/ directory.
#

import os
import sys
import shutil
import re
import tempfile
import atexit
from bs4 import BeautifulSoup

colorize = True
fancy_stack = True
remove_fixme = True
testcase_refs = False
list_tags = False

def readFile(x):
	f = open(x, 'rb')
	data = f.read()
	f.close()
	return data

def htmlEscape(x):
	res = ''
	esc = '&<>'
	for c in x:
		if ord(c) >= 0x20 and ord(c) <= 0x7e and c not in esc:
			res += c
		else:
			res += '&#x%04x;' % ord(c)
	return res

def getAutodeleteTempname():
	tmp = tempfile.mktemp(suffix='duktape-website')
	def f():
		os.remove(tmp)
	atexit.register(f)
	return tmp

# also escapes text automatically
def sourceHighlight(x, sourceLang):
	tmp1 = getAutodeleteTempname()
	tmp2 = getAutodeleteTempname()

	f = open(tmp1, 'wb')  # FIXME
	f.write(x)
	f.close()

	# FIXME: safer execution
	os.system('source-highlight -s %s -c highlight.css --no-doc <"%s" >"%s"' % \
	          (sourceLang, tmp1, tmp2))

	f = open(tmp2, 'rb')
	res = f.read()
	f.close()

	return res

def stripNewline(x):
	if len(x) > 0 and x[-1] == '\n':
		return x[:-1]
	return x

def validateAndParseHtml(data):
	# first parse as xml to get errors out
	ign_soup = BeautifulSoup(data, 'xml')

	# then parse as lenient html, no xml tags etc
	soup = BeautifulSoup(data)

	return soup

re_stack_line = re.compile(r'^(\[[^\x5d]+\])(?:\s+->\s+(\[[^\x5d]+\]))?(?:\s+(.*?))?\s*$')
def renderFancyStack(inp_line):
	# Support various notations here:
	#
	#   [ a b c ]
	#   [ a b c ] -> [ d e f ]
	#   [ a b c ] -> [ d e f ]  (if foo)
	#

	m = re_stack_line.match(inp_line)
	#print(inp_line)
	assert(m is not None)
	stacks = [ m.group(1) ]
	if m.group(2) is not None:
		stacks.append(m.group(2))

	res = []

	res.append('<div class="stack-wrapper">')
	for idx, stk in enumerate(stacks):
		if idx > 0:
			res.append('<span class="arrow"><b>&rarr;</b></span>')
		res.append('<span class="stack">')
		for part in stk.split(' '):
			part = part.strip()
			elem_classes = []
			elem_classes.append('elem')  #FIXME
			if len(part) > 0 and part[-1] == '!':
				part = part[:-1]
				elem_classes.append('active')
			elif len(part) > 0 and part[-1] == '*':
				part = part[:-1]
				elem_classes.append('referred')
			elif len(part) > 0 and part[-1] == '?':
				part = part[:-1]
				elem_classes.append('ghost')

			text = part

			# FIXME: detect special constants like "true", "null", etc?
			if text in [ 'undefined', 'null', 'true', 'false', 'NaN' ] or \
			   (len(text) > 0 and text[0] == '"' and text[-1] == '"'):
				elem_classes.append('literal')

			# FIXME: inline elements for reduced size?
			# The stack elements use a classless markup to minimize result
			# HTML size.  HTML inline elements are used to denote different
			# kinds of elements; the elements should be reasonable for text
			# browsers so a limited set can be used.
			use_inline = False

			if part == '':
				continue
			if part == '[':
				#res.append('<em>[</em>')
				res.append('<span class="cap">[</span>')
				continue
			if part == ']':
				#res.append('<em>]</em>')
				res.append('<span class="cap">]</span>')
				continue

			if part == '...':
				text = '. . .'
				elem_classes.append('ellipsis')
			else:
				text = part

			if 'ellipsis' in elem_classes and use_inline:
				res.append('<i>' + htmlEscape(text) + '</i>')
			elif 'active' in elem_classes and use_inline:
				res.append('<b>' + htmlEscape(text) + '</b>')
			else:
				res.append('<span class="' + ' '.join(elem_classes) + '">' + htmlEscape(text) + '</span>')

		res.append('</span>')

	# FIXME: pretty badly styled now
	if m.group(3) is not None:
		res.append('<span class="stack-comment">' + htmlEscape(m.group(3)) + '</span>')

	res.append('</div>')

	return ' '.join(res) + '\n'  # stack is a one-liner; spaces are for text browser rendering

def parseApiDoc(filename):
	f = open(filename, 'rb')
	parts = {}
	state = None
	for line in f.readlines():
		line = stripNewline(line)
		if line.startswith('='):
			state = line[1:]
		elif state is not None:
			if not parts.has_key(state):
				parts[state] = []
			parts[state].append(line)
		else:
			if line != '':
				raise Exception('unparsed non-empty line: %r' % line)
			else:
				# ignore
				pass
	f.close()

	# remove leading and trailing empty lines
	for k in parts:
		p = parts[k]
		while len(p) > 0 and p[0] == '':
			p.pop(0)
		while len(p) > 0 and p[-1] == '':
			p.pop()

	return parts

def processApiDoc(parts, funcname, testrefs, used_tags):
	res = []

	# this improves readability on e.g. elinks and w3m
	res.append('<hr />')
	#res.append('<hr>')

	# the 'hidechar' span is to allow browser search without showing the char
	res.append('<h2 id="%s"><a href="#%s"><span class="hidechar">.</span>%s()</a></h2>' % (funcname, funcname, funcname))

	if parts.has_key('proto'):
		p = parts['proto']
		res.append('<h3>Prototype</h3>')
		res.append('<pre class="c-code">')
		for i in p:
			res.append(htmlEscape(i))
		res.append('</pre>')
		res.append('')
	else:
		pass

	if parts.has_key('stack'):
		p = parts['stack']
		res.append('<h3>Stack</h3>')
		for line in p:
			res.append('<pre class="stack">' + \
			           '%s' % htmlEscape(line) + \
			           '</pre>')
		res.append('')
	else:
		res.append('<h3>Stack</h3>')
		res.append('<p>No effect.</p>')
		res.append('')

	if parts.has_key('summary'):
		p = parts['summary']
		res.append('<h3>Summary</h3>')

		# If text contains a '<p>', assume it is raw HTML; otherwise
		# assume it is a single paragraph (with no markup) and generate
		# paragraph tags, escaping into HTML

		raw_html = False
		for i in p:
			if '<p>' in i:
				raw_html = True

		if raw_html:
			for i in p:
				res.append(i)
		else:
			res.append('<p>')
			for i in p:
				res.append(htmlEscape(i))
			res.append('</p>')
		res.append('')

	if parts.has_key('example'):
		p = parts['example']
		res.append('<h3>Example</h3>')
		res.append('<pre class="c-code">')
		for i in p:
			res.append(htmlEscape(i))
		res.append('</pre>')
		res.append('')

	if parts.has_key('seealso'):
		p = parts['seealso']
		res.append('<h3>See also</h3>')
		res.append('<ul>')
		for i in p:
			res.append('<li><a href="#%s">%s</a></li>' % (htmlEscape(i), htmlEscape(i)))
		res.append('</ul>')

	if testcase_refs:
		res.append('<h3>Related test cases</h3>')
		if testrefs.has_key(funcname):
			res.append('<ul>')
			for i in testrefs[funcname]:
				res.append('<li>%s</li>' % htmlEscape(i))
			res.append('</ul>')
		else:
			res.append('<p>None.</p>')

	if not testrefs.has_key(funcname):
		res.append('<div class="fixme">This API call has no test cases.</div>')
		
	if list_tags and parts.has_key('tags'):
		# FIXME: placeholder
		res.append('<h3>Tags</h3>')
		res.append('<p>')
		p = parts['tags']
		for idx, val in enumerate(p):
			if idx > 0:
				res.append(' ')
			res.append(htmlEscape(val))
		res.append('</p>')
		res.append('')

	if parts.has_key('fixme'):
		p = parts['fixme']
		res.append('<div class="fixme">')
		for i in p:
			res.append(htmlEscape(i))
		res.append('</div>')
		res.append('')

	return res

def processRawDoc(filename):
	f = open(filename, 'rb')
	res = []
	for line in f.readlines():
		line = stripNewline(line)
		res.append(line)
	f.close()
	res.append('')
	return res

def transformColorizeCode(soup, cssClass, sourceLang):
	for elem in soup.select('pre.' + cssClass):
		input_str = elem.string
		if len(input_str) > 0 and input_str[0] == '\n':
			# hack for leading empty line
			input_str = input_str[1:]

		colorized = sourceHighlight(input_str, sourceLang)

		# source-highlight generates <pre><tt>...</tt></pre>, get rid of <tt>
		new_elem = BeautifulSoup(colorized).tt    # XXX: parse just a fragment - how?
		new_elem.name = 'pre'
		new_elem['class'] = cssClass

		elem.replace_with(new_elem)

def transformFancyStacks(soup):
	for elem in soup.select('pre.stack'):
		input_str = elem.string
		if len(input_str) > 0 and input_str[0] == '\n':
			# hack for leading empty line
			input_str = input_str[1:]

		new_elem = BeautifulSoup(renderFancyStack(input_str)).div  # XXX: fragment?
		elem.replace_with(new_elem)

def transformRemoveClass(soup, cssClass):
	for elem in soup.select('.' + cssClass):
		elem.extract()

def transformReadIncludes(soup, includeDir):
	for elem in soup.select('pre'):
		if not elem.has_key('include'):
			continue
		filename = elem['include']
		del elem['include']
		f = open(os.path.join(includeDir, filename), 'rb')
		elem.string = f.read()
		f.close()

def setNavSelected(soup, pagename):
	# pagename must match <li><a> content
	for elem in soup.select('#site-top-nav li'):
		if elem.text == pagename:
			elem['class'] = 'selected'

# FIXME: refactor shared parts

def scanApiCalls(apitestdir):	
	re_api_call = re.compile(r'duk_[0-9a-zA-Z_]+')

	res = {}  # api call -> [ test1, ..., testN ]

	tmpfiles = os.listdir(apitestdir)
	for filename in tmpfiles:
		if os.path.splitext(filename)[1] != '.c':
			continue

		f = open(os.path.join(apitestdir, filename))
		data = f.read()
		f.close()

		apicalls = re_api_call.findall(data)
		for i in apicalls:
			if not res.has_key(i):
				res[i] = []
			if filename not in res[i]:
				res[i].append(filename)

	for k in res.keys():
		res[k].sort()

	return res

def createTagIndex(api_docs, used_tags):
	res = []
	res.append('<h2 id="bytag">API calls by tag</h2>')

	for tag in used_tags:
		res.append('<h3>' + htmlEscape(tag) + '</h3>')
		res.append('<ul class="taglist">')
		for doc in api_docs:
			if not doc['parts'].has_key('tags'):
				continue
			for i in doc['parts']['tags']:
				if i != tag:
					continue
				res.append('<li><a href="#%s">%s</a></li>' % (htmlEscape(doc['name']), htmlEscape(doc['name'])))
		res.append('</ul>')

	return res

def generateApiDoc(apidocdir, apitestdir):
	templ_soup = validateAndParseHtml(readFile('template.html'))
	setNavSelected(templ_soup, 'API')

	# scan api files

	tmpfiles = os.listdir(apidocdir)
	apifiles = []
	for filename in tmpfiles:
		if os.path.splitext(filename)[1] == '.txt':
			apifiles.append(filename)
	apifiles.sort()
	#print(apifiles)
	print '%d api files' % len(apifiles)

	# scan api testcases for references to API calls

	testrefs = scanApiCalls(apitestdir)
	#print(repr(testrefs))

	# title

	title_elem = templ_soup.select('#template-title')[0]
	del title_elem['id']
	title_elem.string = 'Duktape API'

	# nav

	res = []
	navlinks = []
	navlinks.append(['#introduction', 'Introduction'])
	navlinks.append(['#concepts', 'Concepts'])
	navlinks.append(['#notation', 'Notation'])
	navlinks.append(['#defines', 'Header definitions'])
	navlinks.append(['#bytag', 'API calls by tag'])
	navlinks.append(['', u'\u00a0'])  # XXX: force vertical space
	for filename in apifiles:
		funcname = os.path.splitext(os.path.basename(filename))[0]
		navlinks.append(['#' + funcname, funcname])
	res.append('<ul>')
	for nav in navlinks:
		res.append('<li><a href="' + htmlEscape(nav[0]) + '">' + htmlEscape(nav[1]) + '</a></li>')
	res.append('</ul>')

	nav_soup = validateAndParseHtml('\n'.join(res))
	tmp_soup = templ_soup.select('#site-middle-nav')[0]
	tmp_soup.clear()
	for i in nav_soup.select('body')[0]:
		tmp_soup.append(i)

	# content

	res = []
	res += [ '<h1 class="main-title">Duktape API</h1>' ]

	# FIXME: generate from the same list as nav links for these
	res += processRawDoc('api/intro.html')
	res += processRawDoc('api/concepts.html')
	res += processRawDoc('api/notation.html')
	res += processRawDoc('api/defines.html')

	# scan api doc files

	used_tags = []
	api_docs = []   # [ { 'parts': xxx, 'name': xxx } ]

	for filename in apifiles:
		parts = parseApiDoc(os.path.join(apidocdir, filename))
		funcname = os.path.splitext(os.path.basename(filename))[0]
		if parts.has_key('tags') and 'omit' in parts['tags']:
			print 'Omit API doc: ' + str(funcname)
			continue
		if parts.has_key('tags'):
			for i in parts['tags']:
				if i not in used_tags:
					used_tags.append(i)
		api_docs.append({ 'parts': parts, 'name': funcname })

	used_tags.sort()

	# tag index
	res += createTagIndex(api_docs, used_tags)

	# api docs
	for doc in api_docs:
		# FIXME: Here we'd like to validate individual processApiDoc() results so
		# that they don't e.g. have unbalanced tags.  Or at least normalize them so
		# that they don't break the entire page.

		try:
			data = processApiDoc(doc['parts'], doc['name'], testrefs, used_tags)
			res += data
		except:
			print repr(data)
			print 'FAIL: ' + repr(filename)
			raise

	print('used tags: ' + repr(used_tags))

	res += [ '<hr>' ]

	content_soup = validateAndParseHtml('\n'.join(res))
	tmp_soup = templ_soup.select('#site-middle-content')[0]
	tmp_soup.clear()
	for i in content_soup.select('body')[0]:
		tmp_soup.append(i)
	tmp_soup['class'] = 'content'

	return templ_soup

def generateIndexPage():
	templ_soup = validateAndParseHtml(readFile('template.html'))
	index_soup = validateAndParseHtml(readFile('index/index.html'))
	setNavSelected(templ_soup, 'Home')

	title_elem = templ_soup.select('#template-title')[0]
	del title_elem['id']
	title_elem.string = 'Duktape'

	tmp_soup = templ_soup.select('#site-middle')[0]
	tmp_soup.clear()
	for i in index_soup.select('body')[0]:
		tmp_soup.append(i)
	tmp_soup['class'] = 'content'

	return templ_soup

def generateDownloadPage():
	templ_soup = validateAndParseHtml(readFile('template.html'))
	down_soup = validateAndParseHtml(readFile('download/download.html'))
	setNavSelected(templ_soup, 'Download')

	title_elem = templ_soup.select('#template-title')[0]
	del title_elem['id']
	title_elem.string = 'Downloads'

	tmp_soup = templ_soup.select('#site-middle')[0]
	tmp_soup.clear()
	for i in down_soup.select('body')[0]:
		tmp_soup.append(i)
	tmp_soup['class'] = 'content'

	return templ_soup

def generateGuide():
	templ_soup = validateAndParseHtml(readFile('template.html'))
	setNavSelected(templ_soup, 'Guide')

	title_elem = templ_soup.select('#template-title')[0]
	del title_elem['id']
	title_elem.string = 'Duktape Guide'

	# nav

	res = []
	navlinks = []
	navlinks.append(['#introduction', 'Introduction'])
	navlinks.append(['#gettingstarted', 'Getting started'])
	navlinks.append(['#programming', 'Programming model'])
	navlinks.append(['#types', 'Types'])
	navlinks.append(['#finalization', 'Finalization'])
	navlinks.append(['#coroutines', 'Coroutines'])
	navlinks.append(['#limitations', 'Limitations'])
	navlinks.append(['#comparisontolua', 'Comparison to Lua'])
	for nav in navlinks:
		res.append('<li><a href="' + htmlEscape(nav[0]) + '">' + htmlEscape(nav[1]) + '</a></li>')
	res.append('</ul>')

	nav_soup = validateAndParseHtml('\n'.join(res))
	tmp_soup = templ_soup.select('#site-middle-nav')[0]
	tmp_soup.clear()
	for i in nav_soup.select('body')[0]:
		tmp_soup.append(i)

	# content

	res = []
	res += [ '<h1 class="main-title">Duktape Programmer\'s Guide</h1>' ]

	# FIXME
	res += processRawDoc('guide/intro.html')
	res += processRawDoc('guide/gettingstarted.html')
	res += processRawDoc('guide/programming.html')
	res += processRawDoc('guide/types.html')
	res += processRawDoc('guide/finalization.html')
	res += processRawDoc('guide/coroutines.html')
	res += processRawDoc('guide/limitations.html')
	res += processRawDoc('guide/luacomparison.html')

	res += [ '<hr>' ]
	content_soup = validateAndParseHtml('\n'.join(res))
	tmp_soup = templ_soup.select('#site-middle-content')[0]
	tmp_soup.clear()
	for i in content_soup.select('body')[0]:
		tmp_soup.append(i)
	tmp_soup['class'] = 'content'

	return templ_soup

def generateStyleCss():
	styles = [
		'reset.css',
		'style-html.css',
		'style-content.css',
		'style-top.css',
		'style-middle.css',
		'style-bottom.css',
		'style-index.css',
		'style-download.css',
		'highlight.css'
	]

	style = ''
	for i in styles:
		style += '/* === %s === */\n' % i
		style += readFile(i)

	return style

def postProcess(soup, includeDir):
	# read in source snippets from include files
	if True:
		transformReadIncludes(soup, includeDir)

	if colorize:
		transformColorizeCode(soup, 'c-code', 'c')
		transformColorizeCode(soup, 'ecmascript-code', 'javascript')

	if fancy_stack:
		transformFancyStacks(soup)

	if remove_fixme:
		transformRemoveClass(soup, 'fixme')

	return soup

def writeFile(name, data):
	f = open(name, 'wb')
	f.write(data)
	f.close()

def main():
	outdir = sys.argv[1]; assert(outdir)
	apidocdir = 'api'
	apitestdir = '../api-testcases'
	guideincdir = '../examples/guide'
	apiincdir = '../examples/api'

	print 'Generating style.css'
	data = generateStyleCss()
	writeFile(os.path.join(outdir, 'style.css'), data)
	#writeFile(os.path.join(outdir, 'reset.css'), readFile('reset.css'))
	#writeFile(os.path.join(outdir, 'highlight.css'), readFile('highlight.css'))

	print 'Generating api.html'
	soup = generateApiDoc(apidocdir, apitestdir)
	soup = postProcess(soup, apiincdir)
	writeFile(os.path.join(outdir, 'api.html'), soup.encode('ascii'))

	print 'Generating guide.html'
	soup = generateGuide()
	soup = postProcess(soup, guideincdir)
	writeFile(os.path.join(outdir, 'guide.html'), soup.encode('ascii'))

	print 'Generating index.html'
	soup = generateIndexPage()
	soup = postProcess(soup, None)
	writeFile(os.path.join(outdir, 'index.html'), soup.encode('ascii'))

	print 'Generating download.html'
	soup = generateDownloadPage()
	soup = postProcess(soup, None)
	writeFile(os.path.join(outdir, 'download.html'), soup.encode('ascii'))

	print 'Copying binaries'
	for i in os.listdir('binaries'):
		shutil.copyfile(os.path.join('binaries', i), os.path.join(outdir, i))

if __name__ == '__main__':
	main()

