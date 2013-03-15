/*
 *  TCP sockets binding example.
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <time.h>

#include "duk_api.h"

int duk_socket_create_server_socket(duk_context *ctx) {
	const char *addr = duk_to_string(ctx, 0);
	int port = duk_to_int(ctx, 1);
	int sock;
	struct sockaddr_in sockaddr;
	struct hostent *ent;
	struct in_addr **addr_list;
	struct in_addr *addr_inet;
	int i;
	int rc;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		duk_error(ctx, 1 /*FIXME*/, "%s (errno=%d)", strerror(errno), errno);
	}

	ent = gethostbyname(addr);
	if (!ent) {
		duk_error(ctx, 1 /*FIXME*/, "%s (errno=%d)", strerror(errno), errno);
	}

	addr_list = (struct in_addr **) ent->h_addr_list;
	addr_inet = NULL;
	for (i = 0; addr_list[i]; i++) {
		addr_inet = addr_list[i];
		break;
	}
	if (!addr_inet) {
		duk_error(ctx, 1 /*FIXME*/, "cannot resolve %s", addr);
	}

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr = *addr_inet;

	rc = bind(sock, (const struct sockaddr *) &sockaddr, sizeof(sockaddr));
	if (rc < 0) {
		duk_error(ctx, 1 /*FIXME*/, "%s (errno=%d)", strerror(errno), errno);
	}

	rc = listen(sock, 10 /*backlog*/);
	if (rc < 0) {
		(void) close(sock);
		duk_error(ctx, 1 /*FIXME*/, "%s (errno=%d)", strerror(errno), errno);
	}

	duk_push_int(ctx, sock);
	return 1;
}

int duk_socket_close(duk_context *ctx) {
	int sock = duk_to_int(ctx, 0);
	int rc;

	rc = close(sock);
	if (rc < 0) {
		duk_error(ctx, 1 /*FIXME*/, "%s (errno=%d)", strerror(errno), errno);
	}
	return 0;
}

int duk_socket_accept(duk_context *ctx) {
	int sock = duk_to_int(ctx, 0);
	int rc;
	struct sockaddr_in addr;
	socklen_t addrlen;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addrlen = sizeof(addr);

	rc = accept(sock, (struct sockaddr *) &addr, &addrlen);
	if (rc < 0) {
		duk_error(ctx, 1 /*FIXME*/, "%s (errno=%d)", strerror(errno), errno);
	}

	if (addrlen == sizeof(addr)) {
		uint32_t tmp = ntohl(addr.sin_addr.s_addr);

		duk_push_new_object(ctx);

		duk_push_string(ctx, "fd");
		duk_push_int(ctx, rc);
		duk_put_prop(ctx, -3);
		duk_push_string(ctx, "addr");
		duk_push_sprintf(ctx, "%d.%d.%d.%d", ((tmp >> 24) & 0xff), ((tmp >> 16) & 0xff), ((tmp >> 8) & 0xff), (tmp & 0xff));
		duk_put_prop(ctx, -3);
		duk_push_string(ctx, "port");
		duk_push_int(ctx, ntohs(addr.sin_port));
		duk_put_prop(ctx, -3);

		return 1;
	}
	
	return 0;
}

int duk_socket_read(duk_context *ctx) {
	int sock = duk_to_int(ctx, 0);
	char readbuf[1024];
	int rc;
	void *data;

	rc = recvfrom(sock, (void *) readbuf, sizeof(readbuf), 0, NULL, NULL);
	if (rc < 0) {
		duk_error(ctx, 1 /*FIXME*/, "%s (errno=%d)", strerror(errno), errno);
	}

	data = duk_push_new_fixed_buffer(ctx, rc);
	memcpy(data, readbuf, rc);
	return 1;
}

int duk_socket_write(duk_context *ctx) {
	int sock = duk_to_int(ctx, 0);
	const char *data;
	size_t len;
	ssize_t rc;

	data = duk_to_buffer(ctx, 1, &len);

	/* MSG_NOSIGNAL: avoid SIGPIPE */
#ifdef __APPLE__
	/* FIXME: must use socket options to get same behavior */
	rc = sendto(sock, (void *) data, len, 0, NULL, 0);
#else
	rc = sendto(sock, (void *) data, len, MSG_NOSIGNAL, NULL, 0);
#endif
	if (rc < 0) {
		duk_error(ctx, 1 /*FIXME*/, "%s (errno=%d)", strerror(errno), errno);
	}

	duk_push_int(ctx, rc);
	return 1;
}

int duk_socket_poll(duk_context *ctx) {
	int timeout = duk_to_int(ctx, 1);
	int i, n, nchanged;
	int fd, rc;
	struct pollfd fds[20];
	struct timespec ts;

	memset(fds, 0, sizeof(fds));

	n = 0;
	duk_enum(ctx, 0, 0 /*enum_flags*/);
	while (duk_next(ctx, -1, 0)) {
		if (n >= sizeof(fds) / sizeof(struct pollfd)) {
			return -1;
		}

		/* [... enum key] */
		duk_dup_top(ctx);  /* -> [... enum key key] */
		duk_get_prop(ctx, 0);  /* -> [... enum key val] */
		fd = duk_to_int(ctx, -2);

		duk_push_string(ctx, "events");
		duk_get_prop(ctx, -2);  /* -> [... enum key val events] */

		fds[n].fd = fd;
		fds[n].events = duk_to_int(ctx, -1);
		fds[n].revents = 0;

		duk_pop_n(ctx, 3);  /* -> [... enum] */

		n++;
	}
	/* leave enum on stack */

	memset(&ts, 0, sizeof(ts));
	ts.tv_nsec = (timeout % 1000) * 1000000;
	ts.tv_sec = timeout / 1000;

	/*rc = ppoll(fds, n, &ts, NULL);*/
	rc = poll(fds, n, timeout);
	if (rc < 0) {
		duk_error(ctx, 1 /*FIXME*/, "%s (errno=%d)", strerror(errno), errno);
	}

	duk_push_new_array(ctx);
	nchanged = 0;
	for (i = 0; i < n; i++) {
		/* update revents */

		if (fds[i].revents) {
			/* FIXME: need an duk_array_push */
			duk_push_int(ctx, fds[i].fd);  /* -> [... retarr fd] */
			duk_put_prop_index(ctx, -2, nchanged);
			nchanged++;
		}

		duk_push_int(ctx, fds[i].fd);  /* -> [... retarr key] */
		duk_get_prop(ctx, 0);  /* -> [... retarr val] */
		duk_push_string(ctx, "revents");
		duk_push_int(ctx, fds[i].revents);  /* -> [... retarr val "revents" fds[i].revents] */
		duk_put_prop(ctx, -3);  /* -> [... retarr val] */
		duk_pop(ctx);
	}

	/* [retarr] */

	return 1;
}


void duk_socket_register(duk_context *ctx) {
	duk_push_global_object(ctx);
	duk_push_string(ctx, "socket");
	duk_push_new_object(ctx);

	duk_push_string(ctx, "createServerSocket");
	duk_push_new_c_function(ctx, duk_socket_create_server_socket, 2);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "close");
	duk_push_new_c_function(ctx, duk_socket_close, 1);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "accept");
	duk_push_new_c_function(ctx, duk_socket_accept, 1);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "read");
	duk_push_new_c_function(ctx, duk_socket_read, 1);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "write");
	duk_push_new_c_function(ctx, duk_socket_write, 2);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "poll");
	duk_push_new_c_function(ctx, duk_socket_poll, 2);
	duk_put_prop(ctx, -3);

	duk_put_prop(ctx, -3);
	duk_pop(ctx);
}

