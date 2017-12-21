/*
 * view.c
 *
 * Description: Unix Chat using Message Queue
 * Copyright (c) 2017 Aleksander Djuric. All rights reserved.
 * Distributed under the GNU Lesser General Public License (LGPL).
 * The complete text of the license can be found in the LICENSE
 * file included in the distribution.
 *
 */

#include "cchat.h"

void ncstart() {
	initscr();
	keypad(stdscr, true);
	noecho();
	cbreak();


	if (has_colors()) {
		start_color();
		use_default_colors();

		init_pair(1, COLOR_CYAN, COLOR_BLACK);
		init_pair(2, COLOR_BLACK, COLOR_CYAN);
		init_pair(3, COLOR_GREEN, COLOR_BLACK);
		init_pair(4, COLOR_BLUE, COLOR_BLACK);
		init_pair(5, COLOR_RED, COLOR_BLACK);
	}

	wbkgd(stdscr, COLOR_PAIR(1));
	refresh();
	curs_set(1);
	set_escdelay(0);
	timeout(0);
}

void draw_statbar(WINDOW *status, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	wclear(status);
	vwprintw(status, fmt, args);
	va_end(args);
}

void add_line(WINDOW *win, int *maxrows, int *maxcols, int count) {
	wresize(win, *maxrows + count, *maxcols);
	*maxrows += count;
}

void print_msg(WINDOW *win, int uid, char *msg) {
	struct passwd *pw;

	if ((pw = getpwuid(uid))) {
		wattron(win, COLOR_PAIR(3));
		wprintw(win, "%s: ", pw->pw_name);
		wattroff(win, COLOR_PAIR(3));
	} else {
		wattron(win, COLOR_PAIR(4));
		wprintw(win, "%4d: ", uid);
		wattroff(win, COLOR_PAIR(4));
	}

	wprintw(win, "%s\n", msg);
}

void print_system(WINDOW *win, int attrs, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	wattron(win, attrs);
	vwprintw(win, fmt, args);
	wattroff(win, attrs);

	va_end(args);
}
