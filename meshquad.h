/* $Id: meshquad.h,v 1.1.1.1 2007/12/30 04:44:24 canacar Exp $ */
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

#ifndef _MESH_H_
#define _MESH_H_
#include <assert.h>
#include <stdio.h>
#include "meshbase.h"
#include "bemmesh.h"

class TriMeshQuad {
 public:
	TriMeshQuad();
	virtual ~TriMeshQuad();

	virtual int loadMesh(char *name) = 0;
	virtual int save(char *name) = 0;
	virtual int saveClass(char *name, int cls);

	virtual void smooth(int n) = 0;
	virtual void render(int cls, bool interp, int quad) = 0;

	virtual int getNumClasses(void) const;
	virtual int getNumTris(int cls) const;
	
	inline int getNumTris(void) const
		{ return m_numtris; }
	inline double getWidth(void) const
		{ return m_size.getX(); }
	inline double getHeight(void) const
		{ return m_size.getY(); }
	inline double getDepth(void) const
		{ return m_size.getZ(); }
	inline void setScale(double s)
		{ m_scale = s; }
	inline double getScale(void) const
		{ return m_scale; }
	
	virtual void moveMesh(double dx, double dy, double dz);
	virtual void scaleMesh(double sx, double sy, double sz);
	
	virtual void getMean(double &dx, double &dy, double &dz) {
		m_mean.getCoord(dx, dy, dz);
	}

 protected:
	virtual void calcLimits(void) = 0;
	virtual void calcFaceNorm(void) = 0;
	virtual void calcVertNorm(void) = 0;

	BEMesh *m_bemesh;
	Point3 m_mean, m_size;
	unsigned m_numtris, m_numverts, m_numedges, m_nodeelem;
	double m_scale;   // how to scale the heightmap data
};

#endif
