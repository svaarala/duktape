# Docker image to run clang-format on input source files.

FROM ubuntu:20.04

# Username is assumed to be 'duktape' for now, change only UID/GID if required.
ARG USERNAME=duktape
ARG UID=1000
ARG GID=1000

RUN echo "=== Package installation ===" && \
	apt-get update && apt-get install -qqy clang-format-12 zip unzip

# Add non-root uid/gid to image, replicating host uid/gid if possible.
RUN echo "=== User setup, /work directory creation ===" && \
	groupadd -g $GID -o $USERNAME && \
	useradd -m -u $UID -g $GID -o -s /bin/bash $USERNAME && \
	mkdir /work && chown $UID:$GID /work && chmod 755 /work && \
	echo "PS1='\033[40;37mDOCKER\033[0;34m \u@\h [\w] >>>\033[0m '" > /root/.profile && \
	echo "PS1='\033[40;37mDOCKER\033[0;34m \u@\h [\w] >>>\033[0m '" > /home/$USERNAME/.profile && \
	chown $UID:$GID /home/$USERNAME/.profile

# Switch to non-root user.  (Note that COPY will still copy files as root,
# so use COPY --chown for files copied.)
USER $USERNAME

# Use /work for builds, temporaries, etc.
WORKDIR /work

COPY --chown=duktape:duktape run.sh .
RUN chmod 755 run.sh

# ZIP in, ZIP out.
ENTRYPOINT ["/work/run.sh"]
