# Some useful development targets.

.PHONY: dist
dist:
	rm -f testrunner-server-node.tar.gz
	rm -f testrunner-client-simple-node.tar.gz
	tar cvfz testrunner-server-node.tar.gz \
		server-node/*.js \
		server-node/package.json \
		server-node/config.yaml.template
	tar cvfz testrunner-client-simple-node.tar.gz \
		client-simple-node/*.js \
		client-simple-node/*.sh \
		client-simple-node/package.json \
		client-simple-node/config.yaml.template
