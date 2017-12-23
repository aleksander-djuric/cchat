/*
 * cchat.c
 *
 * Description: Unix Chat using Message Queue
 * Copyright (c) 2017 Aleksander Djuric. All rights reserved.
 * Distributed under the GNU Lesser General Public License (LGPL).
 * The complete text of the license can be found in the LICENSE
 * file included in the distribution.
 *
 */

#include "cchat.h"
#include "view.h"
#include "tools.h"
#include "config.h"

int main (int argc, char **argv) {
	WINDOW *p, *status;
	msgdata_t msg;
	int maxrows, maxcols, start, width, height;
	int uid, cmd, count;
	int running = 1;
	int update = 1;

	setlocale(LC_ALL, "");

	uid = (int) getuid();
	if (ipc_start(uid, argv[0]) < 0)
		return 0;

	ncstart();

	maxrows = 1;
	maxcols = DATASIZE;
	start = 0;
	width = COLS - 1;
	height = LINES - 3;

	status = newwin(1, COLS, 0, 0);
	wbkgd(status, COLOR_PAIR(2));

	p = newpad(maxrows, maxcols);
	wbkgd(p, COLOR_PAIR(1));
	scrollok(p, true);
	mvhline(LINES - 2, 0, ACS_HLINE, COLS);
	mvprintw(LINES - 1, 0, ": ");

	add_line(p, &maxrows, maxcols, 3);
	print_system(p, COLOR_PAIR(1), "Copyright (c) 2017 Aleksander Djuric. All rights reserved.\n");
	print_system(p, COLOR_PAIR(1), "Please type \'help\' for help. Type \'exit\' to quit.\n\n");

	refresh();
	wrefresh(status);

	sprintf(msg.data, "came to chat");
	msg_send(uid, &msg);

	while (running) {
		int lines = maxrows - 1;
		prefresh(p, start, 0, 1, 0, height, width);

		cmd = getch();
		switch (cmd) {
		case KEY_UP:
			if (start != 0) start--;
			break;
		case KEY_DOWN:
			if (start < lines) start++;
			break;
		case KEY_HOME:
			start = 0;
			break;
		case KEY_END:
			if (lines > height)
				start = lines - height;
			break;
		case KEY_PPAGE:
			if (start > height)
				start -= height;
			else start = 0;
			break;
		case KEY_NPAGE:
			if ((lines - start) / height > 1)
				start += height;
			else start = lines - height;
			break;
		case KEY_RESIZE:
			clear();
			wresize(status, 1, COLS);

			width = COLS - 1;
			height = LINES - 3;
			prefresh(p, start, 0, 1, 0, height, width);

			mvhline(LINES - 2, 0, ACS_HLINE, COLS);
			mvprintw(LINES - 1, 0, ": ");

			refresh();
			update = 1;
			break;
		case ERR:
			break;
		default:
			if (iscntrl(cmd)) continue;

			ungetch(cmd);
			timeout(-1);
			curs_set(2);
			echo();
			mvgetnstr(LINES - 1, 2, msg.data, DATASIZE);
			noecho();
			curs_set(1);
			timeout(0);
			move(LINES - 1, 2);
			clrtoeol();

			if (!strcmp(msg.data, "help")) {
				add_line(p, &maxrows, maxcols, 1);
				print_system(p, COLOR_PAIR(1), "Supporting commands list: \'exit\', \'quit\', \'help\'.\n");
				break;
			} else if (!strcmp(msg.data, "quit") ||
				!strcmp(msg.data, "exit")) {
				sprintf(msg.data, "leaves the chat");
				msg_send(uid, &msg);
				running = 0;
				break;
			}

			add_line(p, &maxrows, maxcols, 1);
			print_msg(p, uid, msg.data);
			if (msg_send(uid, &msg) < 0) {
				add_line(p, &maxrows, maxcols, 1);
				print_system(p, COLOR_PAIR(5), "Couldn't send message\n");
			}
			if (lines > height)
				start = lines - height;
			refresh();
			update = 1;
			break;
		}

		if (msg_recv(uid, &msg) > 0) {
			add_line(p, &maxrows, maxcols, 1);
			print_msg(p, msg.from, msg.data);
			update = 1;
		}

		if (update) {
			count = get_count();
			draw_statbar(status, "CChat v."VERSION" | %s | %d user(s) in chat |",
				(count > 0) ? "Online" : "Offline", count);
			wrefresh(status);
			update = 0;
		}

		usleep(1000);
	}

	ipc_finish(uid);

	delwin(p);
	endwin();
	return 0;
}
