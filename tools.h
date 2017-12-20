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

int id_add(char *dst, int dst_size, int id);
int id_del(char *dst, int id);
int reg_client(int qid, int pid);
int unreg_client(int qid, int pid);
int msg_send(int qid, int uid, msgdata_t *msgp);

#endif // _TOOLS_H
