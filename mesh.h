/* $Id: mesh.h,v 1.16 2008/04/14 16:11:39 canacar Exp $ */
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
#include <list>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include "point3.h"
#include "meshbase.h"

using namespace std;

class TriMeshLin {
 public:
	TriMeshLin();
	virtual ~TriMeshLin();

	int loadMesh(const char *name);
	int save(const char *name);
	int saveClass(const char *name, int cls);
	int saveSelected(const char *name, const double *sel);

	void smooth(int n);

	int correctMesh(void);
	int removeBadAspectElements(double atresh);
	int removeSmallElements(double size);
	int removeSmallEdges(double size);
	int flipElements(void);

	void moveMesh(double dx, double dy, double dz);
	void scaleMesh(double sx, double sy, double sz);

	inline const Point3 &getVertex(int idx) const
		{ return m_verts[idx]; }

	inline const Point3 &getVertexNormal(int idx)
	{ if (!m_norms_valid) calcNormals(); return m_norms[idx]; }

	inline const Point3 &getFaceNormal(int idx)
	{ if (!m_fnorms_valid) calcFaceNorm(); return m_fnorms[idx]; }

	inline Point3 &getVertex(int idx)
		{ return (m_verts[idx]); }

	inline Neighbor &getNodeNbrs(int node)
		{ return m_nnode[node]; }

	inline Neighbor &getFaceNbrs(int node)
		{ return m_nface[node]; }

	inline const Point3 *getVerts(void) const
		{ return &(m_verts[0]); }
	inline const unsigned *getTriIndex(void) const
		{ return &(m_tris[0]); }
	inline const Point3 *getVertNorms(void) const
		{ return &(m_norms[0]); }

	inline void setElemInd(unsigned e, int idx, unsigned val){
		assert(e<m_numtris);
		assert(idx>=0 && idx<3);
		assert(val<m_numverts);
		m_tris[3*e+idx]=val;
	}
	
	inline unsigned getElemInd(unsigned e, int idx) const{
		assert(e < m_numtris);
		assert(idx >= 0 && idx < 3);
		return m_tris[3 * e + idx];
	}

	inline Point3 &getElemVert(unsigned e, int idx) {
		assert(e < m_numtris);
		assert(idx >= 0 && idx < 3);
		return m_verts[m_tris[3 * e + idx]];
	}

	inline const Point3 &getElemVert(unsigned e, int idx) const {
		assert(e < m_numtris);
		assert(idx >= 0 && idx < 3);
		return m_verts[m_tris[3 * e + idx]];
	}

	inline int getNumEdges(void) const
		{ return m_numedges; }

	inline int getNumTris(void) const
		{ return m_numtris; }

	inline int getNumVerts(void) const
		{ return m_numverts; }

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

	const Point3& getMean(void) const {
		return m_mean;
	}

	const Point3& getSize(void) const {
		return m_size;
	}

	const Point3 getMid(void) const {
		return m_min + (m_size / 2);
	}

	const Point3 getMin(void) const {
		return m_min;
	}

	int getNumClasses(void) const;
	int getNumTris(int cls) const;
	
	int isElemNeighbor(int e1, int e2);

	void calcFaceNorm(void);
	void calcNormals(void);

	TriMeshLin &set(const TriMeshLin &m);

	inline TriMeshLin &operator=(const TriMeshLin &m) {
		return set(m);
	}

	void capacity(unsigned int ne, unsigned int nv);
	unsigned int addVertex(const Point3 &v);
	int moveVertex(int i, const Point3 &v);
	unsigned int addElem(unsigned int i, unsigned int j, unsigned int k);
	void changeElem(unsigned int e, unsigned int i,
			unsigned int j, unsigned int k);
	Edge *getEdge(unsigned n1, unsigned n2);

	void replaceElements(const vector<unsigned> &faces);

	int fillHoles(void);
	void delElem(unsigned int e);
	void recalculateEdges(void) {
		processVertices();
		clearEdges();
		findEdges();
		calcNeighbors();
		classifyFaces();
	}

	int printIntersectionBoundaries(void);

	typedef list<Edge *> elist_t;

	
 protected:
	const Point3 &updateFaceNormal(unsigned int f);

	float calcPhi(int i, int j);
	void delxyz(int nv, Point3 &del);
	
	void calcNeighbors(void);
	void checkNeighbors(void);

	virtual void calcLimits(void);
	
	int findEdges(void);
	void clearEdges(void);
	Edge *addEdge(unsigned n1, unsigned n2, unsigned el, unsigned s = 0);
	void delEdge(unsigned n1, unsigned n2);
	
	char *getLine(FILE *f);
	int loadSmf(FILE *f);
	int loadTri(FILE *f);
	int loadFS(FILE *f);

	int saveColorInfo(FILE *f, int numv, double r, double g, double b);
	
	int processFaces(void);
	int processEdges(void);
	int processVertices(void);
	int correctEdges(void);
	int classifyFaces(void);
	void classifyFace(int *fclass, int face, int cls);
	void sortFaces(int *fclass, int cls, int start);
	
	void addBadVert(unsigned *bv, int &nv, unsigned node);
	int processBadVertex(unsigned vert);
	int isSingularNode(unsigned nd);
	
	int orientElement(char *fflags, int elem);
	int orientNeighbors(char *fflags, int elem);
	int checkOrientation(void);
	
	void delVertex(unsigned int v2);
	void unlinkVertex(unsigned int v);
	int collapseEdge(unsigned int v1, unsigned int v2)
		{ return collapseEdge(getEdge(v1, v2));}
	int collapseEdge(Edge *e);
	void collapseElement(unsigned int elem);
	int delEdgeElem(unsigned int v1, unsigned int v2, unsigned int elem);
	int flipEdge(Edge *e);
	int checkFlipEdge(Edge *e);
	double nodeAngle(unsigned int n1, unsigned int n2, unsigned int n3);

	void setElem(unsigned int e,
		     unsigned int i, unsigned int j, unsigned int k);

	inline void invalidateNormals(void) {
		m_fnorms_valid = false;
		m_norms_valid = false;
	}
	inline void invalidateVertexNormals(void) {
		m_norms_valid = false;
	}

	int generateBoundary(elist_t &elist, elist_t &blist);
	int isNeighborEdges(Edge *e1, Edge *e2);
	int isStitchable(Edge *e1, Edge *e2);

	int fillNextPair(elist_t &blist);
	int stitchNextPair(elist_t &blist, int pv);
	int stitch(void);

	void print_boundary(const char *hdr, const TriMeshLin::elist_t &blist);


	Point3 m_mean, m_size, m_min;
	double m_dx, m_dy, m_dz;
	unsigned m_numtris, m_numverts, m_numedges, m_nodeelem;
	double m_scale;   // how to scale the heightmap data

	vector<Point3> m_verts; // vertices
	vector<Point3> m_norms; // vertex normals
	vector<unsigned> m_tris;  // face indices (3/face)
	vector<unsigned> m_fsizes; // class face sizes

	vector<unsigned> m_vorigin; // origin vector used by cutting/stitching

	vector<Point3> m_fnorms;
	vector<Neighbor> m_nnode;  // neighboring nodes
	vector<Neighbor> m_nface;  // neighboring faces
	vector<Edge *> m_edges;	   // edge information

	vector<char> m_nflags;	// node flag used internally
	
	double m_lambda, m_mu;
	bool m_fnorms_valid;
	bool m_norms_valid;

	friend class EdgeIter;
};

class EdgeIter {
 public:
	EdgeIter(TriMeshLin &msh);
	Edge *value(void);
	EdgeIter &next(void);
 private:
	TriMeshLin *m_mesh;
	unsigned m_nd;
	Edge *m_edge;
};

#endif

