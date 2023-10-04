//	$Id: main.cxx,v 1.3 2008/02/11 07:23:36 canacar Exp $
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
#include <string.h>
#include <stdio.h>
#include <FL/Fl.H>

#include "showmeshui.h"

int winW = 640;
int winH = 480;
const char *appName = "ShowMesh - OpenGL";

ShowMeshUI *ui;

extern "C" {
size_t strlcpy(char *dst, const char *src, size_t len);
}

int
main(int argc, char *argv[])
{
	static char fn[FILENAME_MAX+1];
	char *cmdfn = NULL;

	ui = new ShowMeshUI();

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	printf("ShowMesh by Can Erkin Acar, Zeynep Akalin Acar, 2011\n");
	printf("Smoothing and correction based on algorithms by Taubin et. al.\n\n");

//	ShowMeshWindow *sm = new ShowMeshWindow(0, 0, winW, winH, appName);
//	sm->end();

	ShowMeshWindow *sm = ui->showmesh_window;

	Fl::gl_visual(FL_DOUBLE|FL_RGB);

	if (argc < 2) {
		printf("Enter filename:\n");
		fgets(fn, FILENAME_MAX, stdin);
		char *z = strchr(fn,'\n');
		if (z)
			*z = 0;
		ui->add_mesh(fn);
	}

	if (argc >= 3 && strcmp(argv[1], "-c") == 0) {
		cmdfn = strdup(argv[2]);
		argv += 2;
		argc -= 2;
	}

	for (int n = 1; n < argc; n++) {
		strlcpy(fn, argv[n], FILENAME_MAX);
		ui->add_mesh(fn);
	}

	if (sm->numMeshes() < 1 && cmdfn == NULL)
		printf ("Warning: Failed to load any mesh!\n");


	if (cmdfn != NULL) {
		printf("Running commands from %s\n", cmdfn);
		command_file(cmdfn);
	}

	if (ui->m_quit)
		return 0;

	ui->main_window->show();
	sm->show();
	return Fl::run();
}
