/*
 *  Example debug transport using a TCP socket
 *
 *  The application has a server socket which can be connected to.
 *  After that data is just passed through.
 *
 *  NOTE: This is Linux specific on purpose, as it's just an example how
 *  a debug transport can be concretely implemented.
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include "duktape.h"

#ifndef DUK_DEBUG_PORT
#define DUK_DEBUG_PORT 9091
#endif

#if 0
#define DEBUG_PRINTS
#endif

static int server_sock = -1;
static int client_sock = -1;

void duk_debug_trans_socket_init(void) {
	struct sockaddr_in addr;
	int on;

	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock < 0) {
		fprintf(stderr, "%s: failed to create server socket: %s\n", __FILE__, strerror(errno));
		fflush(stderr);
		goto fail;
	}

	on = 1;
	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof(on)) < 0) {
		fprintf(stderr, "%s: failed to set SO_REUSEADDR for server socket: %s\n", __FILE__, strerror(errno));
		fflush(stderr);
		goto fail;
	}

	memset((void *) &addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(DUK_DEBUG_PORT);

	if (bind(server_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		fprintf(stderr, "%s: failed to bind server socket: %s\n", __FILE__, strerror(errno));
		fflush(stderr);
		goto fail;
	}

	listen(server_sock, 1 /*backlog*/);
	return;

 fail:
	if (server_sock >= 0) {
		(void) close(server_sock);
		server_sock = -1;
	}
}

void duk_debug_trans_socket_waitconn(void) {
	struct sockaddr_in addr;
	socklen_t sz;

	if (server_sock < 0) {
		fprintf(stderr, "%s: no server socket, skip waiting for connection\n", __FILE__);
		fflush(stderr);
		return;
	}
	if (client_sock >= 0) {
		(void) close(client_sock);
		client_sock = -1;
	}

	fprintf(stderr, "Waiting for debug connection on port %d\n", (int) DUK_DEBUG_PORT);
	fflush(stderr);

	sz = (socklen_t) sizeof(addr);
	client_sock = accept(server_sock, (struct sockaddr *) &addr, &sz);
	if (client_sock < 0) {
		fprintf(stderr, "%s: accept() failed, skip waiting for connection: %s\n", __FILE__, strerror(errno));
		fflush(stderr);
		goto fail;
	}

	fprintf(stderr, "Debug connection established\n");
	fflush(stderr);

	/* XXX: For now, close the listen socket because we won't accept new
	 * connections anyway.  A better implementation would allow multiple
	 * debug attaches.
	 */

	if (server_sock >= 0) {
		(void) close(server_sock);
		server_sock = -1;
	}
	return;

 fail:
	if (client_sock >= 0) {
		(void) close(client_sock);
		client_sock = -1;
	}
}

/* Duktape debug transport callback: partial read */
duk_size_t duk_debug_trans_socket_read(void *udata, char *buffer, duk_size_t length) {
	ssize_t ret;

	(void) udata;  /* not needed by the example */

#if defined(DEBUG_PRINTS)
	fprintf(stderr, "duk_debug_trans_socket_read: udata=%p, buffer=%p, length=%ld\n",
	        (void *) udata, (void *) buffer, (long) length);
	fflush(stderr);
#endif

	if (client_sock < 0) {
		return 0;
	}

	if (length == 0) {
		/* This shouldn't happen. */
		fprintf(stderr, "%s: read request length == 0, closing connection\n", __FILE__);
		fflush(stderr);
		goto fail;
	}

	if (buffer == NULL) {
		/* This shouldn't happen. */
		fprintf(stderr, "%s: read request buffer == NULL, closing connection\n", __FILE__);
		fflush(stderr);
		goto fail;
	}

	/* In a production quality implementation there would be a sanity
	 * timeout here to recover from "black hole" disconnects.
	 */

	ret = read(client_sock, (void *) buffer, (size_t) length);
	if (ret < 0) {
		fprintf(stderr, "%s: debug read failed, errno %d, closing connection: %s\n", __FILE__, errno, strerror(errno));
		fflush(stderr);
		goto fail;
	} else if (ret == 0) {
		fprintf(stderr, "%s: debug read failed, ret == 0 (EOF), closing connection\n", __FILE__);
		fflush(stderr);
		goto fail;
	} else if (ret > (ssize_t) length) {
		fprintf(stderr, "%s: debug read failed, ret too large (%ld > %ld), closing connection\n", __FILE__, (long) ret, (long) length);
		fflush(stderr);
		goto fail;
	}

	return (duk_size_t) ret;

 fail:
	if (client_sock >= 0) {
		(void) close(client_sock);
		client_sock = -1;
	}
	return 0;
}

/* Duktape debug transport callback: partial write */
duk_size_t duk_debug_trans_socket_write(void *udata, const char *buffer, duk_size_t length) {
	ssize_t ret;

	(void) udata;  /* not needed by the example */

#if defined(DEBUG_PRINTS)
	fprintf(stderr, "duk_debug_trans_socket_write: udata=%p, buffer=%p, length=%ld\n",
	        (void *) udata, (void *) buffer, (long) length);
	fflush(stderr);
#endif

	if (client_sock < 0) {
		return 0;
	}

	if (length == 0) {
		/* This shouldn't happen. */
		fprintf(stderr, "%s: write request length == 0, closing connection\n", __FILE__);
		fflush(stderr);
		goto fail;
	}

	if (buffer == NULL) {
		/* This shouldn't happen. */
		fprintf(stderr, "%s: write request buffer == NULL, closing connection\n", __FILE__);
		fflush(stderr);
		goto fail;
	}

	/* In a production quality implementation there would be a sanity
	 * timeout here to recover from "black hole" disconnects.
	 */

	ret = write(client_sock, (const void *) buffer, (size_t) length);
	if (ret <= 0 || ret > (ssize_t) length) {
		fprintf(stderr, "%s: debug write failed, closing connection: %s\n", __FILE__, strerror(errno));
		fflush(stderr);
		goto fail;
	}

	return (duk_size_t) ret;

 fail:
	if (client_sock >= 0) {
		(void) close(client_sock);
		client_sock = -1;
	}
	return 0;
}

duk_size_t duk_debug_trans_socket_peek(void *udata) {
	struct pollfd fds[1];
	int poll_rc;

	(void) udata;  /* not needed by the example */

#if defined(DEBUG_PRINTS)
	fprintf(stderr, "duk_debug_trans_socket_peek: udata=%p\n", (void *) udata);
	fflush(stderr);
#endif

	fds[0].fd = client_sock;
	fds[0].events = POLLIN;
	fds[0].revents = 0;

	poll_rc = poll(fds, 1, 0);
	if (poll_rc < 0) {
		fprintf(stderr, "%s: poll returned < 0, closing connection: %s\n", __FILE__, strerror(errno));
		fflush(stderr);
		goto fail;  /* also returns 0, which is correct */
	} else if (poll_rc > 1) {
		fprintf(stderr, "%s: poll returned > 1, treating like 1\n", __FILE__);
		fflush(stderr);
		return 1;  /* should never happen */
	} else if (poll_rc == 0) {
		return 0;  /* nothing to read */
	} else {
		return 1;  /* something to read */
	}

 fail:
	if (client_sock >= 0) {
		(void) close(client_sock);
		client_sock = -1;
	}
	return 0;
}

void duk_debug_trans_socket_read_flush(void *udata) {
#if defined(DEBUG_PRINTS)
	fprintf(stderr, "duk_debug_trans_socket_read_flush: udata=%p\n", (void *) udata);
	fflush(stderr);
#endif

	(void) udata;  /* not needed by the example */

	/* Read flush: Duktape may not be making any more read calls at this
	 * time.  If the transport maintains a receive window, it can use a
	 * read flush as a signal to update the window status to the remote
	 * peer.  A read flush is guaranteed to occur before Duktape stops
	 * reading for a while; it may occur in other situations as well so
	 * it's not a 100% reliable indication.
	 */

	/* This TCP transport requires no read flush handling so ignore.
	 * You can also pass a NULL to duk_debugger_attach() and not
	 * implement this callback at all.
	 */
}

void duk_debug_trans_socket_write_flush(void *udata) {
#if defined(DEBUG_PRINTS)
	fprintf(stderr, "duk_debug_trans_socket_write_flush: udata=%p\n", (void *) udata);
	fflush(stderr);
#endif

	(void) udata;  /* not needed by the example */

	/* Write flush.  If the transport combines multiple writes
	 * before actually sending, a write flush is an indication
	 * to write out any pending bytes: Duktape may not be doing
	 * any more writes on this occasion.
	 */

	/* This TCP transport requires no write flush handling so ignore.
	 * You can also pass a NULL to duk_debugger_attach() and not
	 * implement this callback at all.
	 */
	return;
}
