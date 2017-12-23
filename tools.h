/*
 * tools.h
 *
 * Description: Unix Chat using Message Queue
 * Copyright (c) 2017 Aleksander Djuric. All rights reserved.
 * Distributed under the GNU Lesser General Public License (LGPL).
 * The complete text of the license can be found in the LICENSE
 * file included in the distribution.
 *
 */

#ifndef _TOOLS_H
#define _TOOLS_H

int ipc_start(int uid, char *path);
void ipc_finish(int uid);

int reg_client(int uid);
int unreg_client(int uid);
int get_count();

int msg_recv(int uid, msgdata_t *msg);
int msg_send(int uid, msgdata_t *msg);

#endif // _TOOLS_H
