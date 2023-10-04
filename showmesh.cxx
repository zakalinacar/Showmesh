//	$Id: showmesh.cxx,v 1.22 2008/04/16 08:11:27 canacar Exp $
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
#include "showmeshui.h"
#include <FL/Fl.H>
#include <FL/gl.h>
#include <GL/glu.h>
#include <string.h>
#include "glcapture.h"
#include "command.h"
#include "meshproc.h"
#include "gl2ps.h"
//---------------------------------------------------------------------------

// number of times we try to correct the mesh
#define MAX_CORRECT 10

extern ShowMeshUI *ui;

ShowMeshWindow::ShowMeshWindow(int X, int Y, int W, int H, const char *L)
	: Fl_Gl_Window(X, Y, W, H, L), eye(0,0,0), rot(90, 0, 0),
	  pf_scale(1,1,1)
{
	idle = ani = capture = filter = 0;
	edges = glcull = help = interp = light = wire = 0;
	revorder = edges = tmode = 0;

	m_mx = m_my = m_mz = 0;
	light = 1;

	fclass = 0;

	tselected = 0;
	tstep = 1;
	tmode = 0;

	num_meshes = 0;

	numiter = 0;
	numcorrect = 0;
	update = 0;
	fclass = 0;
	tstep = 0;
	tselected = 0;

	num_pf = 0;

	glcap = NULL;

	m_delsc = 1;

	setAmbientLight(0.3, 0.3, 0.45);
	setSourceLight(0.9, 0.8, 0.8);
	setLightPos(7.0, 0.0, 0.0);

	text.color(1, 1, 0.8);

	glcap = new GLCapture();
	glcap->setWindow(0, 0, 800, 600);
	glcap->newCapture("sm");
}

/* functions that need to display 2D data on the view plane
 * must call flatMode(true) on entry, and flatMode(false) on exit
 * there should be no display updates in between.
 */
void
ShowMeshWindow::flatMode(bool enable)
{
	static int ref = 0;
	static int view[4];
	if (enable) {
		if (ref++)
			return;
		glPushMatrix();
		glLoadIdentity();
		glGetIntegerv(GL_VIEWPORT, view);
		glViewport(0, 0, w(), h());
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, w(), 0, h(), -100, 100);
		if (light)
			glDisable(GL_LIGHTING);
	} else {
		if (--ref)
			return;
		glPopMatrix();
		glViewport(view[0], view[1], view[2], view[3]);
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		if (light)
			glEnable(GL_LIGHTING);
	}
}

void
ShowMeshWindow::SetRenderModes(void)
{
	if (wire)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	if (light)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
	
	if (glcull)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	else
		glDisable(GL_CULL_FACE);
}


void
ShowMeshWindow::DrawTransformHelp(int fsize, int &y)
{
#if 0
	if (tselected < 0)
		tselected = 0;
	if (tselected >= num_meshes)
		tselected = num_meshes - 1;

	MeshRender *mr = meshes[tselected];
	Point3 o = mr->getOffset();
	Point3 r = mr->getRotation();
#else
	Point3 o = pf_off;
	Point3 r = pf_rot;
#endif
	flatMode(true);

	text.out(20, y -= fsize, "Transform Mode");

	y -= fsize;
//	text.out(20, y -= fsize, "M - select Mesh (%d)", tselected);
	text.out(20, y -= fsize, "X - eXit Transform Mode");
	text.out(20, y -= fsize, "V - Wireframe     : %s", wire ? "on" : "off");

	y -= fsize;
	text.out(20, y -= fsize, "Q - rotate +X (%g)", r.getX());
	text.out(20, y -= fsize, "A - rotate -X");
	text.out(20, y -= fsize, "W - rotate +Y (%g)", r.getY());
	text.out(20, y -= fsize, "S - rotate -Y");
	text.out(20, y -= fsize, "E - rotate +Z (%g)", r.getZ());
	text.out(20, y -= fsize, "D - rotate -Z");

	y -= fsize;
	text.out(20, y -= fsize, "P - move +X (%g)", o.getX());
	text.out(20, y -= fsize, "L - move -X");
	text.out(20, y -= fsize, "O - move +Y (%g)", o.getY());
	text.out(20, y -= fsize, "K - move -Y");
	text.out(20, y -= fsize, "I - move +Z (%g)", o.getZ());
	text.out(20, y -= fsize, "J - move -Z");

	y -= fsize;
	text.out(20, y -= fsize, "> - increase step size (%g)", tstep);
	text.out(20, y -= fsize, "< - decrease step size");
	text.out(20, y -= fsize, "1-6 - set point size");

	y -= fsize;
	text.out(20, y -= fsize, "C - Clear all transforms");
	text.out(20, y -= fsize, "H - Toggle Help display");

	flatMode(false);
}

void 
ShowMeshWindow::DrawHelp(void)
{
	int y = h() - 10;
	int fsize = text.size();

	if (tmode) {
		DrawTransformHelp(fsize, y);
		return;
	}

	flatMode(true);
	
	text.out(20, y -= fsize, "Keyboard Usage");
	text.out(20, y -= fsize, "Q - Quits");
	text.out(20, y -= fsize, "A - Animate       : %s", ani ? "on" : "off");
	text.out(20, y -= fsize, "W - Wireframe     : %s", wire ? "on" : "off");
	text.out(20, y -= fsize, "I - Smooth faces  : %s", interp ? "on" : "off");
	text.out(20, y -= fsize, "L - Lighting      : %s", light ? "on" : "off");
	text.out(20, y -= fsize, "C - OpenGL culling: %s", glcull ? "on" : "off");
	text.out(20, y -= fsize, "E - Edge Display  : %s", edges ? "on" : "off");
	text.out(20, y -= fsize, "K - Capture - %s", capture ? "on" : "off");
	text.out(20, y -= fsize, "F - Filter class (%d) - %s",
		 fclass, filter ? "on" : "off");
	text.out(20, y -= fsize, "S - Save %s", filter ? "current" : "all");
	text.out(20, y -= fsize, "T - Transform Mode");
	text.out(20, y -= fsize, "1 - 1 Smoothing iteration");
	text.out(20, y -= fsize, "2 - 5 Smoothing iterations");
	text.out(20, y -= fsize, "3 - 10 Smoothing iterations");
	text.out(20, y -= fsize, "4 - 100 Smoothing iterations");
	text.out(20, y -= fsize, "b - 1 Correction iteration");
	text.out(20, y -= fsize, "B - 10 Correction iteration");
	text.out(20, y -= fsize, "0 - Stop iterations");

	text.out(20, y -= fsize, "H - This screen");

	text.out(20, y -= (3 * fsize), "Mouse Usage");
	text.out(20, y -= fsize, "Left Button  - Rotates");
	text.out(20, y -= fsize, "Right Button - Rotates");
	text.out(20, y -= fsize, "Both Buttons - Zoom In/Out");

	flatMode(false);

}


void
ShowMeshWindow::setInterp(bool i)
{
	interp = i;
	for (int n = 0; n < num_meshes; n++) {
		if (interp)
			meshes[n]->setFlag(MRF_INTERP);
		else
			meshes[n]->clearFlag(MRF_INTERP);
	}
}

void
ShowMeshWindow::setClip(double x0, double y0, double z0,
		     double x1, double y1, double z1)
{
	for (int n = 0; n < num_meshes; n++) {
		meshes[n]->setClip(x0, y0, z0, x1, y1, z1);
		setFlag(n, MRF_CLIP);
	}
}

void
ShowMeshWindow::setClip(bool on)
{
	for (int n = 0; n < num_meshes; n++) {
		if (on)
			setFlag(n, MRF_CLIP);
		else
			clearFlag(n, MRF_CLIP);
	}
}

void
ShowMeshWindow::glDisplay(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(-eye.getX(), -eye.getY(), -eye.getZ());
	glRotatef(rot.getX(), 1.0f, 0.0f, 0.0f);
	glRotatef(rot.getY(), 0.0f, 1.0f, 0.0f);
	glRotatef(rot.getZ(), 0.0f, 0.0f, 1.0f);

	for (int n = 0; n < m_dip.size(); n++)
		if (m_dip[n].show)
			drawDipole(m_dip[n]);

	if (!pfield.empty()) {
		drawPointField();
	}

	if (revorder)
		for (int n=num_meshes - 1; n >= 0; n--)
			meshes[n]->render(filter ? fclass : -1);
	else
		for (int n=0; n<num_meshes; n++)
			meshes[n]->render(filter ? fclass : -1);

	if (help) {
		DrawHelp();
		drawAxis();
	}
	if (capture && glcap) {
		if (glcap->captureFrame()) putchar('!');
		else putchar('.');
	}
}

void
ShowMeshWindow::glResize(int wid, int ht)
{
	if (!ht)
		return;

	if (glcap)
		glcap->setWindow(0, 0, wid, ht);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, wid, ht);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(90, (double) wid / ht, 1, 4096);

	redraw();
}

double *
ShowMeshWindow::loadPotFile(TriMeshLin *msh, FILE *f)
{
	if (f == NULL || msh == NULL)
		return 0;

	int nn = msh->getNumVerts();
	double *pot = new double[nn];

	int n;
	for (n = 0; n < nn; n++)
		if (fscanf(f, "%lg", pot+n) != 1)
			break;

	if (n != nn) {
		delete[] pot;
		return 0;
	}

	return pot;
}

/* Load 'new' freesurfer .w file */
/* ref: http://www.grahamwideman.com/gw/brain/fs/surfacefileformats.htm */
double *
ShowMeshWindow::loadFSWFile(TriMeshLin *msh, FILE *f)
{
	uint32_t vertex_count;
	uint32_t face_count;
	int n, nn;
	double *pot;	
	FSFile fs(f);

	uint16_t latency;

	if (f == NULL || msh == NULL)
		return NULL;

	if (fs.readI2(latency))
		return NULL;

	if (fs.readI3(vertex_count))
		return NULL;

	nn = msh->getNumVerts();

	if (nn < vertex_count)
		return NULL;

	pot = new double[nn];
	memset(pot, 0, nn * sizeof(double));

	for (n = 0; n < nn; n++) {
		uint32_t index;
		if (fs.readI3(index))
			break;

		if (index >= nn)
			break;

		if (fs.readF4(pot[index]))
			break;
	}

	if (n != nn) {
		delete[] pot;
		return NULL;
	}

	return pot;
}

/* Load 'new' freesurfer curv file */
/* ref: http://www.grahamwideman.com/gw/brain/fs/surfacefileformats.htm */
double *
ShowMeshWindow::loadFSCurvFile(TriMeshLin *msh, FILE *f)
{
	uint32_t vertex_count;
	uint32_t face_count;
	uint32_t magic, tmp;
	int n, nn;
	double *pot;
	FSFile fs(f);

#define FS_CURV_FILE_MAGIC_NUMBER (-1 & 0x00ffffff)

	if (f == NULL || msh == NULL)
		return NULL;

	if (fs.readI3(magic) || magic != FS_CURV_FILE_MAGIC_NUMBER)
		return NULL;

	if (fs.readI4(vertex_count))
		return NULL;

	if (fs.readI4(face_count))
		return NULL;

	if (fs.readI4(tmp) || tmp != 1)
		return NULL;

	nn = msh->getNumVerts();
	if (nn != vertex_count)
		return NULL;

	pot = new double[nn];

	for (n = 0; n < nn; n++) {
		if (fs.readF4(pot[n]))
			break;
	}
	
	if (n != nn) {
		delete[] pot;
		return NULL;
	}

	return pot;
}

int
ShowMeshWindow::load_node_fn(const char *fn, int mn)
{
	if (mn < 0 || mn >= num_meshes)
		return 1;

	MeshRender *r = meshes[mn];
	if (r == NULL)
		return 1;

	FILE *f=fopen(fn,"r");

	if (f == NULL) {
		perror("Failed to open!\n");
		return 1;
	}

	double *pot = loadPotFile(r->getMesh(), f);
	if (pot == NULL) {
		fseek(f, 0, SEEK_SET);
		pot = loadFSCurvFile(r->getMesh(), f);
		if (pot == NULL) {
			fseek(f, 0, SEEK_SET);
			pot = loadFSWFile(r->getMesh(), f);
		}
	}

	if (pot) {
		r->setNField(pot);
		r->setFlag(MRF_SHOW_NCOLOR);
		double n0, n1;
		r->getNFRange(n0, n1);
		printf("Nfield range: [%g %g]\n", n0, n1);

	} else
		return 1;

	fclose(f);

	return 0;
}

int
ShowMeshWindow::load_node_background_fn(const char *fn, int mn)
{
	if (mn < 0 || mn >= num_meshes)
		return 1;

	MeshRender *r = meshes[mn];
	if (r == NULL)
		return 1;

	FILE *f=fopen(fn,"r");

	if (f == NULL) {
		perror("Failed to open!\n");
		return 1;
	}

	double *pot = loadPotFile(r->getMesh(), f);
	if (pot == NULL) {
		fseek(f, 0, SEEK_SET);
		pot = loadFSCurvFile(r->getMesh(), f);
	}

	if (pot) {
		double n0, n1;
		r->setFlag(MRF_SHOW_NBGRND);
		r->setNBackground(pot);
	} else
		return 1;

	fclose(f);

	return 0;
}

int
ShowMeshWindow::set_background_alpha(double alpha, int mn)
{
	if (mn < 0 || mn >= num_meshes)
		return 1;

	MeshRender *r = meshes[mn];
	if (r == NULL)
		return 1;

	if (alpha == 0)
		r->clearFlag(MRF_SHOW_NBGRND);
	else {
		r->setFlag(MRF_SHOW_NBGRND);
		r->setBackgroundAlpha(alpha);
	}

	return 0;
}

int
ShowMeshWindow::set_node_range(double rmin, double rmax, int mn)
{
	if (mn < 0 || mn >= num_meshes)
		return 1;

	MeshRender *r = meshes[mn];
	if (r == NULL)
		return 1;

	r->setNFRange(rmin, rmax);
	r->getNFRange(rmin, rmax);
	printf("Nfield range: [%g %g]\n", rmin, rmax);

	return 0;
}

int
ShowMeshWindow::set_node_auto(int zero, int mn)
{
	if (mn < 0 || mn >= num_meshes)
		return 1;

	MeshRender *r = meshes[mn];
	if (r == NULL)
		return 1;

	r->setNFRangeAuto(zero);
	printf("Nfield range auto [%s]\n", zero ? "zero" : "max");

	return 0;
}

void
ShowMeshWindow::drawArrow(Point3 dc, Point3 dn, double scale)
{
	if (scale == 0)
		return;

	Point3 dn1, dn2;

        if (fabs(dn.getZ()) > fabs(dn.getY())) {
		if (fabs(dn.getZ()) > fabs(dn.getX()))
			dn1 = Point3(dn.getZ(), 0, -dn.getX());
		else
			dn1 = Point3(-dn.getY(), dn.getX(), 0);
	} else {
		if (fabs(dn.getY()) > fabs(dn.getX()))
			dn1 = Point3(dn.getY(), -dn.getX(), 0);
		else
			dn1 = Point3(-dn.getZ(), 0, dn.getX());
	}

	dn1.normalize();
	dn2=Cross(dn1, dn);
	dn2.normalize();

	scale *= m_delsc;
	dn *= scale*6;
	dn1*= scale;
	dn2*= scale;

	Point3 v0(dc + dn);
	Point3 v1(dc + dn1);
	Point3 v2(dc - dn2);
	Point3 v3(dc - dn1);
	Point3 v4(dc + dn2);

	dn.normalize();
	dn1.normalize();
	dn2.normalize();

	glBegin(GL_TRIANGLE_FAN);

	glNormal3f(dn.X(), dn.Y(), dn.Z());
	glVertex3f(v0.X(), v0.Y(), v0.Z());

	glNormal3f(dn1.X(), dn1.Y(), dn1.Z());
	glVertex3f(v1.X(), v1.Y(), v1.Z());

	glNormal3f(-dn2.X(), -dn2.Y(), -dn2.Z());
	glVertex3f(v2.X(), v2.Y(), v2.Z());

	glNormal3f(-dn1.X(), -dn1.Y(), -dn1.Z());
	glVertex3f(v3.X(), v3.Y(), v3.Z());

	glNormal3f(dn2.X(), dn2.Y(), dn2.Z());
	glVertex3f(v4.X(), v4.Y(), v4.Z());

	glNormal3f(dn1.X(), dn1.Y(), dn1.Z());
	glVertex3f(v1.X(), v1.Y(), v1.Z());

	glEnd();

	glBegin(GL_QUADS);

	glNormal3f(-dn.X(), -dn.Y(), -dn.Z());
	glVertex3f(v1.X(), v1.Y(), v1.Z());
	glVertex3f(v4.X(), v4.Y(), v4.Z());
	glVertex3f(v3.X(), v3.Y(), v3.Z());
	glVertex3f(v2.X(), v2.Y(), v2.Z());

	glEnd();
}

void
ShowMeshWindow::drawNode(const Point3 &nd, double scale)
{
	double d = m_delsc * 2 * scale;
	double x=nd.getX();
	double y=nd.getY();
	double z=nd.getZ();

	if (scale == 0)
		return;

	glBegin(GL_TRIANGLE_FAN);

	glNormal3f(1, 0, 0);
	glVertex3f(x+d, y, z);

	glNormal3f(0, 1, 0);
	glVertex3f(x, y+d, z);

	glNormal3f(0, 0, 1);
	glVertex3f(x, y, z+d);

	glNormal3f(0, -1, 0);
	glVertex3f(x, y-d, z);

	glNormal3f(0, 0, -1);
	glVertex3f(x, y, z-d);

	glNormal3f(0, 1, 0);
	glVertex3f(x, y+d, z);

	glEnd();

	glBegin(GL_TRIANGLE_FAN);

	glNormal3f(-1, 0, 0);
	glVertex3f(x-d, y, z);

	glNormal3f(0, 1, 0);
	glVertex3f(x, y+d, z);

	glNormal3f(0, 0, -1);
	glVertex3f(x, y, z-d);

	glNormal3f(0, -1, 0);
	glVertex3f(x, y-d, z);

	glNormal3f(0, 0, 1);
	glVertex3f(x, y, z+d);

	glNormal3f(0, 1, 0);
	glVertex3f(x, y+d, z);

	glEnd();
}

void
ShowMeshWindow::drawDipole(const DInfo &dip)
{
	Point3 dc(dip.D);
	Point3 dn(dip.J);

	if (dn.length() == 0)
		return;

	dn.normalize();

	glColor4f(dip.Color.getX(),dip.Color.getY(),dip.Color.getZ(),1);

	drawNode(dc, dip.scale);
	drawArrow(dc, dn, dip.scale);
}


int
ShowMeshWindow::mark_elem(int mn, int idx, bool nbrs)
{
	if (mn < 0 || mn >= num_meshes)
		return 1;

	TriMeshLin *mesh = meshes[mn]->getMesh();

	if (mesh == NULL)
		return 1;

	double *ef = new double[mesh->getNumTris()];
	memset(ef, 0, sizeof(double) * mesh->getNumTris());

	if (nbrs) {
		for (int i = 0; i < 3; i++) {
			int node = mesh->getElemInd(idx, i);
			Neighbor &nb = mesh->getFaceNbrs(node);
			for (int n = 0; n < nb.count(); n++) {
				unsigned int el = nb[n];
				if (el >= mesh->getNumTris()) {
					printf("Invalid neighbor %x\n", el);
					continue;
				}
				ef[el] = 0.5;
			}
		}
	}

	ef[idx] = 1;

	meshes[mn]->setEField(ef);
	meshes[mn]->setFlag(MRF_SHOW_ECOLOR);

	delete[] ef;

	return (0);
}

int
ShowMeshWindow::mark_node(int mn, int idx, bool nbrs)
{
	if (mn < 0 || mn >= num_meshes)
		return 1;

	TriMeshLin *mesh = meshes[mn]->getMesh();

	if (mesh == NULL)
		return 1;

	Point3 p = mesh->getVertex(idx);

	pfield.clear();

	pfield.push_back(p);

	if (nbrs) {
		Neighbor &nb = mesh->getNodeNbrs(idx);
		for (int n = 0; n < nb.count(); n++) {
			unsigned int nd = nb[n];
			if (nd >= mesh->getNumVerts()) {
				printf("Invalid neighbor %x\n", nd);
				continue;
			}
			pfield.push_back(mesh->getVertex(nd));
		}
	}

	return 0;
}

int
ShowMeshWindow::loadPointField(const char *fname)
{
	char buf[1024];

	printf("Reading point field from %s\n", fname);

	FILE * f = fopen(fname, "r");
	if (f == NULL) {
		perror("fopen");
		return 1;
	}

	pfield.clear();

	int line = 0;
	while (fgets(buf, sizeof(buf), f)) {
		double x, y, z;
		char *e = strchr(buf, '\n');
		line++;

		if (e == NULL) {
			fprintf(stderr, "line %d: too long\n", line);
			return 1;
		}
		*e = '\0';

		if (*buf == '\0' || *buf == '#')
			continue;

		if (sscanf(buf, "%lg %lg %lg", &x, &y, &z) != 3) {
			printf("line %d: invalid number of entries\n", line);
			return 1;
		}

//		printf("%g %g %g\n", x, y, z);

		pfield.push_back(Point3(x - m_mx, y - m_my, z - m_mz));
	}

	printf("Done, %lu points read\n", pfield.size());
	return 0;
}

int
ShowMeshWindow::savePointField(const char *fname)
{
	char buf[1024];

	if (pfield.empty()) {
		printf("No points to save\n");
		return (0);
	}
	printf("Saving point field to %s\n", fname);

	FILE * f = fopen(fname, "w");
	if (f == NULL) {
		perror("fopen");
		return 1;
	}
	for (int n = 0; n < pfield.size(); n++) {
		double x, y, z;
		pfield[n].getCoord(x, y, z);
		fprintf(f, "%g %g %g\n", x + m_mx, y + m_my, z + m_mz);
	}
	fclose(f);

	return (0);
}

int
ShowMeshWindow::drawPointField(void)
{
	glColor4f(1,0,0,1);

	double d = m_delsc;

	glPushMatrix();
	glTranslatef(pf_off.getX(), pf_off.getY(), pf_off.getZ());
	glRotatef(pf_rot.getZ(), 0.0f, 0.0f, 1.0f);
	glRotatef(pf_rot.getY(), 0.0f, 1.0f, 0.0f);
	glRotatef(pf_rot.getX(), 1.0f, 0.0f, 0.0f);
	glScalef(pf_scale.getX(), pf_scale.getY(), pf_scale.getZ());

	int np = pfield.size();
	for (int n = 0; n < np; n++) {
		double x, y, z;
		pfield[n].getCoord(x, y, z);

		glBegin(GL_TRIANGLE_FAN);

		glNormal3f(1, 0, 0);
		glVertex3f(x+d, y, z);

		glNormal3f(0, 1, 0);
		glVertex3f(x, y+d, z);
		
		glNormal3f(0, 0, 1);
		glVertex3f(x, y, z+d);
		
		glNormal3f(0, -1, 0);
		glVertex3f(x, y-d, z);
		
		glNormal3f(0, 0, -1);
		glVertex3f(x, y, z-d);
		
		glNormal3f(0, 1, 0);
		glVertex3f(x, y+d, z);
		
		glEnd();

		glBegin(GL_TRIANGLE_FAN);

		glNormal3f(-1, 0, 0);
		glVertex3f(x-d, y, z);

		glNormal3f(0, 1, 0);
		glVertex3f(x, y+d, z);
		
		glNormal3f(0, 0, -1);
		glVertex3f(x, y, z-d);

		glNormal3f(0, -1, 0);
		glVertex3f(x, y-d, z);

		glNormal3f(0, 0, 1);
		glVertex3f(x, y, z+d);
		
		glNormal3f(0, 1, 0);
		glVertex3f(x, y+d, z);
		
		glEnd();
	}

	glEnd();

	glPopMatrix();
	
	return 0;
}

void
ShowMeshWindow::drawAxis(void)
{
	double d = (w() + h()) /50;

	flatMode(true);

	glPushMatrix();
	
	glTranslatef(1.8*d, 1.8*d, 0);

	glRotatef(rot.getX(), 1.0f, 0.0f, 0.0f);
	glRotatef(rot.getY(), 0.0f, 1.0f, 0.0f);
	glRotatef(rot.getZ(), 0.0f, 0.0f, 1.0f);

	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glColor3f(1,1,1);

	glBegin(GL_LINES);

	glVertex3f(0, 0, 0);
	glVertex3f(d, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, d, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, d);

#if 0
	if (sdrotate) {
		double r, t, p;
		r = d;
		t = sd_theta;
		p = sd_phi;
		sph2rect(r, t, p);
		glColor3f(0,1,1);
		glVertex3f(0,0,0);
		glVertex3f(r, t, p);
		glColor3fv(fgColor.color);
	}
#endif
	glEnd();
	
	d *= 1.4;

	text.out(d, 0, 0, "x");
	text.out(0, d, 0, "y");
	text.out(0, 0, d, "z");

	glPopAttrib();

	glPopMatrix();

	flatMode(false);
}

int
ShowMeshWindow::save_mesh(const char *fn, int mn, int fc)
{
	int ret;
	TriMeshLin *mesh = meshes[mn]->getMesh();
			
        mesh->moveMesh(m_mx, m_my, m_mz);

	if (fc < 0)
		ret = mesh->save(fn);
	else
		ret = mesh->saveClass(fn, fc);

        mesh->moveMesh(-m_mx, -m_my, -m_mz);

	return ret;
}

int
ShowMeshWindow::extract_class(int mn, int fc)
{
	TriMeshLin *mesh = meshes[mn]->getMesh();
	MeshProc mp(mesh);

//        mesh->moveMesh(m_mx, m_my, m_mz);

	int nc = mesh->getNumClasses();
	if (nc == 1) {
		printf("Mesh has only one class\n");
		return 0;
	}

	if (fc == -1) {
		printf("Selecting largest class\n");
		int mx = 0;
		for (int n = 0; n < nc; n++) {
			if (mesh->getNumTris(n) > mx) {
				mx = mesh->getNumTris(n);
				fc = n;
			}
		}
	}

	TriMeshLin *mc = mp.extractClass(fc);
	if (mc == NULL)
		return 1;

	mesh->set(*mc);
//	mesh->moveMesh(-m_mx, -m_my, -m_mz);

	delete mc;

	return 0;
}

int
ShowMeshWindow::trans_keyboard(unsigned char key, int x, int y)
{
	int handled = 1;

#if 0
	if (tselected < 0)
		tselected = 0;
	if (tselected >= num_meshes)
		tselected = num_meshes - 1;

	MeshRender *mr = meshes[tselected];
	Point3 o = mr->getOffset();
	Point3 r = mr->getRotation();
#else
	Point3 o = pf_off;
	Point3 r = pf_rot;
#endif

	switch (key)
	{
	case 'h':
	case 'H':
		help = !help;
		break;
	case 27:
	case 'X':
	case 'x':
		tmode = 0;
#if 0
		for (int m = 0; m < num_meshes; m++)
			meshes[m]->clearFlag(MRF_TRANSFORM);
#endif
		break;
	case 'q':
	case 'Q':
		r.X() += tstep;
		break;
	case 'a':
	case 'A':
		r.X() -= tstep;
		break;
	case 'w':
	case 'W':
		r.Y() += tstep;
		break;
	case 's':
	case 'S':
		r.Y() -= tstep;
		break;
	case 'e':
	case 'E':
		r.Z() += tstep;
		break;
	case 'd':
	case 'D':
		r.Z() -= tstep;
		break;
	case 'p':
	case 'P':
		o.X() += tstep;
		break;
	case 'l':
	case 'L':
		o.X() -= tstep;
		break;
	case 'o':
	case 'O':
		o.Y() += tstep;
		break;
	case 'k':
	case 'K':
		o.Y() -= tstep;
		break;
	case 'i':
	case 'I':
		o.Z() += tstep;
		break;
	case 'j':
	case 'J':
		o.Z() -= tstep;
		break;
	case 'c':
	case 'C':
		o = Point3(0,0,0);
		r = Point3(0,0,0);
		pf_scale = Point3(1,1,1);
		break;
	case 'm':
	case 'M':
		tselected++;
		if (tselected == num_meshes)
			tselected = 0;
		break;
	case 'v':
	case 'V':
		wire = !wire;
		update++;
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
		m_delsc = (key - '0');
		break;
	case '<':
		tstep /= 10;
		if (tstep < 0.01)
			tstep = 0.01;
		break;
	case '>':
		tstep *= 10;
		if (tstep > 100)
			tstep = 100;
		break;
	default:
		handled = 0;
	}

	if (handled) {
#if 0
		mr->setOffset(o);
		mr->setRotation(r);
#else
		pf_off = o;
		pf_rot = r;
#endif

	}

	return (1);
}

int
ShowMeshWindow::keyboard(unsigned char key, int x, int y)
{
	static int seq=0;
	static char buf[100];

	int handled = 1;

	if (tmode && trans_keyboard(key, x, y))
		goto skip;

	switch (key)
	{
	case 27:
	case 'q':
	case 'Q':
		ui->main_window->hide();
	break;
	case 'a':
	case 'A':
		ani = !ani;
		break;
	case 'w':
	case 'W':
		wire = !wire;
		update++;
		break;
	case 'l':
	case 'L':
		light = !light;
		update++;
		break;
	case 'i':
	case 'I':
		setInterp(!interp);
		break;
	case 'e':
	case 'E':
		edges = !edges;
		for (int n=0; n < num_meshes; n++) {
			if (edges) meshes[n]->setFlag(MRF_SHOW_EDGES);
			else meshes[n]->clearFlag(MRF_SHOW_EDGES);
		}
		break;
	case 'b':
		numcorrect++;
		break;
	case 'B':
		numcorrect += 5;
		break;
	case 'c':
	case 'C':
		glcull = !glcull;
		update++;
		break;
	case 'h':
	case 'H':
		help = !help;
		break;
	case 'F':
	{
		int nc = 1;
		for (int n = 0; n < num_meshes; n++) {
			int c = meshes[n]->getNumClasses();
			if (c > nc)
				nc = c;
		}
		if (++fclass >= nc)
			fclass = 0;
		break;
	}
	case 'f':
		filter = !filter;
		break;
	case 's':
	case 'S':
		for (int n=0; n<num_meshes; n++) {
			snprintf(buf, sizeof(buf), "m%02ds%04d.smf", n, seq);
			printf("Saving %s ...\n",buf);

			if (save_mesh(buf, n, filter ? fclass : -1))
				printf("failed!\n");
			else
				printf("ok.\n");
		}
		seq++;
		break;
	case 't':
	case 'T':
		tmode = 1;
		help = 1;
//		for (int m = 0; m < num_meshes; m++)
//			meshes[m]->setFlag(MRF_TRANSFORM);
		break;
	case 'k':
	case 'K':
		capture = !capture;
		printf("Capture %s\n", capture ? "on":"off");
		break;
#if 0
	case ':':
		printf("Command: ");
		fflush(stdout);
		{
			static char buf[MAX_LINE];
			if (fgets(buf, MAX_LINE, stdin))
				execute(this, buf);
		}
		break;
	case '>' :
		printf("Script: ");
		fflush(stdout);
		{
			static char buf[MAX_LINE];
			if (fgets(buf, MAX_LINE, stdin)) {
				char *z=strchr(buf,'\n');
				if(z) *z=0;
				if (command_script(this, buf))
					printf("Failed!");
				else printf("Running %s\n", buf);
			}
		}
		break;
#endif
	case '0':
		numiter = 0;
		numcorrect = 0;
		break;
	case '1':
		numiter++;
		break;
	case '2':
		numiter+=5;
		break;
	case '3':
		numiter+=10;
		break;
	case '4':
		numiter+=100;
		break;

	case 'r':
		

	default:
		handled = 0;
	}

 skip:

	if (handled)
		redraw();

	if (!idle && (ani || numiter || numcorrect || command_need_loop())){
	    Fl::add_idle(ShowMeshWindow::glIdleCB, this);
	}

	return (handled);
}

void
ShowMeshWindow::glMotion(int x, int y)
{

	int dx = x - m_x;
	int dy = y - m_y;
	m_x = x;
	m_y = y;

	if (m_btn == 2)
		eye.Z() += ((float) (dx + dy) * 4.0);
	else if (m_btn ==  1) {
		rot.X() += ((float) dy / 2.0);
		rot.Y() += ((float) dx / 2.0);
	}
	else if (m_btn == 3)
		rot.Z() += ((float) dx / 2.0);

	redraw();
}

int
ShowMeshWindow::improve_mesh(int mn, int cnt, double aspect, double esize)
{
	static int sid = 0;
	char name[64];

	TriMeshLin *mesh = meshes[mn]->getMesh();

	if (mesh == NULL)
		return 1;

	for (int n = 0; n < cnt; n++) {
		int ret;
		MeshProc mp(mesh);
		double avg =  mp.averageEdgeDistance();
		printf("Average Edge Distance: %g\n", avg);

#if 0
		mp.mergeVertices(0);
#endif
		ret = mesh->flipElements();
		printf(" >> flipped %d edges\n", ret);
		ret = mesh->removeBadAspectElements(aspect);
		printf(" >> removed %d bad aspect elements\n", ret);
		ret = mesh->removeSmallElements(avg * esize);
		printf(" >> removed %d small elements\n", ret);
		ret = mp.flipSharpEdges();
		printf(" >> flipped %d sharp edges\n", ret);

#if 0
		double *ef = new double[mesh->getNumTris()];
		mp.mergeElements(ef);
		mesh->recalculateEdges();

		int flag = 0;
		set<unsigned int> nbrs;
		for (int i = 0; i < mesh->getNumTris(); i++) {
			if (ef[i])
				flag++;
			else
				continue;
			for (int m = 0; m < 3; m++) {
				int v = mesh->getElemInd(i, m);
				Neighbor &nb = mesh->getFaceNbrs(v);
				printf("e: %d, v: %d nb: %d\n",
				       i, v, nb.count());
				for (int n = 0; n < nb.count(); n++)
					nbrs.insert(nb[n]);
			}
		}

		printf(">> %d elements flagged as identical \n", flag);
		printf(">> %d neighbors selected\n", nbrs.size());

		for (set<unsigned int>::iterator it = nbrs.begin();
		     it != nbrs.end(); it++)
			ef[*it] += 0.1;

		meshes[mn]->setEField(ef);
		meshes[mn]->setFlag(MRF_SHOW_ECOLOR);
		snprintf(name, sizeof(name), "merged_%02d.dat", sid);
		FILE *f = fopen(name, "wb");
		for (int i = 0; i < mesh->getNumTris(); i++) {
			fprintf(f, "%d %g\n", i, ef[i]);
		}
		fclose(f);
		snprintf(name, sizeof(name), "merged_%02d.smf", sid);
		mesh->save(name);
		sid++;
		delete[] ef;
#endif
	}

	return 0;
}

int
ShowMeshWindow::correct_mesh(int mn)
{
	TriMeshLin *mesh = meshes[mn]->getMesh();
	int tries;

	if (mesh == NULL)
		return 1;

	printf(" >> Correcting ...\n");

	for (tries = 0; tries < MAX_CORRECT; tries++)
		if (!mesh->correctMesh())
			break;

	if (tries == MAX_CORRECT)
		printf(" >> Failed to correct ...\n");
	else
		printf(" >> Corrected in %d tries ...\n", tries + 1);

	return 0;
}

int
ShowMeshWindow::split_mesh(int mn, double thresh)
{
	TriMeshLin *mesh = meshes[mn]->getMesh();

	if (mesh == NULL)
		return 1;

	MeshProc mp(mesh);
	printf("Average Edge Distance: %g\n", mp.averageEdgeDistance());
	printf("Splitting Edges ...\n");
	mp.splitEdges(thresh);
	printf("Average Edge Distance: %g\n", mp.averageEdgeDistance());
	return 0;
}

int
ShowMeshWindow::split_intersecting(int mn)
{
	TriMeshLin *mesh = meshes[mn]->getMesh();

	if (mesh == NULL)
		return 1;

	MeshProc mp(mesh);
	double d = mp.averageEdgeDistance();
	double md = mp.minimumEdgeDistance();

	printf("Average Edge Distance: %g\n", d);
	printf("Minimum Edge Distance: %g\n", md);
	mp.mergeVertices(md/10);

	printf("Splitting intersecting elements ...\n");
	mp.splitIntersecting();
	mesh->printIntersectionBoundaries();
	return 0;
}

int
ShowMeshWindow::process_intersecting(int mn, int fix)
{
	TriMeshLin *mesh = meshes[mn]->getMesh();
	int ni;

	if (mesh == NULL)
		return 1;

	MeshProc mp(mesh);

	if (fix) {
		printf("Fixing for intersecting elements ...\n");
		ni = mp.pushIntersecting();

	} else {
		printf("Looking for intersecting elements ...\n");
		ni = mp.printIntersecting();
	}

	if (ni)
		printf("%d intersections detected\n", ni);
	else
		printf("no intersections\n");

	return (0);
}

int
ShowMeshWindow::process_sharp_edges(int mn, int fix)
{
	TriMeshLin *mesh = meshes[mn]->getMesh();
	int ni;

	if (mesh == NULL)
		return 1;

	MeshProc mp(mesh);

	printf("Looking for sharp edges ...\n");

	if (fix)
		ni = mp.flipSharpEdges();
	else
		ni = mp.printSharpEdges();		

	if (ni)
		printf("%d elements have sharp edges\n", ni);
	else
		printf("no sharp edges\n");

	return (0);
}

void
ShowMeshWindow::glIdle(void)
{
	int post = 0, ret;
	idle = 1;

	if (numcorrect) {
		printf("correcting: %d iterations left ...\n", numcorrect);
		for (int n = 0; n < num_meshes; n++)
			correct_mesh(n);

		numcorrect--;
		if (numcorrect == 0)
			printf("Correcting done.\n");
		post = 1;
	}

	if (numiter) {
		for (int n = 0; n < num_meshes; n++)
			smooth_mesh(n);

		numiter--;
		if (numiter == 0)
			printf("Smoothing done.\n");
		post = 1;
	}

	if (ani) {
		rot.Z() += 1.5;
		if (rot.Z() > 360) rot.setZ(0);
		post=1;
	}

	post += command_loop(this);

   	if(post)
		redraw();

	if (numiter == 0 && ani == 0 && numcorrect == 0 && command_need_loop() == 0) {
#if 0
		printf("REMOVE IDLE\n");
#endif
		idle = 0;
		Fl::remove_idle(ShowMeshWindow::glIdleCB, this);
	}
}

void
ShowMeshWindow::glInit()
{
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glEnable       ( GL_LIGHTING );
	glLightModelfv ( GL_LIGHT_MODEL_AMBIENT, m_ambient_light );
	glLightfv      ( GL_LIGHT0,GL_DIFFUSE, m_source_light );
	glLightfv      ( GL_LIGHT0,GL_POSITION, m_light_pos );
	glEnable       ( GL_LIGHT0 );

	// Enable material properties for lighting
	glEnable        ( GL_COLOR_MATERIAL );
	glColorMaterial ( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
	
	SetRenderModes();

	glClearColor(0, 0, 0.1, 1.0);
	glResize(w(), h());
	if (!idle && (ani || numiter || numcorrect || command_need_loop()))
		Fl::add_idle(ShowMeshWindow::glIdleCB, this);
}

// setup and load the mesh
MeshRender *
ShowMeshWindow::addMesh(const char *fn)
{
        if (num_meshes >= MAX_MESHES) {
                printf ("Too many meshes!\n");
                return (NULL);
        }

        TriMeshLin *mesh = new TriMeshLin();
//        mesh->setScale(2.2);
        if(mesh->loadMesh(fn)){
                printf("failed to load mesh %s!\n",fn);
                return (NULL);
        }
        mesh->scaleMesh(1,1,1);

        if (num_meshes == 0)
                mesh->getMean().getCoord(m_mx, m_my, m_mz);

        mesh->moveMesh(-m_mx, -m_my, -m_mz);

        MeshRender *mr = new MeshRender(*mesh);
        meshes[num_meshes++] = mr;

	double dmax = 0;
	for (int n=0; n<num_meshes; n++) {
		TriMeshLin *mesh = meshes[n]->getMesh();
		double dx = mesh->getWidth();
		double dy = mesh->getWidth();
		double dz = mesh->getWidth();
		double d = sqrt(dx*dx+dy*dy+dz*dz);
		if (dmax < d) dmax = d;
	}
	eye.setZ((float) ((dmax / 2.0)+100));
        
        return mr;
}

void
ShowMeshWindow::setView(double x, double y, double z)
{
        while (x < -360) x +=360;
        while (x >  360) x -=360;
        while (y < -360) y +=360;
        while (y >  360) y -=360;
        while (z < -360) z +=360;
        while (z >  360) z -=360;
	rot.setCoord(x,y,z);
}

void
ShowMeshWindow::setEye(double x, double y, double z)
{
	eye.setCoord(x,y,z);
}

void
ShowMeshWindow::addView(double x, double y, double z)
{
        x+=rot.getX();
	y+=rot.getY();
        z+=rot.getZ();
	setView(x,y,z);
}

void
ShowMeshWindow::addEye(double x, double y, double z)
{
	eye+=Point3(x,y,z);
}

int
ShowMeshWindow::saveEPS(const char *fn, int sel)
{
	FILE *fp;
	int buffsize = 0, state = GL2PS_OVERFLOW;

	printf("Saving display to %s\n", fn);

	if ((fp = fopen(fn, "w")) == NULL) {
		perror("saveEPS");
		return (1);
	}

	text.epsOn();
	unsigned options = GL2PS_SIMPLE_LINE_OFFSET | GL2PS_SILENT |
		GL2PS_OCCLUSION_CULL | GL2PS_BEST_ROOT | GL2PS_NO_PS3_SHADING;

	if (sel)
		options |= GL2PS_GRAY;

	while (state == GL2PS_OVERFLOW) {
		buffsize += 1024*1024;
		gl2psBeginPage ( fn, "ShowMesh",
				 GL2PS_EPS, GL2PS_BSP_SORT, options,
				 GL_RGBA, 0, NULL, buffsize, fp, NULL );
		gl2psLineWidth(0);
		glDisplay();
		state = gl2psEndPage();
	}

	text.epsOff();

	fclose(fp);
	return (0);
}
//---------------------------------------------------------------
void
ShowMeshWindow::draw()
{
	if (!valid()) {
		// set up projection, viewport, etc ...
		// window size is in w() and h().
		// valid() is turned on by FLTK after draw() returns
		glInit();
	} else if (!idle && (ani || numiter || numcorrect || command_need_loop()))
		Fl::add_idle(ShowMeshWindow::glIdleCB, this);

	if (update) {
		SetRenderModes();
		update = 0;
	}
	glDisplay();
}

int
ShowMeshWindow::handle(int event) 
{
	switch(event) {
	case FL_PUSH:
	{
		//... mouse down event ...
		//... position in Fl::event_x() and Fl::event_y()
		m_x = Fl::event_x();
		m_y = Fl::event_y();
		m_btn = Fl::event_button();
		// emulate 3 buttons
		int s = Fl::event_state();
		if ((s & (FL_BUTTON1 | FL_BUTTON3)) ==
		    (FL_BUTTON1 | FL_BUTTON3))
			m_btn = 2;
		take_focus();
		return 1;
	}
	case FL_DRAG:
		//... mouse moved while down event ...
		glMotion(Fl::event_x(), Fl::event_y());
		return 1;
	case FL_RELEASE:    
		//... mouse up event ...
		m_btn = 0;
		return 1;
	case FL_FOCUS :
	case FL_UNFOCUS :
		//... Return 1 if you want keyboard events, 0 otherwise
		return 1;
	case FL_KEYBOARD:
		//... keypress, key is in Fl::event_key(), ascii in Fl::event_text()
		//... Return 1 if you understand/use the keyboard event, 0 otherwise...
		return (keyboard(*Fl::event_text(),
		    Fl::event_x(), Fl::event_y()));
	case FL_SHORTCUT:
		//... shortcut, key is in Fl::event_key(), ascii in Fl::event_text()
		//... Return 1 if you understand/use the shortcut event, 0 otherwise...
		return 0;
	default:
		// pass other events to the base class...
		return Fl_Gl_Window::handle(event);
	}
}
