# Big endian base image for endian testing.
#
# Some packages fail to install (e.g. segfaults), so use a pruned package list
# which allows compiling and running 'duk' but not all other targets.
#
# https://hub.docker.com/r/s390x/ubuntu

FROM s390x/ubuntu:18.04

ARG USERNAME=duktape
ARG UID=1000
ARG GID=1000

RUN echo "=== Timezone setup ===" && \
	echo "Europe/Helsinki" > /etc/timezone && \
	ln -s /usr/share/zoneinfo/Europe/Helsinki /etc/localtime

RUN echo "=== Package install ===" && \
	apt-get update && \
	apt-get install -qqy --no-install-recommends \
		build-essential llvm valgrind strace libc6-dbg \
		git w3m wget curl openssl ca-certificates \
		gcc gcc-4.8 gcc-5 gcc-6 \
		clang clang-tools clang-3.9 clang-4.0 clang-5.0 clang-6.0 clang-7 \
		python python-yaml make bc diffstat colordiff \
		zip unzip vim tzdata cdecl

RUN echo "=== User setup, /work directory creation ===" && \
	groupadd -g $GID -o $USERNAME && \
	useradd -m -u $UID -g $GID -o -s /bin/bash $USERNAME && \
	mkdir /work && chown $UID:$GID /work && chmod 755 /work && \
	echo "PS1='\033[40;37mDOCKER\033[0;34m \u@\h [\w] >>>\033[0m '" > /root/.profile && \
	echo "PS1='\033[40;37mDOCKER\033[0;34m \u@\h [\w] >>>\033[0m '" > /home/$USERNAME/.profile && \
	chown $UID:$GID /home/$USERNAME/.profile

USER $USERNAME
WORKDIR /work

COPY --chown=duktape:duktape gitconfig /home/$USERNAME/.gitconfig
COPY --chown=duktape:duktape prepare_repo.sh .
RUN chmod 644 /home/$USERNAME/.gitconfig && \
    chmod 755 prepare_repo.sh

RUN echo "=== Repo snapshots ===" && \
	mkdir /work/repo-snapshots && \
	git clone https://github.com/svaarala/duktape.git repo-snapshots/duktape && \
	git clone https://github.com/svaarala/duktape-wiki.git repo-snapshots/duktape-wiki && \
	git clone https://github.com/svaarala/duktape-releases.git repo-snapshots/duktape-releases

RUN echo "=== Prepare duktape-prep repo ===" && \
	cp -r repo-snapshots/duktape duktape-prep && \
	cp -r repo-snapshots/duktape-releases duktape-prep/duktape-releases && \
	cd duktape-prep && \
	make linenoise && \
	make clean

RUN echo "=== Versions ===" && \
	echo "GCC:" && gcc -v && \
	echo "CLANG:" && clang -v
