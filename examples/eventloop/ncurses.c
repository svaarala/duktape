/*
 *  Ncurses bindings example.
 *
 *  VALGRIND NOTE: when you use ncurses, there seems to be no way to get a
 *  clean valgrind run.  Even if ncurses state is properly shut down, there
 *  will still be some residual leaks.
 *
 *  Debian: install libncurses5-dev
 */

#include <curses.h>
#include "duktape.h"

static int ncurses_initscr(duk_context *ctx) {
	WINDOW *win;

	win = initscr();
	duk_push_pointer(ctx, (void *) win);
	return 1;
}

static int ncurses_endwin(duk_context *ctx) {
	int rc;

	rc = endwin();
	duk_push_int(ctx, rc);
	return 1;
}

static int ncurses_delscreen(duk_context *ctx) {
	/* XXX: no screen management now */
	(void) ctx;
	return 0;
}

static int ncurses_getmaxyx(duk_context *ctx) {
	int row, col;

	getmaxyx(stdscr, row, col);

	duk_push_array(ctx);
	duk_push_int(ctx, row);
	duk_put_prop_index(ctx, -2, 0);
	duk_push_int(ctx, col);
	duk_put_prop_index(ctx, -2, 1);
	return 1;
}

static int ncurses_printw(duk_context *ctx) {
	int rc;
	const char *str;

	str = duk_to_string(ctx, 0);
	rc = printw("%s", str);
	duk_push_int(ctx, rc);
	return 1;
}

static int ncurses_mvprintw(duk_context *ctx) {
	int y = duk_to_int(ctx, 0);
	int x = duk_to_int(ctx, 1);
	const char *str = duk_to_string(ctx, 2);
	int rc;

	rc = mvprintw(y, x, "%s", str);
	duk_push_int(ctx, rc);
	return 1;
}

static int ncurses_refresh(duk_context *ctx) {
	int rc;

	rc = refresh();
	duk_push_int(ctx, rc);
	return 1;
}

static int ncurses_getch(duk_context *ctx) {
	int rc;

	rc = getch();
	duk_push_int(ctx, rc);
	return 1;
}

void ncurses_register(duk_context *ctx) {
	duk_push_global_object(ctx);
	duk_push_string(ctx, "Ncurses");
	duk_push_object(ctx);

	duk_push_c_function(ctx, ncurses_initscr, 0);
	duk_put_prop_string(ctx, -2, "initscr");

	duk_push_c_function(ctx, ncurses_endwin, 0);
	duk_put_prop_string(ctx, -2, "endwin");

	duk_push_c_function(ctx, ncurses_delscreen, 0);
	duk_put_prop_string(ctx, -2, "delscreen");

	duk_push_c_function(ctx, ncurses_getmaxyx, 0);
	duk_put_prop_string(ctx, -2, "getmaxyx");

	duk_push_c_function(ctx, ncurses_printw, 1);
	duk_put_prop_string(ctx, -2, "printw");

	duk_push_c_function(ctx, ncurses_mvprintw, 3);
	duk_put_prop_string(ctx, -2, "mvprintw");

	duk_push_c_function(ctx, ncurses_refresh, 0);
	duk_put_prop_string(ctx, -2, "refresh");

	duk_push_c_function(ctx, ncurses_getch, 0);
	duk_put_prop_string(ctx, -2, "getch");

	duk_put_prop(ctx, -3);
	duk_pop(ctx);
}

