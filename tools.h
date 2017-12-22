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

int reg_client(int uid, int *data);
int unreg_client(int uid, int *data);
int get_count(int *data);
int msg_send(int qid, int uid, int *data, msgdata_t *msgp);

#endif // _TOOLS_H
