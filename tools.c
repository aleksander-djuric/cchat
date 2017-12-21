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

int id_add(char *dst, int dst_size, int id) {
	char *p;
	int len;

	if (!dst) return -1;

	len = strlen(dst);
	p = dst + len;

	if ((len + sizeof(int)) > dst_size) return -1;

	if (*dst != '\0') *p++ = ' ';
	snprintf(p, dst_size - len, "%d", id);

	return 0;
}

int id_del(char *dst, int id) {
	char str[10];
	char *p;
	int len;

	if (!dst || *dst == '\0') return -1;

	len = sprintf(str, "%d",  id);
	if ((p = strstr(dst, str))) {
		if (*(p + len) == '\0') *p = '\0';
		else strcpy(p, p + len + 1);
	}

	return 0;
}

int reg_client(int qid, int uid) {
	msgdata_t reg;

	if (msgrcv(qid, (void *) &reg, MSGSIZE, CTRL_ID, IPC_NOWAIT) < 0) {
		reg.id = 0;
		reg.data[0] = '\0';
	}

	reg.id += 1;
	reg.type = CTRL_ID;
	if (id_add(reg.data, MSGSIZE, uid) < 0 ||
		msgsnd(qid, (void*) &reg, MSGSIZE, IPC_NOWAIT) < 0) {
		return -1;
	}

	return reg.id;
}

int unreg_client(int qid, int uid) {
	msgdata_t reg;
	struct msqid_ds qstatus;
	char *p;

	if (msgrcv(qid, (void *) &reg, MSGSIZE, CTRL_ID, IPC_NOWAIT) < 0)
		return 0;

	if (reg.id) reg.id -= 1;
	reg.type = CTRL_ID;
	if (id_del(reg.data, uid) < 0) return -1;

	if (msgctl(qid, IPC_STAT, &qstatus) == 0 &&
		uid == qstatus.msg_perm.uid) {
		// try to delegate mq to the next active user
		if ((p = strtok(reg.data, " "))) {
			uid = atoi(p);
			qstatus.msg_perm.uid = (uid_t) uid;
			qstatus.msg_perm.gid = uid;
			msgctl(qid, IPC_SET, &qstatus);
		} else msgctl(qid, IPC_RMID, 0);
	}

	if (msgsnd(qid, (const void*) &reg, MSGSIZE, IPC_NOWAIT) < 0)
		return -1;

	return reg.id;
}

int get_count(int qid) {
	msgdata_t reg;

	if (msgrcv(qid, (void *) &reg, MSGSIZE, CTRL_ID, IPC_NOWAIT) < 0)
		return 0;

	reg.type = CTRL_ID;
	if (msgsnd(qid, (void*) &reg, MSGSIZE, IPC_NOWAIT) < 0)
		return 0;

	return reg.id;
}

int msg_send(int qid, int uid, msgdata_t *msgp) {
	msgdata_t reg;
	char *p;

	if (msgrcv(qid, (void *) &reg, MSGSIZE, CTRL_ID, IPC_NOWAIT) < 0)
		return -1;

	reg.type = CTRL_ID;
	if (msgsnd(qid, (void*) &reg, MSGSIZE, IPC_NOWAIT) < 0)
		return -1;

	msgp->id = uid;
	p = strtok(reg.data, " ");
	while (p) {
		msgp->type = atoi(p);
		if (msgp->type != uid &&
			msgsnd(qid, (void*) msgp, MSGSIZE, 0) < 0) {
			return -1;
		}
		p = strtok(NULL, " ");
	}

	return 0;
}
