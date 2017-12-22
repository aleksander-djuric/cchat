/*
 * tools.c
 *
 * Description: Unix Chat using Message Queue
 * Copyright (c) 2017 Aleksander Djuric. All rights reserved.
 * Distributed under the GNU Lesser General Public License (LGPL).
 * The complete text of the license can be found in the LICENSE
 * file included in the distribution.
 *
 */

#include "cchat.h"

int reg_client(int uid, int *data) {
	if (data[0] < 0) return -1;
	if (data[0] == MAX_USERS) return -1;

	data[0]++, data[data[0]] = uid;

	return data[0];
}

int unreg_client(int uid, int *data) {
	int *s = data + 1;
	int *e = s + data[0];

	if (data[0] < 0) return -1;
	if (data[0] == 0) return 0;

	for (; *s != uid; s++) {
		if (s == e) return data[0];
	}

	if (s + 1 != e)
		memmove(s, s + 1, (e - s) * sizeof(int));
	data[0]--;

	return data[0];
}

int get_count(int *data) { return data[0]; }

int msg_send(int qid, int uid, int *data, msgdata_t *msgp) {
	int *s = data + 1;
	int *e = s + data[0];

	for (; s < e; s++) {
		msgp->from = uid;
		msgp->to = *s;
		if (msgp->to != uid) {
			msgp->to += 1;
			if (msgsnd(qid, (void*) msgp, MSGSIZE, 0) < 0)
				return -1;
		}
	}

	return 0;
}
