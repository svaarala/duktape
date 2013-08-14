/*
 *  Ncurses bindings example.
 *
 *  Debian: install libncursesw5-dev
 */

#include <curses.h>
#include "duktape.h"

int duk_ncurses_initscr(duk_context *ctx) {
	WINDOW *win;

	win = initscr();
	duk_push_pointer(ctx, (void *) win);
	return 1;
}

int duk_ncurses_endwin(duk_context *ctx) {
	int rc;

	rc = endwin();
	duk_push_int(ctx, rc);
	return 1;
}

int duk_ncurses_delscreen(duk_context *ctx) {
	/* XXX: no screen management now */
	return 0;
}

int duk_ncurses_getmaxyx(duk_context *ctx) {
	int row, col;

	getmaxyx(stdscr, row, col);

	duk_push_array(ctx);
	duk_push_int(ctx, row);
	duk_put_prop_index(ctx, -2, 0);
	duk_push_int(ctx, col);
	duk_put_prop_index(ctx, -2, 1);
	return 1;
}

int duk_ncurses_printw(duk_context *ctx) {
	int rc;
	const char *str;

	str = duk_to_string(ctx, 0);
	rc = printw("%s", str);
	duk_push_int(ctx, rc);
	return 1;
}

int duk_ncurses_mvprintw(duk_context *ctx) {
	int y = duk_to_int(ctx, 0);
	int x = duk_to_int(ctx, 1);
	const char *str = duk_to_string(ctx, 2);
	int rc;

	rc = mvprintw(y, x, "%s", str);
	duk_push_int(ctx, rc);
	return 1;
}

int duk_ncurses_refresh(duk_context *ctx) {
	int rc;

	rc = refresh();
	duk_push_int(ctx, rc);
	return 1;
}

int duk_ncurses_getch(duk_context *ctx) {
	int rc;

	rc = getch();
	duk_push_int(ctx, rc);
	return 1;
}

void duk_ncurses_register(duk_context *ctx) {
	duk_push_global_object(ctx);
	duk_push_string(ctx, "ncurses");
	duk_push_object(ctx);

	duk_push_string(ctx, "initscr");
	duk_push_c_function(ctx, duk_ncurses_initscr, 0);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "endwin");
	duk_push_c_function(ctx, duk_ncurses_endwin, 0);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "delscreen");
	duk_push_c_function(ctx, duk_ncurses_delscreen, 0);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "getmaxyx");
	duk_push_c_function(ctx, duk_ncurses_getmaxyx, 0);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "printw");
	duk_push_c_function(ctx, duk_ncurses_printw, 1);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "mvprintw");
	duk_push_c_function(ctx, duk_ncurses_mvprintw, 3);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "refresh");
	duk_push_c_function(ctx, duk_ncurses_refresh, 0);
	duk_put_prop(ctx, -3);

	duk_push_string(ctx, "getch");
	duk_push_c_function(ctx, duk_ncurses_getch, 0);
	duk_put_prop(ctx, -3);

	duk_put_prop(ctx, -3);
	duk_pop(ctx);
}

