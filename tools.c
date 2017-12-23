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

static struct sembuf sem[2];
static int sid, qid, mid;
static int *data;

#ifdef _SEM_SEMUN_UNDEFINED
union semun {
	int val;
	struct semid_ds *buf;
};
#endif

int ipc_start(int uid, char *path) {
	key_t key;

	// generate the ipc key
	if ((key = ftok(path, 0)) < 0) {
		printf("Can't generate message queue key: %s\n", strerror(errno));
		return -1;
	}

	// create message queue
	if ((qid = msgget(key, IPC_CREAT | 0666)) < 0) {
		printf("Can't find or create message queue: %s\n", strerror(errno));
		return -1;
	}

	// get shared memory
	if ((mid = shmget(key, (MAX_USERS + 1) * sizeof(int), IPC_CREAT | 0666)) < 0) {
		printf("Can't find or allocate shared memory: %s\n", strerror(errno));
		msgctl(qid, IPC_RMID, 0);
		return -1;
	}

	// attach shared memory
	if (*(data = shmat(mid, 0, 0)) < 0) {
		printf("Couldn't attach shared memory: %s\n", strerror(errno));
		shmctl(mid, IPC_RMID, 0);
		msgctl(qid, IPC_RMID, 0);
		return -1;
	}

	// set up the semaphore
	if ((sid = semget(key, 1, IPC_CREAT | 0666)) < 0) {
		printf("Can't setup semaphore: %s\n", strerror(errno));
		shmctl(mid, IPC_RMID, 0);
		msgctl(qid, IPC_RMID, 0);
		return -1;
	}

	sem[0].sem_num = 0;
	sem[1].sem_num = 0;
	sem[0].sem_flg = SEM_UNDO;
	sem[1].sem_flg = SEM_UNDO;

	if (data[0] == MAX_USERS) {
		printf("Couldn't register chat client: %s\n", strerror(errno));
		return -1;
	}

	// register chat client
	sem[0].sem_op = 0; // wait for zero
	sem[1].sem_op = 1; // then, inc to lock it
	semop(sid, sem, 2);

	data[0]++, data[data[0]] = uid;

	sem[0].sem_op = -1; // dec to unlock
	semop(sid, sem, 1);

	return 0;
}

void ipc_finish(int uid) {
	union semun sopts;
	struct semid_ds sstatus;
	struct msqid_ds qstatus;
	struct shmid_ds mstatus;
	struct passwd *pw;
	int *s = data + 1;
	int *e = s + data[0];

	// unregister chat client
	if (data[0] > 0) {
		for (; s < e && *s != uid; s++);

		sem[0].sem_op = 0;
		sem[1].sem_op = 1;
		semop(sid, sem, 2);

		if (s + 1 < e)
			memmove(s, s + 1, (e - s) * sizeof(int));
		data[0]--;

		sem[0].sem_op = -1;
		semop(sid, sem, 1);
	}

	// try to delegate all open resources to the next active user
	if (data[0] && (pw = getpwuid(data[1]))) {
		shmdt(data);
		if (shmctl(mid, IPC_STAT, &mstatus) == 0 &&
			uid == mstatus.shm_perm.uid) {
			mstatus.shm_perm.uid = pw->pw_uid;
			mstatus.shm_perm.gid = pw->pw_gid;
			shmctl(mid, IPC_SET, &mstatus);
		}
		if (msgctl(qid, IPC_STAT, &qstatus) == 0 &&
			uid == qstatus.msg_perm.uid) {
			qstatus.msg_perm.uid = pw->pw_uid;
			qstatus.msg_perm.gid = pw->pw_gid;
			msgctl(qid, IPC_SET, &qstatus);
		}
		sopts.val = 0;
		sopts.buf = &sstatus;
		if (semctl(sid, 0, IPC_STAT, sopts) == 0 &&
			uid == sopts.buf->sem_perm.uid) {
			sopts.buf->sem_perm.uid = pw->pw_uid;
			sopts.buf->sem_perm.gid = pw->pw_gid;
			semctl(sid, 0, IPC_SET, sopts);
		}
	} else { // no more active users
		shmdt(data);
		shmctl(mid, IPC_RMID, 0);
		msgctl(qid, IPC_RMID, 0);
		semctl(sid, IPC_RMID, 0);
	}
}

int get_count() { return data[0]; }

int msg_recv(int uid, msgdata_t *msg) {
	return msgrcv(qid, (void *) msg, MSGSIZE, uid + 1, IPC_NOWAIT);
}

int msg_send(int uid, msgdata_t *msg) {
	int *s = data + 1;
	int *e = s + data[0];

	for (; s < e; s++) {
		msg->from = uid;
		msg->to = *s;
		if (msg->to != uid) {
			msg->to += 1;
			if (msgsnd(qid, (void *) msg, MSGSIZE, 0) < 0)
				return -1;
		}
	}

	return 0;
}
