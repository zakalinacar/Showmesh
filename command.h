/* $Id: command.h,v 1.1.1.1 2007/12/30 04:44:24 canacar Exp $ */
/* 
 * Copyright (C) 2014 Can Erkin Acar
 * Copyright (C) 2014 Zeynep Akalin Acar
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _COMMAND_H_
#define _COMMAND_H_

#define MAX_LINE 1024
#include "showmesh.h"

void set_window(ShowMeshWindow *sm);
int execute(const char *cmd);
int command_file (char *fn);
int command_script (char *fn);
int command_loop(ShowMeshWindow *sm);
int command_need_loop(void);
void echo(int on);

#endif
