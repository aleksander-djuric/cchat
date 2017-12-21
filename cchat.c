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
	int qid, uid, cmd, count;
	int running = 1;
	int update = 1;
	key_t key;

	setlocale(LC_ALL, "");

	uid = (int) getuid();
	if (uid < 0 || CTRL_ID <= uid) {
		printf("User Id must be in range 1 .. %d\n", CTRL_ID - 1);
		return 0;
	}

	if (!uid) uid = 1; // allow root as 'bin'

	if ((key = ftok(argv[0], 0)) < 0) {
		printf("Can't generate message queue key: %s\n",
			strerror(errno));
		return 0;
	}

	qid = msgget(key, (IPC_CREAT | 0666));
	if (qid < 0) {
		printf("Couldn't create message queue %d: %s\n", qid,
			strerror(errno));
		return 0;
	}

	if ((count = reg_client(qid, uid)) < 0) {
		printf("Couldn't register chat client: %s\n",
			strerror(errno));
			msgctl(qid, IPC_RMID, 0);
		return 0;
	}

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

	add_line(p, &maxrows, &maxcols, 3);
	print_system(p, COLOR_PAIR(1), "Copyright (c) 2017 Aleksander Djuric. All rights reserved.\n");
	print_system(p, COLOR_PAIR(1), "Press \'Esc\' key to write message. Type \'exit\' to quit.\n\n");

	refresh();
	wrefresh(status);

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
		case 27: // escape
			timeout(-1);
			curs_set(2);
			echo();
			mvprintw(LINES-1, 0, ": ");
			getnstr(msg.data, DATASIZE);
			if (!strcmp(msg.data, "quit") ||
				!strcmp(msg.data, "exit")) {
				running = 0;
				break;
			}
			noecho();
			curs_set(1);
			timeout(0);

			move(LINES - 1, 0);
			clrtoeol();
			add_line(p, &maxrows, &maxcols, 1);
			print_msg(p, uid, msg.data);
			if (msg_send(qid, uid, &msg) < 0) {
				add_line(p, &maxrows, &maxcols, 1);
				print_system(p, COLOR_PAIR(5), "Couldn't send message\n");
			}
			if (lines > height)
				start = lines - height;
			refresh();
			update = 1;
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
			break;
		}

		if (msgrcv(qid, (void *) &msg, MSGSIZE, uid, IPC_NOWAIT) > 0) {
			add_line(p, &maxrows, &maxcols, 1);
			print_msg(p, msg.id, msg.data);
			update = 1;
		}

		if (update) {
			count = get_count(qid);
			draw_statbar(status, "CChat v."VERSION" | %s | %d user(s) in chat | Message queue id: %d",
				(count > 0) ? "Online" : "Offline", count, qid);
			wrefresh(status);
			update = 0;
		}

		usleep(2000);
	}

	unreg_client(qid, uid);

	delwin(p);
	endwin();
	return 0;
}
