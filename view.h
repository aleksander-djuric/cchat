/*
 * view.h
 *
 * Description: Unix Chat using Message Queue
 * Copyright (c) 2017 Aleksander Djuric. All rights reserved.
 * Distributed under the GNU Lesser General Public License (LGPL).
 * The complete text of the license can be found in the LICENSE
 * file included in the distribution.
 *
 */

#ifndef _VIEW_H
#define _VIEW_H

void ncstart();
void draw_statbar(WINDOW *status, const char *fmt, ...);
void add_line(WINDOW *win, int *maxrows, int *maxcols, int count);
void print_msg(WINDOW *win, int uid, char *msg);
void print_system(WINDOW *win, int attrs, const char *fmt, ...);

#endif // _VIEW_H
