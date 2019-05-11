FROM duktape-base-ubuntu-18.04:latest

# Dist result is written to stdout as a ZIP, which allows caller to extract
# result without uid issues (ZIP tolerates leading garbage).
CMD bash -c 'source emsdk-portable/emsdk_env.sh && \
	rm -rf duktape && \
	cp -r repo-snapshots/duktape duktape && \
	cp -r repo-snapshots/duktape-releases duktape/duktape-releases && \
	cd duktape && git pull && \
	cd duktape-releases && git pull && cd .. && \
	make dist-site && \
	ls -l duktape-site-*.tar.* && \
	zip /tmp/out.zip duktape-site-*.tar.* && \
	cat /tmp/out.zip'
