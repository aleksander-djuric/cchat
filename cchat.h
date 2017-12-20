/*
 * cchat.h
 *
 * Description: Unix Chat using Message Queue
 * Copyright (c) 2017 Aleksander Djuric. All rights reserved.
 * Distributed under the GNU Lesser General Public License (LGPL).
 * The complete text of the license can be found in the LICENSE
 * file included in the distribution.
 *
 */

#ifndef _CCHAT_H
#define _CCHAT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <errno.h>
#include <pwd.h>
#include <ncurses.h>

#define MAX_MSGSIZE 4096
#define MSGSIZE (MAX_MSGSIZE-sizeof(long))
#define DATASIZE (MSGSIZE-sizeof(int))
#define CTRL_ID 65535

typedef struct {
	long type;
	int id;
	char data[DATASIZE];
} msgdata_t;

#endif // _CCHAT_H
