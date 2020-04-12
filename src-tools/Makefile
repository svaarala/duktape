.PHONY: all
all: duktool.js

.PHONY: eslint
eslint:
	npx eslint .

.PHONY: clean
clean:
	-@rm -f duktool.js
	-@rm -rf lib/extdeps

.PHONY: cleanall
cleanall: clean
	-@rm -rf js-yaml
	-@rm -rf from-xml
	-@rm -rf node_modules

.PHONY: reformat
reformat:
	npx js-beautify --quiet --replace `git ls-files | grep -E '*.js$$'`

.PHONY: duktool.js
duktool.js: from-xml js-yaml
	npm install
	mkdir -p lib/extdeps
	cp from-xml/from-xml.js lib/extdeps/from-xml.js
	cp js-yaml/dist/js-yaml.js lib/extdeps/js-yaml.js
	npx webpack
	ls -l duktool.js

from-xml:
	git clone https://github.com/svaarala/from-xml

js-yaml:
	git clone https://github.com/svaarala/js-yaml
