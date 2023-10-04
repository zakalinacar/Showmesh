//---------------------------------------------------------------------------
/* $Id: meshrender.h,v 1.7 2008/03/14 03:12:47 canacar Exp $ */
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
#ifndef meshrenderH
#define meshrenderH
//---------------------------------------------------------------------------
#include "mesh.h"

#define MRF_HIDDEN	 0x001
#define MRF_INTERP	 0x002
#define MRF_BOUND	 0x004
#define MRF_SHOW_EDGES	 0x008
#define MRF_SHOW_ECOLOR  0x010
#define MRF_SHOW_NCOLOR  0x020
#define MRF_CLIP	 0x040
#define MRF_TRANSFORM	 0x080
#define MRF_SHOW_EBGRND  0x100
#define MRF_SHOW_NBGRND  0x200

#define CMAP_SIZE       16384

// property flags
#define PRF_CLIP	0x01

struct GColor {
	GColor(){}
	GColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1)
        {color[0] = r; color[1] = g; color[2] = b; color[3] = a;}

	inline GLfloat &R() {return color[0];}
	inline GLfloat &G() {return color[1];}
	inline GLfloat &B() {return color[2];}
	inline GLfloat &A() {return color[3];}
	inline void assign(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1)
        { color[0]=r; color[1]=g; color[2]=b; color[3] = a; }
	GLfloat color[4];
};

struct ColorProp {
	ColorProp():value(0), flags(0), color(1,1,1){}
	double value;
	double background;
	unsigned int flags;
	GColor color;   // element color
};

class MeshRender {
public:
        MeshRender(TriMeshLin &msh);
        ~MeshRender(void) {};

        inline void setFlag(unsigned flag) {
		m_flags |= flag;
	}
        inline void clearFlag(unsigned flag) {
		m_flags &= ~flag;
	}
        inline unsigned getFlag(unsigned flag)
                { return m_flags & flag; }
        inline void setAlpha(double a) {
                if (a < 0) a = 0;
                else if (a > 1) a = 1;
                if (m_alpha != a) {
			m_alpha = a;
			updateAlpha();
		}
        }

        inline void setColor(double r, double g, double b) {
                if (r<0) m_r = 0;
                else if (r>1) m_r = 1;
                else m_r = r;

                if (g<0) m_g = 0;
                else if (g>1) m_g = 1;
                else m_g = g;

                if (b<0) m_b = 0;
                else if (b>1) m_b = 1;
                else m_b = b;
        }

        inline void getColor(double &r, double &g, double &b)
                { r=m_r; g=m_g; b=m_b;}

        inline void getAlpha(double &a)
                { a=m_alpha; }

        inline int getNumClasses(void)
                { return m_mesh->getNumClasses();}
                
        inline TriMeshLin *getMesh(void)
                { return m_mesh; }

	void setEField(double *f, int update=1);
	void setEBackground(double *f);
	void setNField(double *f, int update=1);
	void setNBackground(double *f);
	void setEFRangeAuto(bool zero = false);
	void setNFRangeAuto(bool zero = false);

	inline void setBackgroundAlpha(double alpha) {
		if (alpha < 0)
			alpha = 0;
		else if (alpha > 1)
			alpha = 1;
		m_balpha = alpha;
		updateFields();
	}
	inline double getBackgroundAlpha(void) {
		return m_balpha;
	}
	inline GColor getEFColor(double val)
		{ return (colorMap(val, m_efldmin, m_efldmax)); }

	inline	GColor getNFColor(double val)
		{ return (colorMap(val, m_nfldmin, m_nfldmax)); }

	inline void setColorLog(void) {
		m_log = 1;
		updateFields();
	}

	inline void setColorLinear(void) {
		m_log = 0;
		updateFields();
	}

	inline void setEFRange(double min, double max) {
		m_efldmin=min; m_efldmax=max;
		updateEField();
	}

	inline void setNFRange(double min, double max) {
		m_nfldmin=min; m_nfldmax=max;
		updateNField();
	}

	inline void getEFRange(double &min, double &max) {
		min = m_efldmin; max = m_efldmax;
	}

	inline void getNFRange(double &min, double &max) {
		min = m_nfldmin; max = m_nfldmax;
	}

	void colormapFlip(void);
	void colormapGray(int quant);
	void colormapDefault(void);

	int saveFaces(FILE *f);

	inline const GColor &getEColor(int el) const {
		return m_eprops[el].color;
	}

	inline const GColor &getNColor(int nd) const {
		return m_nprops[nd].color;
	}

	inline double getEField(int el) const {
		return m_eprops[el].value;
	}

	inline double getNField(int nd) const {
		return m_nprops[nd].value;
	}

	inline double getEBackground(int el) const {
		return m_eprops[el].background;
	}

	inline double getNBackground(int nd) const {
		return m_nprops[nd].background;
	}

        int render(int filter = -1);

	void setClip(double x0, double y0, double z0,
		     double x1, double y1, double z1) {
		m_clipX0 = x0;
		m_clipY0 = y0;
		m_clipZ0 = z0;
		m_clipX1 = x1;
		m_clipY1 = y1;
		m_clipZ1 = z1;
		updateClip();
	}

	void setOffset(const Point3 &off) {
		m_move = off;
	}
	void setRotation(const Point3 &rot) {
		m_rot = rot;
	}

	const Point3 &getOffset(void) {
		return m_move;
	}
	const Point3 &getRotation(void) {
		return m_rot;
	}

protected:
	void render_surf(int cls);	
	void render_wireframe(int cls);

	GColor &colorMap(double val, double min, double max);
	inline void setEColor(int el, double val)
		{m_eprops[el].color=colorMap(val, m_efldmin, m_efldmax);}
	inline void setNColor(int nd, double val)
		{m_nprops[nd].color=colorMap(val, m_nfldmin, m_nfldmax);}

	inline void setEColor(int el, double val, double bg, double alpha) {
		GColor c(m_r, m_g, m_b, m_alpha);
		bg = bg * alpha;
		alpha = 1-alpha;
		if (m_flags & MRF_SHOW_ECOLOR)
			c = colorMap(val, m_efldmin, m_efldmax);
		m_eprops[el].color.R() = c.R() * alpha + bg;
		m_eprops[el].color.G() = c.G() * alpha + bg;
		m_eprops[el].color.B() = c.B() * alpha + bg;
		m_eprops[el].color.A() = c.A();
	}
	inline void setNColor(int nd, double val, double bg, double alpha) {
		GColor c(m_r, m_g, m_b, m_alpha);
		bg = bg * alpha;
		alpha = 1 - alpha;
		if (m_flags & MRF_SHOW_NCOLOR)
			c = colorMap(val, m_nfldmin, m_nfldmax);
		m_nprops[nd].color.R() = c.R() * alpha + bg;
		m_nprops[nd].color.G() = c.G() * alpha + bg;
		m_nprops[nd].color.B() = c.B() * alpha + bg;
		m_nprops[nd].color.A() = c.A();
	}
    
	inline void updateFields(void)
		{updateEField(); updateNField();}
	void updateEField();
	void updateNField();
	void updateAlpha();
	void updateClip();


private:
        TriMeshLin *m_mesh;

	typedef vector<ColorProp> CPropVec;

	CPropVec m_eprops;
	CPropVec m_nprops;
	GColor m_cmap[CMAP_SIZE];

        double m_alpha, m_balpha;
        double m_r, m_g, m_b;
        unsigned m_flags;

	bool m_log;
	double m_efldmin, m_efldmax;
	double m_nfldmin, m_nfldmax;

	bool m_clip;
	double m_clipX0, m_clipX1;
	double m_clipY0, m_clipY1;
	double m_clipZ0, m_clipZ1;

	Point3 m_move, m_rot;
};
#endif
