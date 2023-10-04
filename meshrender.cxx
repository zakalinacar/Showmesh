//---------------------------------------------------------------------------
// $Id: meshrender.cxx,v 1.8 2008/04/14 16:19:18 canacar Exp $
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
#include <GL/gl.h>
#include "meshrender.h"
//---------------------------------------------------------------------------
MeshRender::MeshRender(TriMeshLin &mesh)
{
        m_mesh = &mesh;
        m_alpha = 1;
	m_balpha = 0.5; /* half and half */
        m_r = m_g = m_b = 1;
        m_flags = 0;

	m_log = false;

	m_eprops.resize(m_mesh->getNumTris());
	m_nprops.resize(m_mesh->getNumVerts());
	colormapDefault();
}
//---------------------------------------------------------------------------
int
MeshRender::render(int fclass)
{
        if (m_flags & MRF_HIDDEN)
                return 0;

	if (m_flags & MRF_SHOW_EDGES) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1, 0);
	}

	if ((m_flags & MRF_SHOW_ECOLOR) &&
	    m_eprops.size() != m_mesh->getNumTris())
		m_eprops.resize(m_mesh->getNumTris());

	if ((m_flags & (MRF_SHOW_NCOLOR | MRF_SHOW_NBGRND)) &&
	    m_nprops.size() != m_mesh->getNumVerts())
		m_nprops.resize(m_mesh->getNumVerts());

	if ((m_flags & (MRF_SHOW_ECOLOR | MRF_SHOW_NCOLOR |
			MRF_SHOW_EBGRND | MRF_SHOW_NBGRND)) == 0)
		glColor4d(m_r, m_g, m_b, m_alpha);

	if (m_flags & MRF_TRANSFORM) {
		glPushMatrix();
		glTranslatef(m_move.getX(), m_move.getY(), m_move.getZ());
		glRotatef(m_rot.getZ(), 0.0f, 0.0f, 1.0f);
		glRotatef(m_rot.getY(), 0.0f, 1.0f, 0.0f);
		glRotatef(m_rot.getX(), 1.0f, 0.0f, 0.0f);
		glTranslatef(-m_mesh->getMean().getX(),
			     -m_mesh->getMean().getY(),
			     -m_mesh->getMean().getZ());
	}

        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	render_surf(fclass);

        glDisable(GL_BLEND);

	if (m_flags & MRF_SHOW_EDGES) {
		unsigned int fl = m_flags;
		m_flags &= ~(MRF_SHOW_NCOLOR | MRF_SHOW_ECOLOR);
		glDisable(GL_POLYGON_OFFSET_FACTOR);
		glColor3f(0,0,0);
		render_wireframe(fclass);
		m_flags = fl;
	}

	if (m_flags & MRF_TRANSFORM)
		glPopMatrix();

        return 0;
}
//---------------------------------------------------------------------------
void
MeshRender::render_surf(int cls)
{
        int start = 0;
        int end;

        int interp = m_flags & MRF_INTERP;

        if (cls >= 0) {
		int c;
                for (c = 0; c < cls; c++)
                        start += m_mesh->getNumTris(c);
                end = start + m_mesh->getNumTris(c);
        } else
		end = m_mesh->getNumTris();

        glBegin(GL_TRIANGLES);

        for (int t = start; t < end; t++) {
		if (m_flags & MRF_CLIP && m_eprops[t].flags & PRF_CLIP)
			continue;
                if (!interp) {
			const Point3 &n = m_mesh->getFaceNormal(t);
			glNormal3d(n.getX(), n.getY(), n.getZ());
		}

		if (m_flags & (MRF_SHOW_ECOLOR | MRF_SHOW_EBGRND))
			glColor4fv(m_eprops[t].color.color);

                for (int n = 0; n < 3; n++) {
			unsigned vi = m_mesh->getElemInd(t, n);
			if (m_flags & (MRF_SHOW_NCOLOR | MRF_SHOW_NBGRND))
				glColor4fv(m_nprops[vi].color.color);
                        if (interp) {
				const Point3 &nv = m_mesh->getVertexNormal(vi);
				glNormal3d(nv.getX(), nv.getY(), nv.getZ());
			}
			const Point3 &v  = m_mesh->getVertex(vi);
                        glVertex3d(v.getX(), v.getY(), v.getZ());
                }
        }

        glEnd();
}
//---------------------------------------------------------------------------
void
MeshRender::render_wireframe(int cls)
{
        int start = 0;
        int end;

        if (cls >= 0) {
		int c;
                for (c = 0; c < cls; c++)
                        start += m_mesh->getNumTris(c);
                end = start + m_mesh->getNumTris(c);
        } else
		end = m_mesh->getNumTris();

	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBegin(GL_TRIANGLES);


        for (int t = start; t < end; t++) {
		if (m_flags & MRF_CLIP && m_eprops[t].flags & PRF_CLIP)
			continue;
		if (m_flags & MRF_SHOW_ECOLOR)
			glColor4fv(m_eprops[t].color.color);
                for (int n = 0; n < 3; n++) {
			if (m_flags & MRF_SHOW_NCOLOR) {
				unsigned vi = m_mesh->getElemInd(t, n);
				glColor4fv(m_nprops[vi].color.color);
			}
			const Point3 &v  = m_mesh->getElemVert(t, n);
                        glVertex3d(v.getX(), v.getY(), v.getZ());
                }
        }

        glEnd();
	glPopAttrib();
}
//---------------------------------------------------------------------------
GColor &
MeshRender::colorMap(double val, double min, double max)
{
	int idx;

	if (min > max) {
		double t = max;
		max = min;
		min = t;
	}

        if (val > max) val=max;
        if (val < min) val=min;

        double del=max-min;
	if (del == 0) {
		idx = CMAP_SIZE/2;
	} else {
		double i;
		if (m_log)
			i = log1p(val-min) * CMAP_SIZE / log1p(del);
		else
			i = (val - min) * CMAP_SIZE /del;

		idx = (int) floor(i);

		if (idx == CMAP_SIZE)
			idx--;

		assert(idx >=0 && idx < CMAP_SIZE);
	}

        return m_cmap[idx];
}
//---------------------------------------------------------------------------
void
MeshRender::colormapDefault(void)
{
	double val, r, g, b;
	for (int n=0; n<CMAP_SIZE; n++) {
		val=(double)n /(CMAP_SIZE-1);
		r=b=0;
		if (val<0.5) b=1-(2*val);
		if (val>0.5) r=(2*val)-1;

		g = 0.5 * (1 - (b+r));

		if (b < g)
			b = g;
		if (r < g)
			r = g;

		m_cmap[n]=GColor(r,g,b, m_alpha);
	}
	updateFields();
}
//---------------------------------------------------------------------------
void
MeshRender::colormapGray(int nq)
{
	if (nq < 2)
		nq = CMAP_SIZE;
	double inc = 1 / (double)(nq - 1);
	double val = 0;

	int num = 0;
	
	for (int n=0; n<CMAP_SIZE; n++) {
		num += nq;
		if (num >= CMAP_SIZE) {
			val += inc;
			num -= CMAP_SIZE;
		}
		m_cmap[n]=GColor(val,val,val, m_alpha);
	}
	updateFields();
}
//---------------------------------------------------------------------------
void
MeshRender::colormapFlip(void)
{
	int first = 0;
	int last = CMAP_SIZE - 1;
	while (first < last) {
		GColor tmp = m_cmap[first];
		m_cmap[first] = m_cmap[last];
		m_cmap[last] = tmp;
		first++;
		last--;
	}
	updateFields();
}
//---------------------------------------------------------------------------
void
MeshRender::setEField(double *f, int update)
{
	int ne = m_mesh->getNumTris();
	if (ne < 1)
		return;

	m_eprops.resize(ne);

	if (f) {
		for (int n = 0; n < ne; n++)
			m_eprops[n].value = f[n];
	}

	if (update)
		setEFRangeAuto(update < 0);
	else
		updateEField();
}
//---------------------------------------------------------------------------
void
MeshRender::setEBackground(double *f)
{
	double bmin, bmax;
	int ne = m_mesh->getNumTris();
	if (ne < 1)
		return;

	m_eprops.resize(ne);

	if (f) {
		bmin = bmax = f[0];
		for (int n = 1; n < ne; n++) {
			if (f[n] < bmin)
				bmin = f[n];
			if (f[n] > bmax)
				bmax = f[n];
		}
		if (bmin == bmax)
			bmax = bmin + 1;

		for (int n = 0; n < ne; n++) {
			double val = (f[n] - bmin) / (bmax - bmin);
			m_eprops[n].background = val;
		}
	}

	updateEField();
}
//---------------------------------------------------------------------------
void
MeshRender::updateEField(void)
{
	int ne = m_mesh->getNumTris();
	for (int n = 0; n < ne; n++) {
		if (m_flags & MRF_SHOW_NBGRND)
			setEColor(n, m_eprops[n].value, m_eprops[n].background, m_balpha);
		else
			setEColor(n, m_eprops[n].value);
	}
}
//---------------------------------------------------------------------------
void
MeshRender::setEFRangeAuto(bool zero)
{
	int ne = m_mesh->getNumTris();

        m_efldmax = m_efldmin = m_eprops[0].value;

        for (int n = 1; n < ne; n++) {
                double v = m_eprops[n].value;
                if (v < m_efldmin)
			m_efldmin = v;
                else if (v > m_efldmax)
			m_efldmax = v;
        }

	if (zero) {
		double v1 = fabs(m_efldmin);
		double v2 = fabs(m_efldmax);
		if (v1 > v2)
			v2 = v1;
		m_efldmin = -v2;
		m_efldmax = v2;
	}

	updateEField();
}
//---------------------------------------------------------------------------
void
MeshRender::setNField(double *f, int update)
{
	int nn = m_mesh->getNumVerts();
	if (nn < 1)
		return;

	m_nprops.resize(nn);

	if (f) {
		for (int n = 0; n < nn; n++)
			m_nprops[n].value = f[n];
	}
    
	if (update)
		setNFRangeAuto(update < 0);
	else
		updateNField();
}
//---------------------------------------------------------------------------
void
MeshRender::setNBackground(double *f)
{
	double bmin, bmax;
	int nn = m_mesh->getNumVerts();
	if (nn < 1)
		return;

	m_nprops.resize(nn);

	if (f) {
		bmin = bmax = f[0];
		for (int n = 1; n < nn; n++) {
			if (f[n] < bmin)
				bmin = f[n];
			if (f[n] > bmax)
				bmax = f[n];
		}
		if (bmin == bmax)
			bmax = bmin + 1;

		for (int n = 0; n < nn; n++) {
			double val = (f[n] - bmin) / (bmax - bmin);
			m_nprops[n].background = val;
		}
	}

	updateNField();
}
//---------------------------------------------------------------------------
void
MeshRender::setNFRangeAuto(bool zero)
{
	int nn = m_mesh->getNumVerts();
	m_nfldmax = m_nfldmin = m_nprops[0].value;

	for (int n = 1; n < nn; n++) {
		double v = m_nprops[n].value;
		if (v < m_nfldmin)
			m_nfldmin = v;
		else if (v > m_nfldmax)
			m_nfldmax = v;
	}

	if (zero) {
		double v1 = fabs(m_nfldmin);
		double v2 = fabs(m_nfldmax);
		if (v1 > v2)
			v2 = v1;
		m_nfldmin = -v2;
		m_nfldmax = v2;
	}

	updateNField();
}
//---------------------------------------------------------------------------
void
MeshRender::updateNField(void)
{
	int nn = m_mesh->getNumVerts();
	for (int n = 0; n < nn; n++) {
		if (m_flags & MRF_SHOW_NBGRND)
			setNColor(n, m_nprops[n].value, m_nprops[n].background, m_balpha);
		else
			setNColor(n, m_nprops[n].value);
	}
}
//---------------------------------------------------------------------------
void
MeshRender::updateAlpha(void)
{
	int nn = m_mesh->getNumVerts();
	int ne = m_mesh->getNumTris();

	if (m_flags & MRF_SHOW_NCOLOR)
		for (int n = 0; n < nn; n++)
			m_nprops[n].color.A() = m_alpha;

	if (m_flags & MRF_SHOW_ECOLOR)
		for (int n = 0; n < ne; n++)
			m_eprops[n].color.A() = m_alpha;

	for (int n = 0; n < CMAP_SIZE; n++)
		m_cmap[n].A() = m_alpha;
}
//---------------------------------------------------------------------------
void
MeshRender::updateClip(void)
{
	int nn = m_mesh->getNumTris();
	for (int n = 0; n < nn; n++) {
		Point3 p;
		for (int m = 0; m < 3; m++)
			p += m_mesh->getElemVert(n, m);
		p /= 3;

		if (p.getX() < m_clipX0 || p.getX() > m_clipX1 ||
		    p.getY() < m_clipY0 || p.getY() > m_clipY1 ||
		    p.getZ() < m_clipZ0 || p.getZ() > m_clipZ1)
			m_eprops[n].flags |= PRF_CLIP;
		else
			m_eprops[n].flags &= ~PRF_CLIP;
	}
}
//---------------------------------------------------------------------------
