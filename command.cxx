// $Id: command.cxx,v 1.14 2008/04/16 08:11:27 canacar Exp $
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

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "showmeshui.h"

extern ShowMeshUI *ui;

int cmd_show (char *, int);
int cmd_hide (char *, int);
int cmd_set (char *, int);
int cmd_sleep (char *, int);
int cmd_input (char *, int);
int cmd_echo (char *, int);
int cmd_eps_save(char *, int);
int cmd_quit (char *, int);
int cmd_capture (char *, int);
int cmd_animate (char *, int);
int cmd_load (char *, int);
int cmd_pload (char *, int);
int cmd_psave (char *, int);
int cmd_notimp (char *, int);
int cmd_refresh (char *, int);
int cmd_save(char *, int);
int cmd_smooth(char *, int);
int cmd_correct(char *, int);
int cmd_extract(char *, int);
int cmd_improve(char *, int);
int cmd_fill_holes(char *, int);
int cmd_split(char *, int);
int cmd_nfield (char *, int);
int cmd_nfield_auto (char *, int);
int cmd_nfield_load (char *, int);
int cmd_nfield_background (char *, int);
int cmd_nfield_background_load (char *, int);
int cmd_nfield_background_alpha (char *, int);
int cmd_nfield_range (char *, int);
int cmd_info (char *, int);
int cmd_mark (char *, int);
int cmd_fix(char *, int);

int cmd_var_done = 0;
int cmd_var_echo = 1;
int cmd_display = 0;

struct comdef {
	const char *cmd;
	int (*run)(char *, int);
	int sel;
};

struct comdef root[] = {{"show", cmd_show, 0},
			{"hide", cmd_hide, 0},
			{"set", cmd_set, 0},
			{"sleep", cmd_sleep, 0},
			{"load", cmd_load, 0},
			{"pload", cmd_pload, 0},
			{"psave", cmd_psave, 0},
			{"nfield", cmd_nfield, 0},
			{"save", cmd_save, 0},
			{"epssave", cmd_eps_save, 0},
			{"smooth", cmd_smooth, 0},
			{"correct", cmd_correct, 0},
			{"improve", cmd_improve, 0},
			{"extract", cmd_extract, 0},
			{"fill", cmd_fill_holes, 0},
			{"fix", cmd_fix, 0},
			{"split", cmd_split, 0},
			{"echo", cmd_echo, 0},
			{"quit", cmd_quit, 0},
			{"capture", cmd_capture, 0},
			{"animate", cmd_animate, 0},
			{"input", cmd_input, 0},
			{"refresh", cmd_refresh, 0},
			{"script", cmd_input, 1},
			{"info", cmd_info, 0},
			{"mark", cmd_mark, 0},
			{"unmark", cmd_mark, 1},
			{0,0,0}};

ShowMeshWindow *cmd_window = NULL;

void
set_window(ShowMeshWindow *sm)
{
	cmd_window = sm;
}

void
echo(int on)
{
	cmd_var_echo = on;
}

void
skip_ws(char **buf)
{
	assert(buf);
	assert (*buf);

	while (isspace(**buf)) (*buf)++;
}

void strip_ws(char *buf)
{
	assert(buf);

	for (int len = strlen(buf) - 1; len >= 0; len --)
		if (isspace(buf[len]))
			buf[len] = 0;
		else
			break;

}

#define MAX_TOKEN 16

int
get_token(char *tok, char **cmd)
{
	assert (tok);
	int len = 0;
	char *c = *cmd;

	
	for (int n = 0; n < MAX_TOKEN; n++) {
		if ( isalnum (*c) ) {
			*tok ++ = *c ++;
			len ++;
		} else {
			*tok = 0;
			*cmd = c;
			return len;
		}
	}

	tok[0] = 0;

	return 0;
}

int
command(char *cmd, struct comdef *def)
{
	char buf[MAX_TOKEN];

	if (cmd == NULL)
		return 1;
	if (def == NULL)
		return 1;

	skip_ws(&cmd);

	if ( *cmd == 0 || *cmd == '#' )
		return 0;

	if (get_token(buf, &cmd) == 0) {
		return 1;
	}

	for (struct comdef *c = def; c->cmd; c++) {
		if (strcmp(c->cmd, buf) == 0)
			return c->run(cmd, c->sel);
	}

	printf("Not found >%s< commands:", cmd);
	while (def->cmd)
		printf(" %s", (def++)->cmd);
	printf("\n");

	return 1;
}

int
execute(const char *cm)
{
	char *cmd;
	int ret;
	if (cm == NULL)
		return 1;

	cmd = strdup(cm);

	strip_ws(cmd);

	if (cmd_var_echo)
		printf (" > %s \n", cmd);

	ret = command(cmd, root);
	free(cmd);
	cmd_window->redraw();

	return ret;
}

#define MAX_DEPTH 10

int
command_file (char *fn)
{
	static int depth = 0;

	FILE *f = 0;

	if (strcmp(fn , "-") == 0)
		f=stdin;
	else	
		f = fopen(fn, "r");

	if (f == NULL) {
		perror ("Failed to open");
		return 1;
	}

	if (depth > MAX_DEPTH) {
		printf ("Max input depth exceeded!\n");
		return 1;
	}

	depth ++;

	char *buf = new char[MAX_LINE];

	int ret = 0;
	cmd_var_done = 0;

	while (!feof(f)) {
		if (fgets(buf, MAX_LINE, f) == NULL)
			break;

		strip_ws(buf);

		if (cmd_var_echo)
			printf (" > %s \n", buf);

		ret = command (buf, root);
		if (ret) break;
		if (cmd_var_done)
			break;

	}

	depth --;
	cmd_window->redraw();

	delete[] buf;
	fclose(f);
	return ret;
}

// show command
int
cmd_show_mesh(char *arg, int sel)
{
	int nm = cmd_window->numMeshes();

	skip_ws(&arg);
	if (strncasecmp(arg, "all", 3) == 0) {
                for (int n=0; n<nm; n++) {
                        if (sel)
				cmd_window->clearFlag(n, MRF_HIDDEN);
                        else
				cmd_window->setFlag(n, MRF_HIDDEN);
                }
                return 0;
        }
	int n = atoi(arg);
        if (n < 0 || n >= nm) {
                printf ("invalid mesh number %d\n", n);
                return 1;
        }
	if (sel)
		cmd_window->clearFlag(n, MRF_HIDDEN);
	else
		cmd_window->setFlag(n, MRF_HIDDEN);

        return 0;
}

int
cmd_show_bound(char *arg, int sel)
{
	int nm = cmd_window->numMeshes();

	skip_ws(&arg);
	if (strncasecmp(arg, "all", 3) == 0) {
                for (int n=0; n<nm; n++) {
			if (sel)
				cmd_window->clearFlag(n, MRF_BOUND);
			else
				cmd_window->setFlag(n, MRF_BOUND);
                }
                return 0;
        }
	int n = atoi(arg);
        if (n < 0 || n >= nm) {
                printf ("invalid mesh number %d\n", n);
                return 1;
        }
	if (sel)
		cmd_window->clearFlag(n, MRF_BOUND);
	else
		cmd_window->setFlag(n, MRF_BOUND);

        return 0;
}

int
cmd_proc_sharp(char *arg, int sel)
{
	int nm = cmd_window->numMeshes();

	skip_ws(&arg);
	if (strncasecmp(arg, "all", 3) == 0) {
                for (int n=0; n<nm; n++)
			cmd_window->process_sharp_edges(n, sel);
                return 0;
        }

	int n = atoi(arg);
        if (n < 0 || n >= nm) {
                printf ("invalid mesh number %d\n", n);
                return 1;
        }

	cmd_window->process_sharp_edges(n, sel);

        return 0;
}

int
cmd_proc_intersect(char *arg, int sel)
{
	int nm = cmd_window->numMeshes();

	skip_ws(&arg);
	if (strncasecmp(arg, "all", 3) == 0) {
                for (int n=0; n<nm; n++)
			cmd_window->process_intersecting(n, sel);
                return 0;
        }

	int n = atoi(arg);
        if (n < 0 || n >= nm) {
                printf ("invalid mesh number %d\n", n);
                return 1;
        }

	cmd_window->process_intersecting(n, sel);

        return 0;
}

struct comdef cd_show[]={{"mesh", cmd_show_mesh, 1},
                         {"bound", cmd_show_bound, 1},
                         {"intersect", cmd_proc_intersect, 0},
                         {"sharp", cmd_proc_sharp, 0},
			 {0,0,0}};
int
cmd_show (char *arg, int sel)
{
	return command (arg, cd_show);
}


// fix command

struct comdef cd_fix[]={{"intersect", cmd_proc_intersect, 1},
                         {"sharp", cmd_proc_sharp, 1},
                         {"holes", cmd_fill_holes, 1},
			 {0,0,0}};
int
cmd_fix (char *arg, int sel)
{
	return command (arg, cd_fix);
}

// hide command

struct comdef cd_hide[]={{"mesh", cmd_show_mesh, 0},
                         {"bound", cmd_show_bound, 0},
			 {0,0,0}};
int
cmd_hide (char *arg, int sel)
{
	return command (arg, cd_hide);
}

// set command
int
cmd_set_bool (char *arg, int sel)
{
	assert (arg);
	double a;
	bool b;

	skip_ws(&arg);
	strip_ws(arg);

	if (sscanf(arg, "%lg", &a) != 1) {
		if (strcasecmp(arg,"on") == 0)
			a = 1;
		else if (strcasecmp(arg,"off") == 0)
			a = 0;
		else
			return 1;
	}

        if (floor(a) != a)
                return 1;
	b = (a != 0);

	printf("set bool %d, %d\n", sel, b);


	switch(sel) {
	case 0:
		cmd_window->setLight(a);
		break;
	case 1:
		cmd_window->setWireframe(a);
		break;
	case 2:
		cmd_window->setCullFaces(a);
		break;
	case 3:
		cmd_window->setInterp(a);
		break;
	case 4:
		cmd_window->setRevOrder(a);
		break;
	default:
		return 1;
	}

	return 0;
}
int
cmd_set_op2 (char *arg, int sel)
{
	assert (arg);
	double a1, a2;
	if (sscanf(arg, "%lg %lg", &a1, &a2) != 2)
		return 1;

        if (floor(a1) != a1)
                return 1;

        int msh = (int) a1;

        if (msh <0 || msh >= cmd_window->numMeshes())
                return 1;

        if (a2 < 0) a2 = 0;
        if (a2 > 1) a2 = 1;

	cmd_window->setAlpha(msh, a2);

	return 0;
}

double anim_ex = 0;
double anim_ey = 0;
double anim_ez = 0;
double anim_rx = 1;
double anim_ry = 0;
double anim_rz = 0;

int
cmd_set_op3 (char *arg, int sel)
{
	assert (arg);
	double x, y, z;
	if (sscanf(arg, "%lg %lg %lg", &x, &y, &z) != 3)
		return 1;

	switch (sel) {
        case 0:
		cmd_window->setView(x, y, z);
                break;
        case 1:
		cmd_window->setEye(x, y, z);
                break;
        case 2:
                anim_ex = x;
                anim_ey = y;
                anim_ez = z;
                break;
        case 3:
                anim_rx = x;
                anim_ry = y;
                anim_rz = z;
		break;
        case 4:
                cmd_window->setPRotation(x, y, z);
		break;
        case 5:
                cmd_window->setPOffset(x, y, z);
		break;
        case 6:
                cmd_window->setPScale(x, y, z);
		break;
        default:
                return 1;
        }

	return 0;
}

int
cmd_set_op6(char *arg, int sel)
{
	double a1, a2, a3, a4, a5, a6;
	
	skip_ws(&arg);
	strip_ws(arg);

	if (strcasecmp(arg, "off") == 0) {
		ui->showmesh_window->setClip(false);
		return 0;
	}

	if (strcasecmp(arg, "on") == 0) {
		ui->showmesh_window->setClip(true);
		return 0;
	}

	if (sscanf(arg, "%lg %lg %lg %lg %lg %lg",
		   &a1, &a2, &a3, &a4, &a5, &a6) != 6)
		return 1;

	ui->showmesh_window->setClip(a1, a2, a3, a4, a5, a6);

	return 0;
}

int
cmd_set_dipole(char *arg, int sel)
{
	double a1, a2, a3, a4, a5, a6;
	long di;
	char *ep = NULL;


	skip_ws(&arg);
	strip_ws(arg);

	di = strtol(arg, &ep, 0);
	if (ep == arg)
		return 1;

#define MAX_DIPOLES 100
	if (di < 0 || di > MAX_DIPOLES)
		return 1;

	if (strcasecmp(ep, "off") == 0) {
		ui->showmesh_window->setDipole(di,0,0,0,1,0,0,false);
		return 0;
	}

	if (sscanf(ep, "%lg %lg %lg %lg %lg %lg",
		   &a1, &a2, &a3, &a4, &a5, &a6) != 6)
		return 1;

	ui->showmesh_window->setDipole(di, a1, a2, a3, a4, a5, a6);
	return 0;
}

int
cmd_set_op4 (char *arg, int sel)
{
	assert (arg);
	double a1, a2, a3, a4;
	if (sscanf(arg, "%lg %lg %lg %lg", &a1, &a2, &a3, &a4) != 4)
		return 1;

        if (floor(a1) != a1)
                return 1;

        int msh = (int) a1;
        if (msh <0 || msh >= cmd_window->numMeshes())
                return 1;

        if (a2 < 0) a2 = 0;
        else if (a2 > 1) a2 = 1;

        if (a3 < 0) a3 = 0;
        else if (a3 > 1) a3 = 1;

        if (a4 < 0) a4 = 0;
        else if (a4 > 1) a4 = 1;

        cmd_window->setColor(msh, a2, a3, a4);
	return 0;
}



int
cmd_set_dcolor (char *arg, int sel)
{
	assert (arg);
	double a1, a2, a3, a4;
	if (sscanf(arg, "%lg %lg %lg %lg", &a1, &a2, &a3, &a4) != 4)
		return 1;

        if (floor(a1) != a1)
                return 1;

        int d = (int) a1;
        if (d < 0)
                return 1;

        if (a2 < 0) a2 = 0;
        else if (a2 > 1) a2 = 1;

        if (a3 < 0) a3 = 0;
        else if (a3 > 1) a3 = 1;

        if (a4 < 0) a4 = 0;
        else if (a4 > 1) a4 = 1;

        cmd_window->setDipoleColor(d, a2, a3, a4);
	return 0;
}

// set command
int
cmd_set_dscale (char *arg, int sel)
{
	assert (arg);
	double a1, a2;
	if (sscanf(arg, "%lg %lg", &a1, &a2) != 2)
		return 1;

        if (floor(a1) != a1)
                return 1;

        int d = (int) a1;

        if (d <0)
                return 1;

	cmd_window->setDipoleScale(d, a2);

	return 0;
}

struct comdef cd_set[]={{"view", cmd_set_op3, 0},
			{"eye", cmd_set_op3, 1},
			{"move", cmd_set_op3, 2},
			{"rotate", cmd_set_op3, 3},
                        {"alpha", cmd_set_op2, 0},
                        {"color", cmd_set_op4, 0},
			{"dipole", cmd_set_dipole, 0},
			{"clip", cmd_set_op6, 1},
			{"protate", cmd_set_op3, 4},
			{"pshift", cmd_set_op3, 5},
			{"pscale", cmd_set_op3, 6},
			{"dcolor", cmd_set_dcolor, 7},
			{"dscale", cmd_set_dscale, 7},
			{"light", cmd_set_bool, 0},
			{"wire", cmd_set_bool, 1},
			{"cull", cmd_set_bool, 2},
			{"interp", cmd_set_bool, 3},
			{"revorder", cmd_set_bool, 4},
			{0,0,0}};
int
cmd_set (char *arg, int sel)
{
	return command (arg, cd_set);
}


// mark command
int
cmd_mark_node(char *arg, int nbrs)
{
	double a;
	int idx;

	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	if (sscanf(arg, "%lg", &a) != 1 || floor(a) != a) {
		printf("Error!\n");
		return 1;
	}

	idx = (int)a;
	
	if (ui->showmesh_window->mark_node(0, idx, nbrs)) {
		printf("Error!\n");
		return 1;
	}

	return 0;
}

int
cmd_mark_elem(char *arg, int nbrs)
{
	double a;
	int idx;

	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	if (sscanf(arg, "%lg", &a) != 1 || floor(a) != a) {
		printf("Error!\n");
		return 1;
	}

	idx = (int)a;

	if (ui->showmesh_window->mark_elem(0, idx, nbrs)) {
		printf("Error!\n");
		return 1;
	}

	return 0;
}

int
cmd_unmark_node(char *arg, int nbrs)
{
	double a;
	int idx;

	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	if (sscanf(arg, "%lg", &a) != 1 || floor(a) != a) {
		printf("Error!\n");
		return 1;
	}

	idx = (int)a;
	
	if (ui->showmesh_window->mark_node(0, idx, nbrs)) {
		printf("Error!\n");
		return 1;
	}

	return 0;
}

int
cmd_unmark_elem(char *arg, int nbrs)
{
	double a;
	int idx;

	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	if (sscanf(arg, "%lg", &a) != 1 || floor(a) != a) {
		printf("Error!\n");
		return 1;
	}

	idx = (int)a;

	if (ui->showmesh_window->mark_elem(0, idx, nbrs)) {
		printf("Error!\n");
		return 1;
	}

	return 0;
}


struct comdef cd_mark[]={{"node", cmd_mark_node, 0},
			 {"elem", cmd_mark_elem, 0},
			 {"nnode", cmd_mark_node, 1},
			 {"nelem", cmd_mark_elem, 1},
			 {0,0,0}};

struct comdef cd_unmark[]={{"node", cmd_unmark_node, 0},
			 {"elem", cmd_unmark_elem, 0},
			 {0,0,0}};

int
cmd_mark(char *arg, int sel)
{
	return command(arg, sel ? cd_unmark : cd_mark);
}

// nfield command

struct comdef cd_nfield[]={{"load", cmd_nfield_load, 0},
			   {"range", cmd_nfield_range, 0},
			   {"auto", cmd_nfield_auto, 0},
			   {"zero", cmd_nfield_auto, 1},
			   {"background", cmd_nfield_background, 0},
			   {0,0,0}};
int
cmd_nfield(char *arg, int sel)
{
	return command(arg, cd_nfield);
}

int
cmd_nfield_load (char *arg, int sel)
{
	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	if (ui->showmesh_window->load_node_fn(arg, 0)) {
		printf("Error!\n");
		return 1;
	}

	printf ("Done.\n");
	return 0;
}

struct comdef cd_nfield_background[]={{"load", cmd_nfield_background_load, 0},
				      {"on", cmd_nfield_background_alpha, 1},
				      {"off", cmd_nfield_background_alpha, 0},
				      {"alpha", cmd_nfield_background_alpha, -1},
				      {0,0,0}};

int
cmd_nfield_background(char *arg, int sel)
{
	return command(arg, cd_nfield_background);
}

int
cmd_nfield_background_load (char *arg, int sel)
{
	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	if (ui->showmesh_window->load_node_background_fn(arg, 0)) {
		printf("Error!\n");
		return 1;
	}

	printf ("Done.\n");
	return 0;
}

int
cmd_nfield_background_alpha(char *arg, int sel)
{
	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);
	double alpha;

	if (sel == -1) {
		if (sscanf(arg, "%lg", &alpha) != 1 || alpha < 0 || alpha > 1) {
			printf("Error!\n");
			return 1;
		}
	} else 
		alpha = sel; // 0 or 1

	if (ui->showmesh_window->set_background_alpha(alpha, 0)) {
		printf("Error!\n");
		return 1;
	}

	printf ("Done.\n");
	return 0;
}

int
cmd_nfield_range (char *arg, int sel)
{
	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);
	double rmin, rmax;
	if (sscanf(arg, "%lg %lg", &rmin, &rmax) != 2) {
		printf("Usage: nfield range <rmin> <rmax>!\n");
		return 1;
	}

	if (ui->showmesh_window->set_node_range(rmin, rmax, 0)) {
		printf("Error!\n");
		return 1;
	}

	printf ("Done.\n");
	return 0;
}

int
cmd_nfield_auto (char *arg, int sel)
{
	if (ui->showmesh_window->set_node_auto(sel, 0)) {
		printf("Error!\n");
		return 1;
	}

	printf ("Done.\n");
	return 0;
}

// info command

int
cmd_info(char *arg, int sel) {
	printf("View Information:\n");
	double x, y, z;

	ui->showmesh_window->getEye().getCoord(x, y, z);
	printf("  Eye:  %g %g %g\n", x, y, z);

	ui->showmesh_window->getView().getCoord(x, y, z);
	printf("  View: %g %g %g\n", x, y, z);

	printf("\n");
	return 0;
}


// sleep command

int
cmd_sleep (char *arg, int sel)
{
	assert (arg);
	int del = atoi(arg);
	if (del < 1) return 1;
#ifdef __WIN32__ 
	printf ("Sleep not supported yet\n");
#else
	printf ("Sleeping %d seconds ...\n", del);
	sleep(del);
#endif
	return 0;
}

// input command

int
cmd_input (char *arg, int sel)
{
	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	if (sel) {
		printf ("Running script from file >%s<\n", arg);
		if (command_script(arg)) {
			printf("Error!\n");
			return 1;
		}
	} else {
		printf ("Running commands from file >%s<\n", arg);
		if (command_file(arg)) {
			printf("Error!\n");
			return 1;
		}
	}
	printf ("Done.\n");
	return 0;
}

// load command

int
cmd_load (char *arg, int sel)
{
	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	if (ui->add_mesh(arg) == NULL) {
		printf("Error!\n");
		return 1;
	}
	printf ("Done.\n");
	return 0;
}

int
cmd_pload(char *arg, int sel)
{
	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	if (ui->showmesh_window->loadPointField(arg)) {
		printf("Error!\n");
		return 1;
	}
	printf ("Done.\n");
	return 0;
}

int
cmd_psave(char *arg, int sel)
{
	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	if (ui->showmesh_window->savePointField(arg)) {
		printf("Error!\n");
		return 1;
	}
	printf ("Done.\n");
	return 0;
}



int
cmd_save(char *arg, int sel)
{
	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	printf("Saving first mesh first class as: %s\n", arg);

	if (ui->showmesh_window->save_mesh(arg, 0, 0)) {
		printf("Error!\n");
		return 1;
	}

	printf ("Done.\n");
	return 0;
}

int
cmd_eps_save(char *arg, int sel)
{
	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	printf("Saving EPS as: %s\n", arg);

	if (ui->showmesh_window->saveEPS(arg, 0)) {
		printf("Error!\n");
		return 1;
	}

	printf ("Done.\n");
	return 0;
}

int
cmd_smooth(char *arg, int sel)
{
	assert(arg);
	skip_ws(&arg);
	strip_ws(arg);

	int cnt = atoi(arg);
	if (cnt <= 0)
		cnt = 1;

	printf("Smoothing first mesh %d times\n", cnt);
	if (ui->showmesh_window->smooth_mesh(0, cnt)) {
		printf("Error!\n");
		return 1;
	}
	printf ("Done.\n");
	return 0;
}

int
cmd_correct(char *arg, int sel)
{
	printf("Correcting first mesh\n");
	if (ui->showmesh_window->correct_mesh(0)) {
		printf("Error!\n");
		return 1;
	}
	printf ("Done.\n");
	return 0;
}

int
cmd_fill_holes(char *arg, int sel)
{
	printf("Correcting first mesh\n");
	if (ui->showmesh_window->fill_holes(0)) {
		printf("Error!\n");
		return 1;
	}
	printf ("Done.\n");
	return 0;
}

int
cmd_improve(char *arg, int sel)
{
	int cnt, na;
	double aspect, esize;
	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	na = sscanf(arg, "%d %lg %lg", &cnt, &aspect, &esize);
	if (na < 1 || cnt < 1)
		cnt = 1;
	if (na < 2 || aspect < 0)
		aspect = 0.001;
	if (na < 3 || esize < 0)
		esize = 0.001;
	printf("Improving first mesh %d times\n", cnt);
	printf(" Element Aspect: %g, Size Ratio: %g \n", aspect, esize);
	if (ui->showmesh_window->improve_mesh(0, cnt, aspect, esize)) {
		printf("Error!\n");
		return 1;
	}
	printf ("Done.\n");
	return 0;
}

int
cmd_extract(char *arg, int sel)
{
	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	int cls;

	if (*arg == '\0')
		cls = -1;
	else
		cls = atoi(arg);

	printf("Extracting class %d from the first mesh\n", cls);
	if (ui->showmesh_window->extract_class(0, cls)) {
		printf("Error!\n");
		return 1;
	}
	printf ("Done.\n");
	return 0;
}

int
cmd_split(char *arg, int sel)
{
	assert (arg);
	skip_ws(&arg);
	strip_ws(arg);

	if (strncmp(arg, "edges", 5) == 0) {
		arg += 5;
		skip_ws(&arg);

		double thresh = atof(arg);
		if (thresh <= 0)
			thresh = 1;

		printf("Splitting edges with threshold %g\n", thresh);
		if (ui->showmesh_window->split_mesh(0, thresh)) {
			printf("Error!\n");
			return 1;
		}
	} else if (strncmp(arg, "intersect", 9) == 0){
		printf("Splitting intersecting elements\n");
		if (ui->showmesh_window->split_intersecting(0)) {
			printf("Error!\n");
			return 1;
		}
	} else {
		printf("'split edges <thresh>' or 'split intersect'\n");
		return 1;
	}

	printf ("Done.\n");
	return 0;
}


// command not implemented

int
cmd_notimp (char *arg, int sel)
{
	printf ("Not implemented!\n");
	return 0;
}

// echo command

int
cmd_echo (char *arg, int sel)
{
	printf (">> %s\n", arg);
	return 0;
}

// refresh command

int
cmd_refresh (char *arg, int sel)
{
	printf ("refresh\n");
        cmd_display = 1;
	return 0;
}

// quit command

int
cmd_quit (char *arg, int sel)
{
	printf ("Quit command ...\n");
	ui->quit();
	return 0;
}

// animate command

int anim_count = 0;

int
cmd_animate (char *arg, int sel)
{
	printf("Anim Count: %d\n", anim_count);
        if (anim_count) return 1;
        int count = atoi(arg);
        if (count < 1) count = 1;
        anim_count = count;
	printf("Anim Count <- %d\n", anim_count);
	return 0;
}

// capture command

int
cmd_capture (char *arg, int sel)
{
	int capture = cmd_window->getCapture();
	skip_ws(&arg);
	if (strncasecmp(arg, "on", 2) == 0)
		capture = 1;
        else if (strncasecmp(arg, "off", 3) == 0)
                capture = 0;
        else
		capture = !capture;
        printf("Capture %s\n", capture ? "on":"off");
	cmd_window->setCapture(capture);

	return 0;
}

FILE *command_fd = 0;

int command_script(char *fn)
{
        if (fn == NULL) return 1;

        if (command_fd) {
                printf("Stopping already running script\n");
                fclose(command_fd);
        }
        command_fd = fopen(fn, "r");
        if (command_fd == NULL) {
                printf ("Failed to open script file %s\n", fn);
                return 1;
        }
        return 0;
}

int
command_need_loop(void)
{
//	printf("need_loop: %s\n", (anim_count||command_fd) ? "yes" : "no");
	if (anim_count || command_fd)
		return 1;
	return 0;
}

int
command_loop(ShowMeshWindow *sm)
{
	static char buf[MAX_LINE];
//	printf("Command loop!\n");

	if (cmd_window == NULL)
		cmd_window = sm;
	if (cmd_window != sm)
		return 0;

        if (anim_count) {
		printf("Anim Count == %d\n", anim_count);
                cmd_window->addEye(anim_ex, anim_ey, anim_ez);
                cmd_window->addView(anim_rx, anim_ry, anim_rz);
                anim_count --;
                return 1;
        }
        if (command_fd == NULL)
                return 0;

	if (fgets(buf, MAX_LINE, command_fd) == NULL) {
		fclose(command_fd);
                command_fd = NULL;
                return 0;
        }

	strip_ws(buf);

	if (cmd_var_echo)
		printf (" > %s \n", buf);
                
        cmd_display = 0;
	int ret = command (buf, root);
        if (ret) {
                printf ("Error!\n");
                return 0;
        }

        return cmd_display;
}

