FROM ubuntu:18.04

# Set timezone to Europe/Helsinki, some tests depend on it.  Must be done
# first to avoid interactive prompts in tzdata install.
RUN echo "=== Timezone setup ===" && \
	echo "Europe/Helsinki" > /etc/timezone && \
	ln -s /usr/share/zoneinfo/Europe/Helsinki /etc/localtime

# Node.js and packages.  This set should cover everything necessary to build
# Duktape, the duktape.org website, run tests, etc.
RUN echo "=== Node.js and package install ===" && \
	apt-get update && apt-get install -qy curl && \
	curl -sL https://deb.nodesource.com/setup_10.x | bash - && \
	apt-get update && \
	apt-get install -qy \
		build-essential gcc gcc-multilib clang clang-tools llvm valgrind \
		python python-yaml make git bc ant diffstat colordiff nodejs \
		python-bs4 tidy wget curl source-highlight rst2pdf openjdk-11-jre \
		zip unzip genisoimage vim w3m screen tzdata

# Use /build for builds, temporaries, etc.
WORKDIR /build

# Emscripten.  Source emsdk-portable/emsdk_env.sh to get 'emcc' into PATH.
RUN echo "=== Emscripten ===" && \
	wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz && \
	tar xvfz emsdk-portable.tar.gz && \
	cd emsdk-portable && \
	./emsdk update && \
	./emsdk install latest && \
	./emsdk activate latest && \
	cd .. && \
	rm emsdk-portable.tar.gz

# Clone some useful repositories into 'repo-snapshots' to reduce network
# traffic.  Derived images can then just 'git pull' to get them up to date.
RUN echo "=== Repo snapshots ===" && \
	mkdir /build/repo-snapshots && \
	git clone https://github.com/svaarala/duktape.git repo-snapshots/duktape && \
	git clone https://github.com/svaarala/duktape-wiki.git repo-snapshots/duktape-wiki && \
	git clone https://github.com/svaarala/duktape-releases.git repo-snapshots/duktape-releases

# /build/duktape-prep is prepared to be close to master in case the derived
# image needs master.  Copy it from the repo snapshot, but leave the snapshot
# intact.  Try to minimize network traffic by pulling in some dependencies into
# the image.
RUN echo "=== Prepare duktape-prep repo ===" && \
	cp -r repo-snapshots/duktape duktape-prep && \
	cp -r repo-snapshots/duktape-releases duktape-prep/duktape-releases && \
	cd duktape-prep && \
	make runtestsdeps linenoise UglifyJS2 && \
	make duktape-releases lz-string jquery-1.11.2.js && \
	make emscripten && emscripten/emcc -s WASM=0 -o /tmp/test_mandel2.js misc/test_mandel2.c && ls -l /tmp/test_mandel2.js && rm /tmp/test_mandel2.* && \
	make luajs underscore lodash bluebird.js closure-compiler && \
	make clean

# Shared script for setting up the duktape/ directory.
COPY prepare_repo.sh .
RUN chmod 755 prepare_repo.sh

# Dump some versions.
RUN echo "=== Versions ===" && \
	echo "GCC:" && gcc -v && \
	echo "CLANG:" && clang -v && \
	bash -c 'echo "EMCC:" && . emsdk-portable/emsdk_env.sh && emcc -v'
