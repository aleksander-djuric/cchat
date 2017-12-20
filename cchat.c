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
	WINDOW *p;
	struct msqid_ds qstatus;
	msgdata_t msg;
	int maxrows, maxcols, start, width, height;
	int qid, uid, cmd, ret;
	int running = 1;
	key_t key;

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

	if (msgctl(qid, IPC_STAT, &qstatus) < 0) {
		printf("Couldn't get message queue status: %s\n",
			strerror(errno));
		return 0;
	}

	if ((ret = reg_client(qid, uid)) < 0) {
		printf("Couldn't register chat client: %s\n",
			strerror(errno));
		if (uid == qstatus.msg_perm.uid)
			msgctl(qid, IPC_RMID, 0);
		return 0;
	}

	ncstart();

	maxrows = 1;
	maxcols = DATASIZE;
	start = 0;
	width = COLS - 1;
	height = LINES - 2;

	p = newpad(maxrows, maxcols);
	wbkgd(p, COLOR_PAIR(1));
	scrollok(p, true);

	mvhline(LINES-2, 0, ACS_HLINE, COLS);
//	mvprintw(LINES-1, 0, ": ");

	add_line(p, &maxrows, &maxcols, 8);
	print_system(p, COLOR_PAIR(3), "CChat v."VERSION" Copyright (c) 2017 Aleksander Djuric.\n");
	print_system(p, COLOR_PAIR(3), "Chat created, message queue id %d\n", qid);
	print_system(p, COLOR_PAIR(3), "Chat created by user with user id: %d\n", qstatus.msg_perm.uid);
	print_system(p, COLOR_PAIR(3), "Chat created by user with group id: %d\n", qstatus.msg_perm.gid);
	print_system(p, COLOR_PAIR(3), "%d message(s) in chat\n", (int) qstatus.msg_qnum);
	print_system(p, COLOR_PAIR(3), "%d user(s) in chat\n", ret);
	print_system(p, COLOR_PAIR(3), "Press \'Esc\' key to write message. Type \'exit\' to quit.\n\n", ret);
	refresh();

	while (running) {
		prefresh(p, start, 0, 0, 0, height, width);

		cmd = getch();
		switch (cmd) {
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
			
			move(LINES-1, 0);
			clrtoeol();
			add_line(p, &maxrows, &maxcols, 1);
			print_msg(p, uid, msg.data);
			if ((ret = msg_send(qid, uid, &msg)) < 0) {
				add_line(p, &maxrows, &maxcols, 1);
				if (ret == -1)
					print_system(p, COLOR_PAIR(4), "Couldn't send message\n");
				else print_system(p, COLOR_PAIR(4), "Chat is not available\n");
			}
			refresh();
			break;
		case KEY_RESIZE:
			clear();

			width = COLS - 1;
			height = LINES - 2;
			prefresh(p, start, 0, 0, 0, height, width);

			mvhline(LINES-2, 0, ACS_HLINE, COLS);
			mvprintw(LINES-1, 0, ":");

			refresh();
			break;
		case ERR:
			break;
		default:
			break;
		}

		if (msgrcv(qid, (void *) &msg, MSGSIZE, uid, IPC_NOWAIT) > 0) {
			add_line(p, &maxrows, &maxcols, 1);
			print_msg(p, msg.id, msg.data);
		}

		usleep(500);
	}

	unreg_client(qid, uid);

	if (uid == qstatus.msg_perm.uid)
		msgctl(qid, IPC_RMID, 0);

	delwin(p);
	endwin();
	return 0;
}
