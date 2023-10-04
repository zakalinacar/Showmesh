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
#ifndef _SHOWMESH_H_
#define _SHOWMESH_H_

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include "meshrender.h"
#include "glcapture.h"

#include "gluttext.h"

#define MAX_MESHES 20

class DInfo {
public:
	DInfo() : Color(1, 1, 1),scale(1),show(false) {}
	Point3 D;
	Point3 J;
	Point3 Color;
	double scale;
	bool show;
};

class ShowMeshWindow : public Fl_Gl_Window
{
public:
	ShowMeshWindow(int X, int Y, int W, int H, const char *L = NULL);

	MeshRender *addMesh(const char *fn);
	void setView(double x, double y, double z);
	void setEye(double x, double y, double z);
	void addView(double x, double y, double z);
	void addEye(double x, double y, double z);

	const Point3 &getView() const {
		return rot;
	}

	const Point3 &getEye() const {
		return eye;
	}

	int save_mesh(const char *fn, int mn, int fc = -1);
	int load_node_fn(const char *fn, int mn);
	int load_node_background_fn(const char *fn, int mn);
	int set_node_range(double rmin, double rmax, int mn);
	int set_node_auto(int zero, int mn);
	int extract_class(int mn, int fc);
	int set_background_alpha(double alpha, int mn);

	int smooth_mesh(int mn, int cnt = 1) {
		TriMeshLin *mesh = meshes[mn]->getMesh();
		if (mesh == NULL)
			return 1;
		mesh->smooth(cnt);
		return 0;
	}

	int fill_holes(int mn) {
		TriMeshLin *mesh = meshes[mn]->getMesh();
		if (mesh == NULL)
			return 1;
		return (mesh->fillHoles());
	}

	int correct_mesh(int mn);
	int improve_mesh(int mn, int cnt = 1,
	    double aspect = 0.001, double esize = 1000);
	int split_mesh(int mn, double thresh);
	int split_intersecting(int mn);
	int process_intersecting(int mn, int fix);
	int process_sharp_edges(int mn, int fix);

	int mark_elem(int mn, int idx, bool nbrs);
	int mark_node(int mn, int idx, bool nbrs);

	void setAmbientLight(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0) {
		m_ambient_light[0] = r;
		m_ambient_light[1] = g;
		m_ambient_light[2] = b;
		m_ambient_light[3] = a;
	}
	void setSourceLight(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0) {
		m_source_light[0] = r;
		m_source_light[1] = g;
		m_source_light[2] = b;
		m_source_light[3] = a;
	}
	void setLightPos(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0) {
		m_light_pos[0] = r;
		m_light_pos[1] = g;
		m_light_pos[2] = b;
		m_light_pos[3] = a;
	}

	int numMeshes(void) {
		return num_meshes;
	}
	TriMeshLin *getMesh(int n) {
		return (n < 0 || n > num_meshes) ? NULL: meshes[n]->getMesh();
	}
	void setFlag(int n, unsigned flag) {
		meshes[n]->setFlag(flag);
	}
	void clearFlag(int n, unsigned flag) {
		meshes[n]->clearFlag(flag);
	}
	void setAlpha(int n, double a) {
		meshes[n]->setAlpha(a);
	}

	void setColor(int n, double r, double g, double b) {
		meshes[n]->setColor(r, g, b);
	}

	bool getCapture(void) {
		return capture;
	}

	void setCapture(bool cap) {
		capture = cap;
	}

	bool getWireframe(void) {
		return wire;
	}

	void setWireframe(bool w) {
		wire = w;
		SetRenderModes();
	}

	bool getInterp(void) {
		return interp;
	}

	void setInterp(bool i);

	bool getCullFaces(void) {
		return glcull;
	}

	void setCullFaces(bool c) {
		glcull = c;
		SetRenderModes();
	}

	bool getRevOrder(void) {
		return revorder;
	}

	void setRevOrder(bool r) {
		revorder = r;
	}

	bool getLight(void) {
		return light;
	}

	void setLight(bool l) {
		light = l;
		SetRenderModes();
	}

	void getMeshOffset(double &mx, double &my, double &mz) {
		mx = m_mx;
		my = m_my;
		mz = m_mz;
	}

	Point3 getMeshOffset(void) {
		return Point3(m_mx, m_my, m_mz);
	}

	void setDipole(int d, double x, double y, double z,
		       double px, double py, double pz, bool show = true) {
		m_dip.resize(d+1);
		m_dip[d].D.setCoord(x - m_mx, y - m_my, z - m_mz);
		m_dip[d].J.setCoord(px, py, pz);
		m_dip[d].show = show;
	}
	void setDipoleColor(int d, double x, double y, double z) {
		if (d < 0 || d >= m_dip.size())
		    return;
		m_dip[d].Color.setCoord(x, y, z);
	}
	void setDipoleScale(int d, double s) {
		if (d < 0 || d >= m_dip.size())
			return;
		m_dip[d].scale = s;
	}

	void setPOffset(double x, double y, double z) {
		pf_off.setCoord(x, y, z);
	}
	void setPRotation(double x, double y, double z) {
		pf_rot.setCoord(x, y, z);
	}
	void setPScale(double x, double y, double z) {
		pf_scale.setCoord(x, y, z);
	}

	void setClip(double x0, double y0, double z0,
		     double x1, double y1, double z1);
	void setClip(bool on);


	void drawArrow(Point3 dc, Point3 dn, double s);
	void drawDipole(const DInfo &dip);
	void drawNode(const Point3 &nd, double s = 1);
	int drawPointField(void);
	void drawAxis(void);
	int loadPointField(const char *fname);
	int savePointField(const char *fname);
	int saveEPS(const char *fn, int sel);

private:
	void draw();
	int handle(int);

	void flatMode(bool enable);
	void SetRenderModes(void);
	void DrawHelp(void);
	void DrawTransformHelp(int fsize, int &y);

	void glDisplay(void);
	void glResize(int w, int h);
	void glMouse(int button, int state, int x, int y);
	void glMotion(int x, int y);
	void glInit(void);
	void glIdle(void);

	int keyboard(unsigned char key, int x, int y);
	int trans_keyboard(unsigned char key, int x, int y);

	static void glIdleCB(void *obj) {
		((ShowMeshWindow *)obj)->glIdle();
	}

	double *loadPotFile(TriMeshLin *msh, FILE *f);
	double * loadFSCurvFile(TriMeshLin *msh, FILE *f);
	double * loadFSWFile(TriMeshLin *msh, FILE *f);

	Point3 eye;
	Point3 rot;
	bool idle;
	bool ani;	// animate?
	bool capture;	// capture
	bool filter;
	bool glcull;	// gl culling?
	bool help;	// help?
	bool interp;	// interpolated normals
	bool light;	// lighting?
	bool wire;	// wireframe?
	bool revorder;	// mesh drawing order reversed?
	bool edges;
	bool tmode;

	GLCapture *glcap;
	GlutText text;

	GLfloat m_ambient_light[4];
	GLfloat  m_source_light[4];
	GLfloat     m_light_pos[4];

	MeshRender *meshes[MAX_MESHES];

	int num_meshes;
	int numiter;
	int numcorrect;
	int update;
	int fclass;
	double tstep;
	int tselected;

	int num_pf;
	vector<Point3> pfield;
	Point3 pf_off, pf_rot, pf_scale;

	double m_delsc;

	// mesh mean
	double m_mx, m_my, m_mz;

	// mouse data
	int m_x, m_y, m_btn;

	vector<DInfo> m_dip;
};

#endif
