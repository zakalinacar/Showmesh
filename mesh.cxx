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

#include <stdint.h>
#include <limits.h>
#include <math.h>
#include "mesh.h"
#include <string.h>
//---------------------------------------------------------------------------
TriMeshLin::TriMeshLin(void)
{
	m_numverts = m_numtris = m_numedges = m_nodeelem = 0;

//	float kPB = 0.1;
	float kPB = 0.5;

//	m_lambda = 0.6307;
	m_lambda = 0.6307;
	m_mu = 1 / (kPB - 1 / m_lambda);
	invalidateNormals();
}
//---------------------------------------------------------------------------
TriMeshLin::~TriMeshLin()
{
	for (unsigned n = 0; n < m_edges.size(); n++) {
		while (m_edges[n]) {
			Edge *e=m_edges[n];
			m_edges[n]=e->next;
			delete e;
		}
	}
}
//---------------------------------------------------------------------------
int
TriMeshLin::getNumClasses(void) const
{
	return m_fsizes.size();
}
//---------------------------------------------------------------------------
int
TriMeshLin::getNumTris(int cls) const
{
	if (cls < 0 || cls >= getNumClasses())
		return 0;
	return m_fsizes[cls];
}
//---------------------------------------------------------------------------
Edge *
TriMeshLin::getEdge(unsigned n1, unsigned n2)
{
	assert(n1 != n2);
	assert(n1 >= 0 && n1 < m_numverts);
	assert(n2 >= 0 && n2 < m_numverts);

	if (n1 > n2) {
		int tmp = n2;
		n2 = n1;
		n1 = tmp;
	}

	for (Edge *ep = m_edges[n1]; ep; ep=ep->next)
		if (n2 == ep->node2)
			return ep;
	return 0;
}
//---------------------------------------------------------------------------
void
TriMeshLin::delEdge(unsigned n1, unsigned n2)
{
	assert(n1 != n2);
	assert(n1 >= 0 && n1 < m_numverts);
	assert(n2 >= 0 && n2 < m_numverts);

	if(n1 > n2){
		int tmp = n2;
		n2 = n1;
		n1 = tmp;
	}

	Edge *ep = m_edges[n1];
	if (ep == NULL)
		return;

	if (ep->node2 == n2) {
		m_edges[n1] = ep->next;
		delete ep;
		m_numedges--;
		return;
	}

	Edge *e;
	while ((e = ep->next) != NULL){
		if (n2 == e->node2) {
			ep->next = e->next;
			delete e;
			m_numedges--;
			return;
		}
		ep = e;
	}
}
//---------------------------------------------------------------------------
Edge *
TriMeshLin::addEdge(unsigned n1, unsigned n2, unsigned el, unsigned s)
{
	if (n1 == n2)
		assert(n1 != n2);

	assert(n1 >= 0 && n1 < m_numverts);
	assert(n2 >= 0 && n2 < m_numverts);

	if (n1 > n2) {
		int tmp = n2;
		n2 = n1;
		n1 = tmp;
	}

	Edge *ep;
	for (ep = m_edges[n1]; ep != NULL; ep = ep->next) {
		if (n2 == ep->node2) {
			assert(n1 == ep->node1);
			for (int n = 0; n < ep->nelem; n++)
				if (ep->elem[n] == el)
					return ep;

			if (ep->nelem == MAX_EDGE_ELEM){
				/* XXX: Edge neighbor list overflow! */
				assert(0);
				return NULL;
			}
			ep->elem[ep->nelem++] = el;
			return ep;
		}
	}

	ep = new Edge(n1,n2);
	ep->elem[0] = el;
	ep->nelem = 1;
	ep->next = m_edges[n1];
	ep->store = s;
	m_edges[n1] = ep;
	m_numedges++;

	return ep;
}
//---------------------------------------------------------------------------
int
TriMeshLin::findEdges(void)
{
	if (m_numedges)
		return 1;

	MESH_LOG("Calculating edges\n");
	m_edges.resize(m_numverts);

	for(unsigned n = 0; n < m_numverts; n++)
		m_edges[n] = NULL;

	for(unsigned n = 0; n < m_numtris; n++) {
		addEdge(getElemInd(n, 0), getElemInd(n, 1), n);
		addEdge(getElemInd(n, 1), getElemInd(n, 2), n);
		addEdge(getElemInd(n, 2), getElemInd(n, 0), n);
	}

	return 0;
}
//---------------------------------------------------------------------------
TriMeshLin &
TriMeshLin::set(const TriMeshLin &m)
{
	char c;
	double x, y, z;

	m_tris.clear();
	m_verts.clear();
	m_nflags.clear();
	m_norms.clear();

	invalidateNormals();
	m_numverts = m.getNumVerts();
	m_numtris = m.getNumTris();

	m_verts.resize(m_numverts);
	m_tris.resize(m_numtris * 3);

	for (unsigned int v = 0; v < m_numverts; v++)
		getVertex(v) = m.getVertex(v);

	for (unsigned int t = 0; t < m_numtris; t++) {
		for (int i = 0; i < 3; i++)
			setElemInd(t, i, m.getElemInd(t, i));
	}

	m_nflags.resize(m_numverts);
	m_norms.resize(m_numverts);

	calcLimits();

	calcNeighbors();
	findEdges();

        calcNormals();

	return (*this);
}
//---------------------------------------------------------------------------
#define MAX_LINE 1024
char *
TriMeshLin::getLine(FILE *f)
{
	static char buf[MAX_LINE];
	if (f == NULL)
		return 0;
	for(;;) {
		char *s = fgets(buf, MAX_LINE, f);
		if (s == NULL)
			return NULL;
		if (*s != '\n' && *s != '\r' && *s != '#')
			break;
	}
	return buf;
}
//---------------------------------------------------------------------------
int
TriMeshLin::loadSmf(FILE *f)
{
	char c;
	double x, y, z;

	m_tris.clear();
	m_verts.clear();
	m_nflags.clear();
	m_norms.clear();

	invalidateNormals();

	unsigned max = 0;
	for (;;) {
		char *s = getLine(f);
		if (s == NULL)
			break;
		if (s[0] == 'v') {
			if (sscanf(s, "%c %lf %lf %lf", &c, &x, &y, &z) != 4)
				return 2;
			m_verts.push_back(Point3(x,y,z));
		}
		if (s[0] == 't' || s[0] == 'f') {
			if (sscanf(s, "%c %lf %lf %lf", &c, &x, &y, &z) != 4)
				return 3;
			if (floor(x) != x || x <= 0)
				return 5;
			if (floor(y) != y || y <= 0)
				return 5;
			if (floor(z) != z || z <= 0)
				return 5;
			if (max < x)
				max = (int) floor(x);
			if (max < y)
				max = (int) floor(y);
			if (max < z)
				max = (int) floor(z);

			m_tris.push_back((int)x - 1);
			m_tris.push_back((int)y - 1);
			m_tris.push_back((int)z - 1);
		}
	}

	m_numverts = m_verts.size();
	m_numtris = m_tris.size();

	assert((m_numtris % 3) == 0);
	m_numtris /= 3;

	if (max > m_numverts || max < 1)
		return 1;

	m_nflags.resize(m_numverts);
	m_norms.resize(m_numverts);

	calcLimits();

	return 0;
}
//---------------------------------------------------------------------------
int
TriMeshLin::loadTri(FILE *f)
{
	char *buf;

	int nv, id;
	float x, y, z, nx, ny, nz;
	VertexStore vs;

	unsigned u[3];
	int cnt = 0;

	m_tris.clear();
	m_verts.clear();
	m_nflags.clear();
	m_norms.clear();
	invalidateNormals();

	for (;;) {
		if ((buf = getLine(f)) == 0)
			break;
		if (sscanf(buf, "%d", &nv) != 1)
			return 1;
		if (nv != 3)
			return 1;

		for (int n = 0; n < nv; n++) {
			if ((buf = getLine(f)) == 0)
				break;
			sscanf(buf, "%f %f %f %f %f %f",
			       &x, &y, &z, &nx, &ny, &nz);
			id = vs.addVertex(x, y, z, nx, ny, nz);
			if (id < 0)
				return 1;
			u[n]=id;
			cnt++;
		}
		m_tris.push_back(u[2]);
		m_tris.push_back(u[1]);
		m_tris.push_back(u[0]);
		m_numtris++;
	}

	if (m_numtris == 0)
		return 1;
	if (m_tris.size() != m_numtris * 3)
		return 1;

	m_numverts = vs.numVertices();

	if (m_numverts <= 0)
		return 1;

	m_verts.resize(m_numverts);
	m_nflags.resize(m_numverts);
	m_norms.resize(m_numverts);

	for (unsigned n = 0; n < m_numverts; n++) {
		m_nflags[n] = 0;
		Vertex *v = vs.getVertex(n);
		if (v == NULL)
			return 1;
		if (v->id < 0)
			return 1;
		if ((unsigned)v->id != n)
			return 1;
		m_verts[n] = v->coord;
		m_norms[n] = v->normal * -1 / (float)v->ncount;
	}

	calcLimits();

	return 0;
}
//---------------------------------------------------------------------------
/* Ref: http://www.grahamwideman.com/gw/brain/fs/surfacefileformats.htm */ 
#define FS_TRIANGLE_FILE_MAGIC_NUMBER (-2 & 0x00ffffff)
#define FS_INFO_SIZE 128

int
TriMeshLin::loadFS(FILE *f)
{
	FSFile fs(f);

	uint32_t magic;
	char info[FS_INFO_SIZE];
	int pf = 0;
	uint32_t i, vertex_count, face_count;
	uint32_t face;
	double vertex[3];

	m_tris.clear();
	m_verts.clear();
	m_nflags.clear();
	m_norms.clear();

	invalidateNormals();

	if (fs.readI3(magic))
		return 1;

	if (magic != FS_TRIANGLE_FILE_MAGIC_NUMBER)
		return 1;

	if (fs.readString(info, sizeof(info)))
		return 1;

	if (fs.readI4(vertex_count))
		return 1;

	if (fs.readI4(face_count))
		return 1;
	
	printf("loadFS: %u vertices, %u faces, [%s]\n", vertex_count, face_count, info);

	for (i = 0; i < vertex_count; i++)
	{
		if (fs.readF4(vertex[0]))
			return 1;
		if (fs.readF4(vertex[1]))
			return 1;
		if (fs.readF4(vertex[2]))
			return 1;

		m_verts.push_back(Point3(vertex[0], vertex[1], vertex[2]));
	}

	for (i = 0; i < face_count; i++) {
		for (int j = 0; j < 3; j++) {
			if (fs.readI4(face))
				return 1;

			if (face >= vertex_count)
				return 1;

			m_tris.push_back(face);
		}
	}

	m_numverts = m_verts.size();
	m_numtris = m_tris.size();

	assert((m_numtris % 3) == 0);
	m_numtris /= 3;

	m_nflags.resize(m_numverts);
	m_norms.resize(m_numverts);

	calcLimits();

	return 0;
}
//---------------------------------------------------------------------------
int
TriMeshLin::loadMesh(const char *name)
{
	int ret;
	const char *s;

	if (name == NULL)
		return 1;

	FILE *f = fopen(name, "rb");
	if (f == NULL)
		return 1;

	/* check if filename ends with .smf */
	s = strstr(name, ".smf");
	if (s != NULL && strlen(s) == 4) {
		ret = loadSmf(f);
	} else {
		ret = loadTri(f);
		if (ret) {
			if (fseek(f, 0, SEEK_SET) == 0)
				ret = loadFS(f);
		}
	}

	if (ret)
		return 1;

	MESH_LOG("(uncorrected) mean: %f, %f, %f\n",
	       m_mean.X(),m_mean.Y(),m_mean.Z());

	MESH_LOG("mesh corner: %f, %f, %f\n",
	       m_min.X(),m_min.Y(),m_min.Z());

	MESH_LOG("mesh_size: %f, %f, %f\n",
	       m_size.X(),m_size.Y(),m_size.Z());

	calcNeighbors();
	findEdges();
	classifyFaces();
	checkOrientation();
	calcNormals();

	return (0);
	
}
//---------------------------------------------------------------------------
int
TriMeshLin::correctMesh(void)
{
	calcNeighbors();
	findEdges();

	if (processFaces()){
		MESH_LOG("Bad faces encountered!\n");
		return 1;
	}

	if (processVertices()) {
		MESH_LOG("Bad vertices encountered!\n");
		return 1;
	}

	if (processEdges()) {
		MESH_LOG("Bad edges encountered!\n");
	}

	if (correctEdges()) {
		MESH_LOG("Failed to correct edges!\n");
		return 1;
	}

	MESH_LOG("After Correction:\n");

	if (processFaces()) {
		MESH_LOG("Bad faces encountered!\n");
		return 1;
	}

	if (processVertices()) {
		MESH_LOG("Bad vertices encountered!\n");
		return 1;
	}

	if (processEdges()) {
		MESH_LOG("More bad edges encountered trying once more...!\n");
		if (correctEdges()) {
			MESH_LOG("Failed to correct edges!\n");
			return 1;
		}
	}

	if (processEdges()) {
		MESH_LOG("Still More bad edges!\n");
	}

	classifyFaces();

	checkOrientation();

	MESH_LOG("(after correction) mean: %f, %f, %f\n",
	       m_mean.X(),m_mean.Y(),m_mean.Z());

	MESH_LOG("mesh corner: %f, %f, %f\n",
	       m_min.X(),m_min.Y(),m_min.Z());

	MESH_LOG("mesh_size: %f, %f, %f\n",
	       m_size.X(),m_size.Y(),m_size.Z());

	calcNormals();

	return 0;
}
//---------------------------------------------------------------------------
int
TriMeshLin::isSingularNode(unsigned nd)
{
/*
	int start=m_nface[nd][0];
	if(start<0){
		printf("Standalone node found: %d\n",nd);
		return 0;
	}

	int curr=start;
	int np=nd; // dummy
	int nnext=-1;
	int count=1;
	int next;

	while(1){
		int n1=getElemInd(curr,0);
		int n2=getElemInd(curr,1);
		int n3=getElemInd(curr,2);

		if(start==curr){
			if(n1==nd){
				n1=n2;
				nnext=n3;
			}else if(n2==nd) nnext=n3;
			else nnext=n2;
		}else{
			if(n1==nd){
				if(n2==np) n1=n3;
				else n1=n2;
			}else if(n1==np){
				if(n2==nd) n1=n3;
				else n1=n2;
			}
		}
		// n1 is our next node
		Edge *e=getEdge(nd,n1);
		if(e==0){
			assert(e);
		}
		if(e->nelem>2){
			printf("node adj to singular edge! \n");
			return 1;
		}
		int next;
		if(e->nelem<2){
			assert(e->elem[0]==curr);
			next=-1;
		}else{
			next=e->elem[0];
			if(next==curr) next=e->elem[1];
		}
		if(next==start || next<0) break;
		curr=next;
		np=n1;
		count++;
	}
	if(next<0){ // trace backwards!
		while(1){
			int n1=getElemInd(curr,0);
			int n2=getElemInd(curr,1);
			int n3=getElemInd(curr,2);

			if(start==curr){
				n1==nnext;
			}else{
				if(n1==nd){
					if(n2==np) n1=n3;
					else n1=n2;
				}else if(n1==np){
					if(n2==nd) n1=n3;
					else n1=n2;
				}
			}
			// n1 is our next node
			Edge *e=getEdge(nd,n1);
			assert(e);
			if(e->nelem>2){
				printf("node adj to singular edge! \n");
				return 1;
			}
			int next=e->elem[0];
			if(next==curr) next=e->elem[1];
			if(next==start){
				printf("OOPS! connecttivity problem! \n");
				return 0;
			}
			if(next<0) break;
			np=n1;
			count++;
		}
	}
	int nnbr;
	for(nnbr=0; (m_nface[nd])[nnbr]>=0; nnbr++);
	if(count!=nnbr)
		return 1;
	return 0;
*/

	int nfnbr;
	int fclass[MAX_NEIGHBOR];
	int fnbrs[MAX_NEIGHBOR];

	for (int n = 0; n < MAX_NEIGHBOR; n++)
		fclass[n]=-1;

	for (nfnbr = 0; nfnbr < MAX_NEIGHBOR; nfnbr++)
		if ((fnbrs[nfnbr] = getFaceNbrs(nd)[nfnbr])<0)
			break;

	int cls = 0;
	for (;;) {
		int n;
		for (n = 0; n < nfnbr; n++)
			if(fclass[n]<0)
				break;
		if (n == nfnbr)
			break;

		fclass[n] = cls;

		for (; n < nfnbr; n++){
			if (fclass[n] != cls)
				continue;
			int np = n;
			for (int m = 0; m < 3; m++) {
				unsigned nf = getElemInd(fnbrs[np],m);
				if (nf == nd)
					continue;
				for (int i = 0; i < nfnbr; i++) {
					if (fclass[i] >= 0)
						continue;
					for (int k = 0; k < 3; k++) {
						if (getElemInd(fnbrs[i], k) == nf) {
							fclass[i] = cls;
							if (n>i)
								n = i - 1;
						}
					}
				}
			}
		}
		cls++;
	}

	if (cls > 1)
		return 1;

	return 0;
}
//---------------------------------------------------------------------------
int
TriMeshLin::correctEdges(void)
{
	// origin vector to identify the originating vertex
	m_vorigin.clear();
	m_vorigin.resize(m_numverts, (unsigned int) -1);

	for (unsigned n = 0; n < m_numverts; n++)
		m_nflags[n] = 0;

	// this is Cutting part

	// mark nodes for an edge with more than three face
	for (unsigned n = 0; n < m_numverts; n++) {
		for (Edge *e = m_edges[n]; e; e = e->next) {
			if (e->nelem > 2) {
				m_nflags[e->node1] = 1;
				m_nflags[e->node2] = 1;
			}
		}
	}

	// mark the singular nodes too
	for (unsigned n = 0; n < m_numverts; n++) {
		if (m_nflags[n])
			continue;
		m_nflags[n]=isSingularNode(n);
	}

	for (;;) {
		int found = 0;
		int ok = 0;

		// loop until no marked nodes left, or none could be fixed
		for(unsigned n = 0; n < m_numverts; n++) {
			if (m_nflags[n] == 0)
				continue;
			found=1;
			if (processBadVertex(n))
				continue;
			ok = 1;
		}

		if (!found)
			break;

		if (!ok)
			return 1;
	}

	// and now Stitching ...

//	return stitch();
	return 0;
}


// starting with the first edge, add edges to blist from elist that
// are connected to form a boundary,
int
TriMeshLin::generateBoundary(elist_t &elist, elist_t &blist)
{
	elist_t::iterator ei, bi, bn, en;

	Edge *e = elist.front();
	elist.pop_front();

	blist.clear();
	blist.push_front(e);

	unsigned int n1 = e->node1;
	unsigned int n2 = e->node2;

	for (bi = blist.begin(); bi != blist.end(); bi = bn) {
		bn = bi;
		bn++;
		Edge *b = *bi;

		for (ei = elist.begin(); ei != elist.end(); ei = en) {
			en = ei;
			en++;
			Edge *e = *ei;
			if (b->node1 == e->node1 || b->node1 == e->node2 ||
			    b->node2 == e->node2 || b->node2 == e->node1) {
				blist.insert(bn, e);
				bn = bi;
				bn++;
				elist.remove(e);

				if (e->node1 == n1)
					n1 = e->node2;
				else if (e->node2 == n1)
					n1 = e->node1;
				else if (e->node1 == n2)
					n2 = e->node2;
				else if (e->node2 == n2)
					n2 = e->node1;

				if (n1 == n2)
					return (0);
			}
		}
	}

	if (n1 != n2) {
		printf("Incomplete boundary! ");
		Edge *ep = getEdge(n1, n2);
		if (ep == NULL) {
			printf("No edge for [%d %d]\n", n1, n2);
			return (1);
		}
		printf("Edge [%d %d] completes the boundary\n", n1, n2);
	}

	return (0);
}


int
TriMeshLin::isStitchable(Edge *e1, Edge *e2)
{
	unsigned int e11 = e1->node1;
	unsigned int e12 = e1->node2;
	unsigned int e21 = e2->node1;
	unsigned int e22 = e2->node2;

	unsigned int o11 = m_vorigin[e11];
	unsigned int o12 = m_vorigin[e12];
	unsigned int o21 = m_vorigin[e21];
	unsigned int o22 = m_vorigin[e22];

	if (o12 != -1) {
		if (e11 == e21 && o12 == o22)
			return 1;
		if (e11 == e22 && o12 == o21)
			return 1;
	}
	if (o11 != -1) {
		if (o11 == o21 && e12 == e22)
			return 1;
		if (o11 == o22 && e12 == e21)
			return 1;
	}

	return 0;
}

int
TriMeshLin::isNeighborEdges(Edge *e1, Edge *e2)
{
	if (e1->node1 == e2->node1 || e1->node1 == e2->node2 ||
	    e1->node2 == e2->node1 || e1->node2 == e2->node2) 
		return 1;
	return 0;
}

void
TriMeshLin::print_boundary(const char *hdr, const TriMeshLin::elist_t &blist)
{
	TriMeshLin::elist_t::const_iterator e;

	printf("%s: boundary, %lu edges:\n", hdr, blist.size());
	for (e = blist.begin(); e != blist.end(); e++) {
		Edge *ed = *e;
		printf("  Edge: [%d %d], o: [%d %d], nelem: %d, elem0: %d, len: %g\n",
		       ed->node1, ed->node2,
//		       m_vorigin[ed->node1], m_vorigin[ed->node2],
		       -1,-1,
		       ed->nelem, ed->elem[0],
		       (getVertex(ed->node1) - getVertex(ed->node2)).length());
	}
}

// picks the next pair to stitch from blist, if possible using the
// last stitched vertex, pv
int
TriMeshLin::stitchNextPair(elist_t &blist, int pv)
{
	elist_t::iterator e1, e2, en;

	Edge *ed1, *ed2;

	ed1 = ed2 = NULL;

	for (e1 = blist.begin(); e1 != blist.end(); e1 = en) {
		en = e1;
		en++;
		for (e2 = en; e2 != blist.end(); e2++) {
			if (!isStitchable(*e1, *e2))
				continue;
			ed1 = *e1;
			ed2 = *e2;
			if (ed1->node1 == pv || ed1->node2 == pv)
				goto found;
		}
	}

	if (ed1 == NULL)
		return -1;

 found:
	unsigned int v0, v1, v2;

	if (ed1->node1 == ed2->node1) {
		v0 = ed1->node1;
		v1 = ed1->node2;
		v2 = ed2->node2;
	} else if (ed1->node1 == ed2->node2) {
		v0 = ed1->node1;
		v1 = ed1->node2;
		v2 = ed2->node1;
	} else if (ed1->node2 == ed2->node1) {
		v0 = ed1->node2;
		v1 = ed1->node1;
		v2 = ed2->node2;
	} else if (ed1->node2 == ed2->node2) {
		v0 = ed1->node2;
		v1 = ed1->node1;
		v2 = ed2->node1;
	} else {
		// should not happen
		printf("Selected edges do not match!\n");
		assert(0);
	}

	if (v2 < v1) {
		unsigned t = v1;
		v1 = v2;
		v2 = t;
	}

	// remove the two edges from the list
	blist.remove(ed1);
	blist.remove(ed2);

	printf("Stitching edges (%d %d) (%d %d) collapsing vertex %d to %d\n",
	       ed1->node1, ed1->node2, ed2->node1, ed2->node2, v2, v1);

	if (v1 == v2) {
		printf("Edges Already stitched.\n");
		return (-1);
	}

	// first change the edges (edge list will have to be recomputed)
	for (e1 = blist.begin(); e1 != blist.end(); e1++) {
		Edge *e = (*e1);
		if (e->node1 == v2)
			e->node1 = v1;
		else if (e->node2 == v2)
			e->node2 = v1;
	}

	print_boundary(" Modified", blist);

	Neighbor &nf1 = getFaceNbrs(v1);
	Neighbor &nf2 = getFaceNbrs(v2);

	for (int n = 0; n < nf2.count(); n++) {
		unsigned int el = nf2[n];
		nf1.add(el);
		printf("Updating element: %d (%d %d %d) ->",
		       el, getElemInd(el, 0),
		       getElemInd(el, 1), getElemInd(el, 2));
		for (int m = 0; m < 3; m++) {
			if (getElemInd(el, m) == v2) {
				setElemInd(el, m, v1);
				break;
			}
		}
		printf(" (%d %d %d)\n", getElemInd(el, 0),
		       getElemInd(el, 1), getElemInd(el, 2));
	}

	return v0;
}

int
TriMeshLin::stitch(void)
{
	int stitched = 0;
	elist_t elist, blist;

	for (EdgeIter it(*this); it.value(); it.next()) {
		Edge *ed = it.value();
		if (ed->nelem != 1)
			continue;
		elist.push_back(ed);
	}

	elist_t::iterator bi, ei;
	while (!elist.empty()) {
		printf("\nGenerating boundary:\n");
		generateBoundary(elist, blist);
		print_boundary("Stitching", blist);

		while (!blist.empty()) {
			int pv = -1;
			pv = stitchNextPair(blist, pv);
			if (pv == -1)
				break;
			stitched++;
		}
	}

	if (stitched) {
		clearEdges();
		// and reclaculate them
		calcNeighbors();
		findEdges();
		processVertices();
	}

	return 0;
}
//---------------------------------------------------------------------------
int
TriMeshLin::fillNextPair(elist_t &blist)
{
	elist_t::iterator e1, e2, en;

	Edge *ed1, *ed2, *ed3;

	ed1 = ed2 = ed3 = NULL;

	for (e1 = blist.begin(); e1 != blist.end(); e1 = en) {
		en = e1;
		en++;
		for (e2 = en; e2 != blist.end(); e2++) {
			if (!isNeighborEdges(*e1, *e2))
				continue;
			ed1 = *e1;
			ed2 = *e2;
			break;
		}
		if (e2 != blist.end())
			break;
	}

	if (ed1 == NULL)
		return -1;

	unsigned int v0, v1, v2;

	if (ed1->node1 == ed2->node1) {
		v0 = ed1->node1;
		v1 = ed1->node2;
		v2 = ed2->node2;
	} else if (ed1->node1 == ed2->node2) {
		v0 = ed1->node1;
		v1 = ed1->node2;
		v2 = ed2->node1;
	} else if (ed1->node2 == ed2->node1) {
		v0 = ed1->node2;
		v1 = ed1->node1;
		v2 = ed2->node2;
	} else if (ed1->node2 == ed2->node2) {
		v0 = ed1->node2;
		v1 = ed1->node1;
		v2 = ed2->node1;
	} else {
		// should not happen
		printf("Selected edges do not match!\n");
		assert(0);
	}

	// try to maintain a good orientation
	unsigned int el = ed1->elem[0];
	int m;
	for (m = 0; m < 3; m++) {
		if (getElemInd(el, m) == v0)
			break;
	}
	assert(m < 3);
	m++;
	if (m > 2)
		m = 0;

	if (getElemInd(el, m) == v1) {
		unsigned t = v1;
		v1 = v2;
		v2 = t;
	}

	printf("  Adding element %d %d %d\n", v0, v1, v2);

	addElem(v0, v1, v2);

	ed3 = getEdge(v1, v2);
	assert(ed3);

	if (ed3->nelem == 1) {
		printf("  Adding edge [%d %d] to boundary\n", v1, v2);
		blist.push_front(ed3);
	} else {
		printf("  Removing edge [%d %d] from boundary\n", v1, v2);
		blist.remove(ed3);
	}

	// remove the three edges from the list
	assert(ed1->nelem == 2);
	assert(ed2->nelem == 2);

	blist.remove(ed1);
	blist.remove(ed2);

	return 0;
}
//---------------------------------------------------------------------------
int
TriMeshLin::fillHoles(void)
{
	int filled = 0;
	elist_t elist, blist;

	for (EdgeIter it(*this); it.value(); it.next()) {
		Edge *ed = it.value();
		if (ed->nelem != 1)
			continue;
		elist.push_back(ed);
	}

	elist_t::iterator bi, ei;

	print_boundary("edges:", elist);

	while (!elist.empty()) {
		generateBoundary(elist, blist);
		print_boundary("\n Filling", blist);

		while (!blist.empty()) {
			if (fillNextPair(blist))
				break;
			filled++;
		}
	}

	if (filled) {
		invalidateNormals();
		clearEdges();
		// and reclaculate them
		calcNeighbors();
		findEdges();
		processVertices();
	}

	return 0;
}
//---------------------------------------------------------------------------
int
TriMeshLin::printIntersectionBoundaries(void)
{
	int filled = 0;
	elist_t elist, blist;

	for (EdgeIter it(*this); it.value(); it.next()) {
		Edge *ed = it.value();
		if (ed->nelem == 2)
			continue;
		elist.push_back(ed);
	}

	elist_t::iterator bi, ei;
	while (!elist.empty()) {
		printf("\nGenerating boundary:\n");
		generateBoundary(elist, blist);
		print_boundary("\n Boundary", blist);
	}

	return 0;
}
//---------------------------------------------------------------------------
int
TriMeshLin::processBadVertex(unsigned vert)
{
	int numbad = 1;

	unsigned bads[MAX_EDGE_ELEM];

	bads[0] = vert;

	for (int n = 0; n < MAX_NEIGHBOR; n++) {
		int nt = getNodeNbrs(vert)[n];
		if (nt<0)
			break;
		Edge *e = getEdge(vert,nt);
		assert(e);

		if (e->nelem>2) {
			assert(numbad < MAX_EDGE_ELEM);
			bads[numbad++] = nt;
		}
	}

	int nfnbr;
	int fclass[MAX_NEIGHBOR];
	int fnbrs[MAX_NEIGHBOR];

	for (int n = 0; n < MAX_NEIGHBOR; n++)
		fclass[n] = -1;

	for (nfnbr = 0; nfnbr < MAX_NEIGHBOR; nfnbr++)
		if ((fnbrs[nfnbr] = getFaceNbrs(vert)[nfnbr]) < 0)
			break;

	int cls = 0;

	for (;;) {
		int n;
		for (n = 0; n < nfnbr; n++)
			if (fclass[n] < 0)
				break;
		if (n == nfnbr)
			break;

		fclass[n] = cls;

		for (; n < nfnbr; n++) {
			if (fclass[n] != cls)
				continue;
			int np = n;

			for (int m = 0; m < 3; m++) {
				unsigned nf = getElemInd(fnbrs[np], m);
//				if(nf==vert || nf==nbad)
//					continue;
				int skip = 0;
				for (int t = 0; t < numbad; t++) {
					if (bads[t] == nf) {
						skip = 1;
						break;
					}
				}
				if (skip)
					continue;
				for (int i = 0; i < nfnbr; i++) {
					if (fclass[i] >= 0)
						continue;
					for (int k = 0; k < 3; k++) {
						if (getElemInd(fnbrs[i], k) == nf) {
							fclass[i] = cls;
							if (n > i)
								n = i - 1;
						}
					}
				}
			}
		}
		cls++;
	}

	// we now have cls classes (add cls-1 vertices)

	if (cls == 1) {
		m_nflags[vert] = 0;
		return 0; // no change
	}
	assert(cls > 1);

	// remove vert and faces from neighboring nodes
	// remove edges incident to vert

	for (int n = 0; n < MAX_NEIGHBOR; n++) {
		int ntmp = getNodeNbrs(vert)[n];
		if (ntmp < 0)
			break;
		getNodeNbrs(ntmp).del(vert);
		delEdge(vert, ntmp);

		for (int m = 0; m < nfnbr; m++)
			getFaceNbrs(ntmp).del(fnbrs[m]);
		getNodeNbrs(vert)[n] = -1;
	}

	int nbase = m_numverts-1;

	m_numverts = nbase + cls;
	getNodeNbrs(vert).clear();
	getNodeNbrs(vert).clear();

	// duplicate vertex
	m_verts.resize(m_numverts, m_verts[vert]);
	m_norms.resize(m_numverts, m_norms[vert]);
	m_nflags.resize(m_numverts);
	m_edges.resize(m_numverts, 0);
	m_nnode.resize(m_numverts); // clear
	m_nface.resize(m_numverts); // clear

	m_nflags[vert] = 0;

	for (unsigned n = nbase; n < m_numverts; n++)
		m_nflags[n] = 0;

	unsigned origin = vert;
	while (m_vorigin[origin] != (unsigned int) -1)
		origin = m_vorigin[origin];

	printf("Splitting vertex %d, origin: %d\n", vert, origin);
	m_vorigin.resize(m_numverts, origin);

	// set new indices
	for (int i = nbase + 1; i < m_numverts; i++)
		printf("  added vertex %d (%d)\n", i, m_vorigin[i]);

	for (int n = 0; n < nfnbr; n++) {
		if (fclass[n] == 0)
			continue;
		unsigned el = fnbrs[n];
		unsigned vnew = nbase + fclass[n];
		printf("Updating element: %d (%d %d %d) ->",
		       el, getElemInd(el, 0),
		       getElemInd(el, 1), getElemInd(el, 2));
		for (int m = 0; m < 3; m++) {
			if (getElemInd(el, m) == vert) {
				setElemInd(el, m, vnew);
				break;
			}
		}
		printf(" (%d %d %d)\n", getElemInd(el, 0),
		       getElemInd(el, 1), getElemInd(el, 2));
	}

	// set edge and neighborhood info
	for (int n = 0; n < nfnbr; n++) {
		unsigned el = fnbrs[n];
		unsigned vnew, v0, v1, v2;
		Edge *ep;

		vnew = (fclass[n] == 0) ? vert : nbase + fclass[n];
		v0 = getElemInd(el, 0);
		v1 = getElemInd(el, 1);
		v2 = getElemInd(el, 2);

		printf("Adding edges, element: %d (%d, %d, %d), vnew: %d\n",
		       el, v0, v1, v2, vnew);

		ep = addEdge(v0, v1, el);
		ep = addEdge(v1, v2, el);
		ep = addEdge(v2, v0, el);

		for (int m = 0; m < 3; m++) {
			unsigned n1 = getElemInd(el, m);
			getFaceNbrs(n1).add(el);
			for (int k = 0; k < 3; k++) {
				if (k == m)
					continue;
				unsigned n2 = getElemInd(el, k);
				getNodeNbrs(n1).add(n2);
			}
		}
	}

	invalidateNormals();

	// phew!
	return 0;
}
//---------------------------------------------------------------------------
int
TriMeshLin::processFaces(void)
{
	int bad=0;

	for (unsigned n = 0; n < m_numtris; n++) {
		int n1 = getElemInd(n,0);
		int n2 = getElemInd(n,1);
		int n3 = getElemInd(n,2);
		if (n1 == n2 || n1 == n3 || n2 == n3)
			bad++;
	}

	MESH_LOG("%d faces, %d bad\n", m_numtris, bad);
	return bad;
}
//---------------------------------------------------------------------------
/*
 * Checks for edges with > 2 elements
 */
int
TriMeshLin::processEdges(void)
{
	int nmax = 0;
	int nmin = MAX_EDGE_ELEM;
	int bad = 0;

	for (unsigned n = 0; n<m_numverts; n++) {
		for (Edge *e = m_edges[n]; e; e = e->next) {
			if (nmin > e->nelem)
				nmin = e->nelem;
			if (nmax < e->nelem)
				nmax=e->nelem;
			if (e->nelem > 2)
				bad++;
		}
	}

	MESH_LOG("%d edges, %d bad - min %d, max %d neighbors\n",
	       m_numedges, bad, nmin, nmax);

	return bad;
}
//---------------------------------------------------------------------------
/*
 * Checks if there are vertices with no elements/edges attached
 */
int
TriMeshLin::processVertices(void)
{
	int nfmax, nnmax;
	int nfmin, nnmin;
	int bad = 0;

	for (unsigned n = 0; n < m_numverts; n++) {
		int nn, nf;
		nn = getNodeNbrs(n).count();
		nf = getFaceNbrs(n).count();

		if (n == 0) {
			nfmin = nfmax = nf;
			nnmin = nnmax = nn;
		} else {

			if (nnmin > nn)
				nnmin = nn;
			if (nnmax < nn)
				nnmax = nn;
			if (nfmin > nf)
				nfmin = nf;
			if (nfmax < nf)
				nfmax = nf;
		}
		if (nf == 0) {
			bad++;
			delVertex(n);
			n--;
		}
	}

	MESH_LOG("%d vertices, %d bad - ", m_numverts, bad);
	MESH_LOG("min %d, max %d node - ", nnmin, nnmax);
	MESH_LOG("min %d, max %d face neighbors\n", nfmin, nfmax);

	return bad;
}
//---------------------------------------------------------------------------
int
TriMeshLin::orientElement(char *fflags, int elem)
{
	unsigned n1, n2;
	n2 = getElemInd(elem,2);

	int rev = 0;
	int cnt = 0;

	for (int n = 0; n < 3; n++, n2 = n1) {
		n1 = getElemInd(elem, n);
		Edge *e = getEdge(n1, n2);
		if (e->nelem > 2)
			return 1;
		if (e->nelem < 2)
			continue;
		int el = e->elem[0];

		if (el == elem)
			el = e->elem[1];
		if (fflags[el] < 1 || fflags[el] > 2)
			continue;

		int i, rr;
		for (i = 0; i < 3; i++)
			if (getElemInd(el,i) == n1)
				break;

		i++;
		if (i == 3)
			i = 0;
		if (getElemInd(el, i) == n2)
			rr=0;
		else {
			i -= 2;
			if (i<0)
				i += 3;
			if (getElemInd(el,i) == n2)
				rr=1;
			else
				return 4;
		}
		if (cnt && rr != rev)
			return 3;
		rev = rr;
		cnt++;
	}

	if (cnt == 0)
		return 4;

	if (rev) {
		int tmp = getElemInd(elem, 0);
		setElemInd(elem, 0, getElemInd(elem, 1));
		setElemInd(elem, 1, tmp);
		MESH_LOG("element %d reversed\n", elem);
		invalidateNormals();
	}

	return 1;
}
//---------------------------------------------------------------------------
int
TriMeshLin::orientNeighbors(char *fflags, int elem)
{
	int n1, n2;
	n2 = getElemInd(elem, 2);
	for (int n = 0; n < 3; n++, n2 = n1) {
		n1 = getElemInd(elem, n);
		Edge *e = getEdge(n1, n2);

		if (e->nelem > 2)
			return 4;
		if (e->nelem < 2)
			continue;

		int el = e->elem[0];

		if (el == elem)
			el=e->elem[1];
		if (fflags[el])
			continue;
		fflags[el] = orientElement(fflags,el);
	}

	return 2;
}
//---------------------------------------------------------------------------
int
TriMeshLin::checkOrientation(void)
{
	char *fflags = new char[m_numtris];
	// flag values:
	// 0: not checked
	// 1: checked
	// 2: processed (neighbors checked)
	// 3: error (checked neighbors with conflicting orientations)
	// 4: error (mesh connectivity problem)

	MESH_LOG("Checking Orientation:\n");
	for (unsigned int n = 0; n < m_numtris; n++)
		fflags[n] = 0;

	// mark start of each class as checked
	int c = 0;
	for (int n = 0; n < getNumClasses(); n++) {
		fflags[c] = 1;
		c += getNumTris(n);
	}

	for (;;) {
		int found = 0;
		for (unsigned n = 0; n < m_numtris; n++) {
			if (fflags[n] != 1)
				continue;
			found = 1;
			fflags[n] = orientNeighbors(fflags, n);
		}
		if (!found)
			break;
	}

	int bad = 0;
	int err = 0;
	int out = 0;
	for (unsigned int n = 0; n < m_numtris; n++) {
		if (fflags[n] == 3)
			bad++;
		else if (fflags[n] == 4)
			err++;
		else if (fflags[n] == 0)
			out++;
	}

	delete[] fflags;

	MESH_LOG("%d triangles, %d disoriented, %d errors, %d unreachable\n",
	       m_numtris, bad, err, out);

	return 0;
}
//---------------------------------------------------------------------------
int
TriMeshLin::saveColorInfo(FILE *f, int numv, double r, double g, double b)
{
	assert(f);
	fprintf(f, "bind c vertex\n");
	for (int m = 0; m < numv; m++)
		fprintf(f, "c %g %g %g\n", r, g, b);

	return 0;
}
//---------------------------------------------------------------------------
int
TriMeshLin::saveClass(const char *name, int cls)
{
	MESH_LOG("Saving class %d to file %s\n", cls, name);
	int *fmap = new int[m_numverts];
	int *rmap = new int[m_numverts];

	for (unsigned n = 0; n < m_numverts; n++)
		fmap[n] = rmap[n] = -1;

	int start = 0;
	for (int c = 0; c < cls; c++)
		start += m_fsizes[c];
	int size = m_fsizes[cls];

	// map nodes
	int numv = 0;
	for (int f = 0; f < size; f++) {
		for (int m = 0; m < 3; m++) {
			int nd = getElemInd(start + f, m);
			if (fmap[nd] < 0) {
				fmap[nd] = numv;
				rmap[numv++] = nd;
			}
		}
	}

	MESH_LOG("Class contains %d faces, %d vertices\n", size, numv);

	FILE *f = fopen(name, "w");
	if (f == 0)
		return 1;

	for (int m = 0; m < numv; m++) {
		int n = rmap[m];
		fprintf(f, "v %f %f %f\n", m_verts[n].getX(),
			m_verts[n].getY(), m_verts[n].getZ());
	}

	for (int n = 0; n < size; n++) {
		fprintf(f, "t");
		for (int m = 0; m < 3; m++) {
			int nd = fmap[getElemInd(start + n, m)] + 1;
			assert(nd > 0);
			fprintf(f, " %d", nd);
		}
		fprintf(f,"\n");
	}

	saveColorInfo(f, numv, 1, 1, 1);

	fclose(f);

	delete[] fmap;
	delete[] rmap;
	return 0;
}
//---------------------------------------------------------------------------
int
TriMeshLin::saveSelected(const char *name, const double *sel)
{
	int size = 0;
	for (int i = 0; i < m_numtris; i++)
		if(sel[i])
			size++;

	MESH_LOG("Saving selected elements to file %s\n", name);

	int *fmap = new int[m_numverts];
	int *rmap = new int[m_numverts];

	for (unsigned n = 0; n < m_numverts; n++)
		fmap[n] = rmap[n] = -1;

	// map nodes
	int numv = 0;
	for (int f = 0; f < m_numtris; f++) {
		if (sel[f] == 0)
			continue;
		for (int m = 0; m < 3; m++) {
			int nd = getElemInd(f, m);
			if (fmap[nd] < 0) {
				fmap[nd] = numv;
				rmap[numv++] = nd;
			}
		}
	}

	MESH_LOG("Selection contains %d faces, %d vertices\n", size, numv);

	FILE *f = fopen(name, "w");
	if (f == 0)
		return 1;

	for (int m = 0; m < numv; m++) {
		int n = rmap[m];
		fprintf(f, "v %f %f %f\n", m_verts[n].getX(),
			m_verts[n].getY(), m_verts[n].getZ());
	}

	for (int n = 0; n < m_numtris; n++) {
		if (sel[n] == 0)
			continue;
		fprintf(f, "t");
		for (int m = 0; m < 3; m++) {
			int nd = fmap[getElemInd(n, m)] + 1;
			assert(nd > 0);
			fprintf(f, " %d", nd);
		}
		fprintf(f,"\n");
	}

	for (int n = 0; n < m_numtris; n++) {
		if (sel[n] == 0)
			continue;
		fprintf(f, "e %g\n", sel[n]);
	}

	fclose(f);

	delete[] fmap;
	delete[] rmap;
	return 0;
}
//---------------------------------------------------------------------------
int
TriMeshLin::save(const char *name)
{
	FILE *f = fopen(name,"w");
	if (f == 0)
		return 1;

	for (unsigned int n = 0; n < m_numverts; n++)
		fprintf(f, "v %f %f %f\n", m_verts[n].getX(),
			m_verts[n].getY(), m_verts[n].getZ());

	const unsigned int *u = getTriIndex();

	for(unsigned int n = 0; n < m_numtris; n++, u += 3)
		fprintf(f, "t %d %d %d\n", u[0] + 1, u[1] + 1, u[2] + 1);

	saveColorInfo(f, m_numverts, 1, 1, 1);

	fclose(f);

	return 0;
}
//---------------------------------------------------------------------------
void
TriMeshLin::calcNeighbors(void)
{
	MESH_LOG("Calculating neighbors\n");

	m_nnode.resize(m_numverts);
	m_nface.resize(m_numverts);

	for (unsigned int n = 0; n < m_numverts; n++) {
		getNodeNbrs(n).clear();
		getFaceNbrs(n).clear();
	}

	for (unsigned int n = 0; n < m_numtris; n++) {
		unsigned int *u = (unsigned int *) &m_tris[3 * n];
		for (int m = 0; m < 3; m++) {
			getFaceNbrs(u[m]).add(n);
			for (int k = 0; k < 3; k++) {
				if (k == m)
					continue;
				getNodeNbrs(u[m]).add(u[k]);
			}
		}
	}

	MESH_LOG("Done.\n");
}
//---------------------------------------------------------------------------
// Debugging function for checking neighbors
void
TriMeshLin::checkNeighbors(void)
{
	MESH_LOG("Checking  neighbors\n");

	vector<Neighbor> nnode;  // neighboring nodes
	vector<Neighbor> nface;  // neighboring faces

	nnode.resize(m_numverts);
	nface.resize(m_numverts);

	// reconstruct neighbor list
	for (unsigned int n = 0; n < m_numtris; n++) {
		unsigned int *u = (unsigned int *) &m_tris[3 * n];

		for (int m = 0; m < 3; m++) {
			nface[u[m]].add(n);
			for (int k = 0; k < 3; k++) {
				if (k == m)
					continue;
				nnode[u[m]].add(u[k]);
			}
		}
	}

	for (unsigned int n = 0; n < m_numverts; n++) {
		const Neighbor &n1 = nnode[n];
		const Neighbor &n2 = getNodeNbrs(n);
		if (n1 == n2 )
			continue;
		MESH_LOG("Node %d node neighborhood mismatch: ", n);
		for (int m = 0; m < max(n1.count(), n2.count()); m++)
			MESH_LOG_ADD("%d:%d ", n1[m], n2[m]);
		MESH_LOG_ADD("\n");
	}

	for (unsigned int n = 0; n < m_numverts; n++) {
		const Neighbor &n1 = nface[n];
		const Neighbor &n2 = getFaceNbrs(n);
		if (n1 == n2 )
			continue;
		MESH_LOG("Node %d face neighborhood mismatch: ", n);
		for (int m = 0; m < max(n1.count(), n2.count()); m++)
			MESH_LOG_ADD("%d:%d ", n1[m], n2[m]);
		MESH_LOG_ADD("\n");
	}

	MESH_LOG("Done.\n");
}
//---------------------------------------------------------------------------
const Point3 &
TriMeshLin::updateFaceNormal(unsigned int f)
{
	unsigned *u = &m_tris[3*f];
	Point3 cr;

	Point3 v1(m_verts[u[1]] - m_verts[u[0]]);
	Point3 v2(m_verts[u[2]] - m_verts[u[1]]);

	cr = Cross(v1,v2);
	cr.normalize();
	m_fnorms[f] = cr;

	return m_fnorms[f];
}
//---------------------------------------------------------------------------
void
TriMeshLin::calcFaceNorm(void)
{
	if (m_fnorms_valid)
		return;

	m_fnorms.resize(m_numtris);

	for (unsigned n = 0; n < m_numtris; n++)
		updateFaceNormal(n);

	m_fnorms_valid = true;
}
//---------------------------------------------------------------------------
void
TriMeshLin::calcNormals(void)
{
	if (m_norms_valid)
		return;

	calcFaceNorm();

	if (m_nface.empty())
		calcNeighbors();

	for (unsigned n = 0; n < m_numverts; n++) {
		Point3 nt(0, 0, 0);
		Neighbor &nb = getFaceNbrs(n);
		int m;
		for (m = 0; m < MAX_NEIGHBOR; m++) {
			if (nb[m] < 0)
				break;
			nt += m_fnorms[nb[m]];
		}

		if (m == 0){
//			printf("Vertex with no neighbors: %d!\n", n);
			nt.setX(1);
		} else
			nt.normalize();
		m_norms[n] = nt;
	}
	m_norms_valid = true;
}
//---------------------------------------------------------------------------
float
TriMeshLin::calcPhi(int i, int j)
{
	double dist = (m_verts[i] - m_verts[j]).length();

	if (dist == 0)
		return 1e20;
	return 1/dist;
}
//---------------------------------------------------------------------------
void
TriMeshLin::delxyz(int nv, Point3 &del)
{
        float phi[MAX_NEIGHBOR], sumphi=0;
        int nn;
        Neighbor &nb = getNodeNbrs(nv);

        for (nn = 0; nn < MAX_NEIGHBOR; nn++) {
		if (nb[nn] < 0)
			break;
		sumphi += phi[nn] = calcPhi(nv, nb[nn]);
        }

        del.setCoord(0,0,0);
        if (sumphi == 0)
		return;
        for (int n = 0; n < nn; n++)
		del += (phi[n] / sumphi) * (m_verts[nb[n]] - m_verts[nv]);
}
//---------------------------------------------------------------------------
void
TriMeshLin::smooth(int iter)
{
/*	static Point3 *delvec=0;
	
if(delvec==0) delvec=new Point3[m_numverts];
*/

//    printf("Smoothing ...\n");

/*
  for(int n=0; n<m_numverts; n++) m_nflags[n]=0;
  for(int n=0; n<m_numverts; n++){
  for(Edge *e=m_edges[n]; e; e=e->next){
  if(e->nelem!=2){
  m_nflags[e->node1]=m_nflags[e->node2]=1;
  break;
  }
  }
  }
*/


	for(int i = 0; i < iter; i++) {
//		printf("iteration %d\n",i);
/*		for(int n=0; n<m_numverts; n++)
		delxyz(n,delvec[n]);
		for(int n=0; n<m_numverts; n++)
		m_verts[n]+=m_lambda*delvec[n];
		for(int n=0; n<m_numverts; n++)
		delxyz(n,delvec[n]);
		for(int n=0; n<m_numverts; n++)
		m_verts[n]+=m_mu*delvec[n];
*/
		invalidateNormals();
		for (unsigned int n = 0; n < m_numverts; n++) {
			if (m_nflags[n])
				continue;
			Point3 dv;
			delxyz(n, dv);
			m_verts[n] += m_lambda * dv;
		}
		for (unsigned int n = 0; n < m_numverts; n++) {
			if (m_nflags[n])
				continue;
			Point3 dv;
			delxyz(n, dv);
			m_verts[n] += m_mu * dv;
		}
	}

	calcNormals();
//	printf("Done.\n");
}
//---------------------------------------------------------------------------
void
TriMeshLin::classifyFace(int *fclass, int face, int cls)
{
	for (int n = 0; n < 3; n++) {
		int nd = getElemInd(face, n);
		for (int b = 0; b < MAX_NEIGHBOR; b++) {
			int el = getFaceNbrs(nd)[b];
			if (el < 0)
				break;
			int c = fclass[el];
			if (c < 0)
				fclass[el] = cls;
			else if (( c | 1) != cls)
				MESH_LOG("Class mismatch!\n");
		}
	}
}
//---------------------------------------------------------------------------
void
TriMeshLin::sortFaces(int *fclass, int cls, int start)
{
	int end = m_numtris-1;
	for (;;) {
		while (fclass[start] == cls && start < end)
			start++;

		while (fclass[end] != cls && start < end)
			end--;
		if (start >= end)
			return;

		for (int n = 0; n < 3; n++) {
			int tmp = getElemInd(start, n);
			setElemInd(start, n, getElemInd(end, n));
			setElemInd(end, n, tmp);
		}
		fclass[end] = fclass[start];
		fclass[start] = cls;
	}
}
//---------------------------------------------------------------------------
int
TriMeshLin::classifyFaces(void)
{
	MESH_LOG("Classifying faces\n");

	calcNeighbors();

	int *fclass = new int[m_numtris];
	for (unsigned int n = 0; n < m_numtris; n++)
		fclass[n] = -1;

	int cls = 0;

	for (;;) {
		unsigned int n;
		for (n = 0; n < m_numtris; n++)
			if (fclass[n] < 0)
				break;

		if (n == m_numtris)
			break;
		fclass[n] = cls + 1;

		for (;;) {
			int found = 0;
			for (unsigned int m = n; m < m_numtris; m++) {
				if (fclass[m] < 0)
					continue;
				if ((fclass[m] & 1) == 0)
					continue;
				classifyFace(fclass, m, cls + 1);
				fclass[m] = cls;
				found = 1;
			}
			if (!found)
				break;
		}
		cls += 2;
	}

	cls /= 2;

	MESH_LOG("%d classes found\n", cls);

	int *histog = new int[cls];
	for (int n = 0; n < cls; n++)
		histog[n] = 0;

	for (unsigned int n = 0; n < m_numtris; n++) {
		int c = fclass[n] / 2;
		fclass[n] = c;
		histog[c]++;
	}

	int start = 0;
	m_fsizes.clear();
	for (int n = 0; n < cls; n++) {
		int max = histog[0];
		int pos = 0;
		for (int m = 0; m < cls; m++) {
			if (histog[m] > max) {
				max = histog[m];
				pos = m;
			}
		}
		sortFaces(fclass, pos, start);
		start += max;
		histog[pos] = 0;
		m_fsizes.push_back(max);
	}

	clearEdges();
	// and reclaculate them
	calcNeighbors();
	findEdges();

//	FILE *f=fopen("histog.out","wt");
//	for(int n=0; n<cls; n++)
//		fprintf(f,"%d %d\n",n+1,histog[n]);
//	fclose(f);

	delete[] histog;
	delete[] fclass;
	return cls;
}
void
TriMeshLin::clearEdges(void)
{
	for (unsigned int n = 0; n < m_numverts; n++) {
		while (m_edges[n]) {
			Edge *e = m_edges[n];
			m_edges[n] = e->next;
			delete e;
		}
		getFaceNbrs(n).clear();
	}
	m_numedges = 0;
}
//---------------------------------------------------------------------------
void
TriMeshLin::calcLimits(void)
{
        double minx, miny, minz;
        double maxx, maxy, maxz;
        double sumx, sumy, sumz;

        sumx = minx = maxx = m_verts[0].getX();
        sumy = miny = maxy = m_verts[0].getY();
        sumz = minz = maxz = m_verts[0].getZ();

        for (unsigned n = 1; n < m_numverts; n++) {
                double x = m_verts[n].getX();
                double y = m_verts[n].getY();
                double z = m_verts[n].getZ();
                sumx += x;
                sumy += y;
                sumz += z;

                if (minx > x)
			minx = x;
                if (maxx < x)
			maxx = x;
                if (miny > y)
			miny = y;
                if (maxy < y)
			maxy = y;
                if (minz > z)
			minz = z;
                if (maxz < z)
			maxz = z;
        }

	m_mean.setCoord(sumx / m_numverts, sumy / m_numverts, sumz / m_numverts);
        m_size.setCoord(maxx - minx, maxy - miny, maxz - minz);
	m_min.setCoord(minx, miny, minz);
}
//---------------------------------------------------------------------------
void
TriMeshLin::moveMesh(double dx, double dy, double dz)
{
        for (unsigned n = 0; n < m_numverts; n++)
		m_verts[n] += Point3(dx, dy, dz);

        calcLimits();
}
//---------------------------------------------------------------------------
void
TriMeshLin::scaleMesh(double sx, double sy, double sz)
{
        for (unsigned n = 0; n <m_numverts; n++) {
                m_verts[n].X() *= sx;
                m_verts[n].Y() *= sy;
                m_verts[n].Z() *= sz;
        }

        calcLimits();
	invalidateNormals();
        calcNormals();
}
//---------------------------------------------------------------------------
// remove the elements with bad aspect ratios (dmin / dmax < tresh)
// return total number of elements removed
int
TriMeshLin::removeSmallEdges(double etresh)
{
	unsigned int ns = m_numtris;
	int modified;
	unsigned int start = 0;

	do {
		modified = 0;
		for (EdgeIter it(*this); it.value(); it.next()) {
			Edge *e = it.value();
			Point3 p(getVertex(e->node1));
			p -= getVertex(e->node2);
			if (p.length() < etresh)
				modified += collapseEdge(e);
			if (modified)
				break;
		}
	} while(modified);

	if (ns != m_numtris)
		calcNormals();

	return ns - m_numtris;
}
//---------------------------------------------------------------------------
// remove the elements with bad aspect ratios (dmin / dmax < tresh)
// return total number of elements removed
int
TriMeshLin::removeBadAspectElements(double atresh)
{
	unsigned int ns = m_numtris;
	int modified;
	unsigned int start = 0;

	do {
		modified = 0;
		for (unsigned int n = start; n < m_numtris; n++) {
			unsigned int *u = &m_tris[3 * n];
			double d1 = (m_verts[u[0]] - m_verts[u[1]]).length();
			double d2 = (m_verts[u[0]] - m_verts[u[2]]).length();
			double d3 = (m_verts[u[1]] - m_verts[u[2]]).length();
			double dmin, dmax;

			if (d2 > d1) {
				dmin = d1;
				dmax = d2;
			} else {
				dmin = d2;
				dmax = d1;
			}

			if (d3 > dmax)
				dmax = d3;
			else if (d3 < dmin)
				dmin = d3;

			// correctly handles the 0/0 case
			if (dmin > atresh * dmax)
				continue;

			if (dmin == d1)
				modified += collapseEdge(u[0], u[1]);
			else if (dmin == d2)
				modified += collapseEdge(u[0], u[2]);
			else
				modified += collapseEdge(u[1], u[2]);
			if (modified) {
				start = (n > 10) ? n - 10 : 0;
				break;
			}
		}
	} while(modified);

	if (ns != m_numtris)
		calcNormals();

	return ns - m_numtris;
}
//---------------------------------------------------------------------------
// remove small elements where (dmin * dmax < size^2)
// return total number of elements removed
int
TriMeshLin::removeSmallElements(double size)
{
	unsigned int ns = m_numtris;
	unsigned int start = 0;
	int modified;

	do {
		modified = 0;
		for (unsigned int n = start; n < m_numtris; n++) {
			unsigned int *u = &m_tris[3 * n];
			double d1 = (m_verts[u[0]] - m_verts[u[1]]).length();
			double d2 = (m_verts[u[0]] - m_verts[u[2]]).length();
			double d3 = (m_verts[u[1]] - m_verts[u[2]]).length();
			double dmin, dmax;

			dmax = (d2 > d1) ? d2 : d1;
			dmax = (d3 > dmax) ? d3 : dmax;

			dmin = (d2 > d1) ? d1 : d2;
			dmin = (d3 > dmin) ? dmin : d3;

			// correctly handles the 0/0 case
			if ((dmax *dmin) > size * size)
				continue;

			collapseElement(n);
			modified++;
			start = (n > 10) ? (n - 10) : 0;
			break;
		}
	} while(modified);

	calcNormals();

	return ns - m_numtris;
}
//---------------------------------------------------------------------------
// remove the elements with bad aspect ratios (dmin / dmax < tresh)
// return total number of elements removed
int
TriMeshLin::flipElements(void)
{
	int flipped = 0;

	// we need valid normal information
	calcNormals();

	for (unsigned int n = 0; n < m_numverts; n++) {
		Edge *en;
		for (Edge *e = m_edges[n]; e != NULL; e = en) {
			en = e->next;
			flipped += checkFlipEdge(e);
		}
	}

	return flipped;
}
//---------------------------------------------------------------------------
void
TriMeshLin::collapseElement(unsigned int elem)
{
	unsigned int *eu = &m_tris[3 * elem];
	unsigned int v[3];

	v[0] = eu[0];
	v[1] = eu[1];
	v[2] = eu[2];

	Edge *e;

	MESH_LOG("Collapsing element %d\n", elem);
	invalidateNormals();

	// first delete neighboring elements
	e = getEdge(v[0], v[1]);
	while (e->nelem > 0)
		delElem(e->elem[0]);

	e = getEdge(v[0], v[2]);
	while (e->nelem > 0)
		delElem(e->elem[0]);

	e = getEdge(v[1], v[2]);
	while (e->nelem > 0)
		delElem(e->elem[0]);

	// move one vertex to mid point of element
	m_verts[v[0]] = (m_verts[v[0]] + m_verts[v[1]] + m_verts[v[2]]) / 3;
	e = NULL; // will be invalid

	// delete all edges belonging to v1, v2 and v3
	for (int m = 0; m < 3; m++) {
		while (getNodeNbrs(v[m]).count()) {
			unsigned int vn = getNodeNbrs(v[m])[0];
			delEdge(v[m], vn);
			getNodeNbrs(v[m]).del(vn);
			getNodeNbrs(vn).del(v[m]);
		}
	}

	// re-add edges of v1 from its face list
	for (int n = 0; n < MAX_NEIGHBOR; n++) {
		unsigned int f = getFaceNbrs(v[0])[n];
		unsigned int *u = &m_tris[3 * f];
		if ((int)f < 0)
			break;
		for (int m = 0; m < 3; m++) {
			if (u[m] == v[0])
				continue;
			addEdge(v[0], u[m], f);
			getNodeNbrs(v[0]).add(u[m]);
			getNodeNbrs(u[m]).add(v[0]);
		}
	}

	// add all neighboring faces of v2 to v1
	while (getFaceNbrs(v[1]).count() > 0) {
		unsigned int f = getFaceNbrs(v[1])[0];
		getFaceNbrs(v[1]).del(f);
		getFaceNbrs(v[0]).add(f);

		unsigned int *u = &m_tris[3 * f];
		for (int m = 0; m < 3; m++) {
			if (u[m] == v[1]) {
				u[m] = v[0];
				continue;
			}
			addEdge(v[0], u[m], f);
			getNodeNbrs(v[0]).add(u[m]);
			getNodeNbrs(u[m]).add(v[0]);
		}
	}

	// add all neighboring faces of v2 to v1
	while (getFaceNbrs(v[2]).count() > 0) {
		unsigned int f = getFaceNbrs(v[2])[0];
	        getFaceNbrs(v[2]).del(f);
		getFaceNbrs(v[0]).add(f);

		unsigned int *u = &m_tris[3 * f];
		for (int m = 0; m < 3; m++) {
			if (u[m] == v[2]) {
				u[m] = v[0];
				continue;
			}
			addEdge(v[0], u[m], f);
			getNodeNbrs(v[0]).add(u[m]);
			getNodeNbrs(u[m]).add(v[0]);
		}
	}

	// remove nodes v2 and v3
	unlinkVertex(v[1]);
	unlinkVertex(v[2]);
	MESH_LOG("Collapsed one element\n");
}
//---------------------------------------------------------------------------
int
TriMeshLin::collapseEdge(Edge *e)
{
	assert(e != NULL);
	unsigned int v1 = e->node1;
	unsigned int v2 = e->node2;

	MESH_LOG("Collapsing edge %d -- %d\n", v1, v2);

	int nc = 0;
	for (int n = 0; n < MAX_NEIGHBOR; n++) {
		unsigned int n1 = getNodeNbrs(v1)[n];
		if ((int)n1 < 0)
			break;
		for (int m = 0; m < MAX_NEIGHBOR; m++) {
			unsigned int n2 = getNodeNbrs(v2)[m];
			if ((int)n2 < 0)
				break;
			if (n1 == n2) {
				nc++;
				break;
			}
		}
	}

	if (nc != e->nelem) {
		MESH_LOG("Not collapsing edge!\n");
		return 0;
	}
	invalidateNormals();

	// first delete neighboring elements
	while (e->nelem > 0)
		delElem(e->elem[0]);

	// move one vertex to mid point of edge
	m_verts[v1] = (m_verts[v1] + m_verts[v2]) / 2;
	e = NULL; // will be invalid

	// delete all edges belonging to v1
	while ((int)getNodeNbrs(v1)[0] >= 0) {
		unsigned int v = getNodeNbrs(v1)[0];
		delEdge(v1, v);
		getNodeNbrs(v1).del(v);
		getNodeNbrs(v).del(v1);
	}

	// delete all edges belonging to v2
	while ((int)getNodeNbrs(v2)[0] >= 0) {
		unsigned int v = getNodeNbrs(v2)[0];
		delEdge(v2, v);
		getNodeNbrs(v2).del(v);
		getNodeNbrs(v).del(v2);
	}

	// re-add edges of v1 from its face list
	for (int n = 0; n < MAX_NEIGHBOR; n++) {
		unsigned int f = getFaceNbrs(v1)[n];
		unsigned int *u = &m_tris[3 * f];
		if ((int)f < 0)
			break;
		for (int m = 0; m < 3; m++) {
			if (u[m] == v1)
				continue;
			addEdge(v1, u[m], f);
			getNodeNbrs(v1).add(u[m]);
			getNodeNbrs(u[m]).add(v1);
		}
	}

	// add all neighboring faces of v2 to v1
	while ((int)getFaceNbrs(v2)[0] >= 0) {
		unsigned int f = getFaceNbrs(v2)[0];
		getFaceNbrs(v2).del(f);
		getFaceNbrs(v1).add(f);

		unsigned int *u = &m_tris[3 * f];
		for (int m = 0; m < 3; m++) {
			if (u[m] == v2) {
				u[m] = v1;
				continue;
			}
			addEdge(v1, u[m], f);
			getNodeNbrs(v1).add(u[m]);
			getNodeNbrs(u[m]).add(v1);
		}
	}

	// remove node v2
	unlinkVertex(v2);
	MESH_LOG("Collapsed one edge\n");

	return 1;
}
//---------------------------------------------------------------------------
void
TriMeshLin::delVertex(unsigned int v)
{
	assert(getFaceNbrs(v)[0] == -1);
	assert(getNodeNbrs(v)[0] == -1);

	unsigned int last = m_numverts - 1;
	MESH_LOG("Deleting vertex %d, total %d\n", v, last);

	invalidateNormals();

	if (v < last) {
		m_verts[v] = m_verts[last];
		m_norms[v] = m_norms[last];
		m_nflags[v] = m_nflags[last];

		for (int n = 0; n < MAX_NEIGHBOR; n++) {
			int vn = getNodeNbrs(last)[n];
			if (vn < 0)
				break;
			Edge *e = getEdge(last, vn);
			for (int m = 0; m < e->nelem; m++)
				addEdge(v, vn, e->elem[m]);

			delEdge(last, vn);
			getNodeNbrs(vn).del(last);
			getNodeNbrs(vn).add(v);
			getNodeNbrs(v).add(vn);
		}

		for (int n = 0; n < MAX_NEIGHBOR; n++) {
			int f = getFaceNbrs(last)[n];
			if (f < 0)
				break;
			getFaceNbrs(v).add(f);
			unsigned int *u = (unsigned int *) &m_tris[3 * f];
			for (int m = 0; m < 3; m++) {
				if (u[m] == last)
					u[m] = v;
			}
		}
	}

	m_numverts--;
	m_verts.resize(m_numverts);
	m_norms.resize(m_numverts);
	m_nflags.resize(m_numverts);
	m_edges.resize(m_numverts);
	m_nnode.resize(m_numverts);
	m_nface.resize(m_numverts);
}
//---------------------------------------------------------------------------
void
TriMeshLin::capacity(unsigned int ne, unsigned int nv)
{
	m_verts.reserve(nv);
	m_norms.reserve(nv);
	m_nflags.reserve(nv);
	m_edges.reserve(nv);
	m_nnode.reserve(nv);
	m_nface.reserve(nv);

	m_tris.resize(ne * 3);
	m_fnorms.resize(ne);	
}
//---------------------------------------------------------------------------
unsigned int
TriMeshLin::addVertex(const Point3 &v)
{
	unsigned int i = m_numverts++;
	m_verts.resize(m_numverts);
	m_norms.resize(m_numverts);
	m_nflags.resize(m_numverts);
	m_edges.resize(m_numverts);
	m_nnode.resize(m_numverts);
	m_nface.resize(m_numverts);
	m_verts[i] = v;

	return i;
}
//---------------------------------------------------------------------------
int
TriMeshLin::moveVertex(int vert, const Point3 &v)
{
	if (vert < 0 || vert >= m_numverts)
		return -1;

	m_verts[vert] = v;

	/* XXX only calculate neighboring? */
	invalidateNormals();

	return 0;
}
//---------------------------------------------------------------------------
void
TriMeshLin::setElem(unsigned int e, unsigned int i,
		    unsigned int j, unsigned int k)
{
	setElemInd(e, 0, i);
	setElemInd(e, 1, j);
	setElemInd(e, 2, k);

	// fix face neighbors
	getFaceNbrs(i).add(e);
	getFaceNbrs(j).add(e);
	getFaceNbrs(k).add(e);

	// fix node neighbors
	getNodeNbrs(i).add(j);
	getNodeNbrs(i).add(k);
	getNodeNbrs(j).add(i);
	getNodeNbrs(j).add(k);
	getNodeNbrs(k).add(i);
	getNodeNbrs(k).add(j);

	// add Edges
	addEdge(i, j, e);
	addEdge(j, k, e);
	addEdge(k, i, e);

	updateFaceNormal(e);
	invalidateVertexNormals();
}
//---------------------------------------------------------------------------
unsigned int
TriMeshLin::addElem(unsigned int i, unsigned int j, unsigned int k)
{
	assert(i < m_numverts);
	assert(j < m_numverts);
	assert(k < m_numverts);
	assert(i != j);
	assert(i != k);
	assert(k != j);

	unsigned int e = m_numtris++;
	m_tris.resize(m_numtris * 3);
	m_fnorms.resize(m_numtris);

	setElem(e, i, j, k);

	return e;
}
//---------------------------------------------------------------------------
void
TriMeshLin::changeElem(unsigned int e, unsigned int i,
		       unsigned int j, unsigned int k)
{
	assert(i < m_numverts);
	assert(j < m_numverts);
	assert(k < m_numverts);
	assert(i != j);
	assert(i != k);
	assert(k != j);

	// XXX class bacomes bad, need re-classify!
	unsigned int last = m_numtris - 1;
	unsigned int *u = &m_tris[3 * e];

	MESH_LOG("Changing element %d\n", e);

	if (delEdgeElem(u[0], u[1], e) == 0) {
		getNodeNbrs(u[0]).del(u[1]);
		getNodeNbrs(u[1]).del(u[0]);
		delEdge(u[0], u[1]);
	}
	if (delEdgeElem(u[0], u[2], e) == 0) {
		getNodeNbrs(u[0]).del(u[2]);
		getNodeNbrs(u[2]).del(u[0]);
		delEdge(u[0], u[2]);
	}
	if (delEdgeElem(u[1], u[2], e) == 0) {
		getNodeNbrs(u[1]).del(u[2]);
		getNodeNbrs(u[2]).del(u[1]);
		delEdge(u[1], u[2]);
	}

	for (int m = 0; m < 3; m++)
		getFaceNbrs(u[m]).del(e);

	setElem(e, i, j, k);
}
//---------------------------------------------------------------------------
// Just make sure the vertex is isolated. but leave it as it is
void
TriMeshLin::unlinkVertex(unsigned int v)
{
	assert(getFaceNbrs(v)[0] == -1);
	assert(getNodeNbrs(v)[0] == -1);
}
//---------------------------------------------------------------------------
int
TriMeshLin::delEdgeElem(unsigned int v1, unsigned int v2, unsigned int elem)
{
	Edge *e = getEdge(v1, v2);
	assert(e);

	int m = 0;
	for (int n = 0; n < e->nelem; n++) {
		if (e->elem[n] == elem)
			continue;
		e->elem[m++] = e->elem[n];
	}
	assert(m == e->nelem - 1);
	e->nelem = m;

	return m;
}
//---------------------------------------------------------------------------
void
TriMeshLin::delElem(unsigned int e)
{
	unsigned int last = m_numtris - 1;
	// XXX class bacomes bad, need re-classify!

	MESH_LOG("Deleting element %d\n", e);

	unsigned int *u = &m_tris[3 * e];

	delEdgeElem(u[0], u[1], e);
	delEdgeElem(u[0], u[2], e);
	delEdgeElem(u[1], u[2], e);

	for (int m = 0; m < 3; m++)
		getFaceNbrs(u[m]).del(e);

	if (e < last) {
		unsigned int *l = &m_tris[3 * last];
		for (int m = 0; m < 3; m++) {
			u[m] = l[m];
			getFaceNbrs(l[m]).del(last);
			getFaceNbrs(l[m]).add(e);
		}

		delEdgeElem(l[0], l[1], last);
		delEdgeElem(l[0], l[2], last);
		delEdgeElem(l[1], l[2], last);

		addEdge(l[0], l[1], e);
		addEdge(l[0], l[2], e);
		addEdge(l[1], l[2], e);
	}

	m_numtris--;
	m_tris.resize(m_numtris * 3);

	invalidateNormals();
}
//---------------------------------------------------------------------------
int
TriMeshLin::flipEdge(Edge *e)
{
	unsigned int e1, e2, n1, n2, p1, p2, *l;

	if (e->nelem != 2)
		return (1);

	e1 = e->elem[0];
	e2 = e->elem[1];

	assert(e1 != e2);

	n1 = e->node1;
	n2 = e->node2;

	assert(n1 != n2);

	p1 = p2 = UINT_MAX;
	
	l = &m_tris[3 * e1];
	for (int m = 0; m < 3; m++) {
		if (l[m] != n1 && l[m] != n2) {
			p1 = l[m];
			break;
		}
	}

	l = &m_tris[3 * e2];
	for (int m = 0; m < 3; m++) {
		if (l[m] != n1 && l[m] != n2) {
			p2 = l[m];
			break;
		}
	}

	assert (p1 != UINT_MAX && p2 != UINT_MAX);

	if (p1 == p2 || getEdge(p1, p2) != NULL) {
		MESH_LOG("Not Flipping: %d -- %d\n", n1, n2);
		return (1);
	}

	MESH_LOG("Flipping: %d -- %d\n", n1, n2);

	// flip element neighbors for p & n
	getFaceNbrs(n1).del(e2);
	getFaceNbrs(p1).add(e2);
	getFaceNbrs(n2).del(e1);
	getFaceNbrs(p2).add(e1);

	// flip edge neighbors
	getNodeNbrs(n1).del(n2);
	getNodeNbrs(n2).del(n1);
	getNodeNbrs(p1).add(p2);
	getNodeNbrs(p2).add(p1);
	
	// flip the elements
	l = &m_tris[3 * e1];
	for (int m = 0; m < 3; m++) {
		if (l[m] == n2) {
			l[m] = p2;
		}
		assert(l[m] == p1 || l[m] == p2 || l[m] == n1);
	}

	l = &m_tris[3 * e2];
	for (int m = 0; m < 3; m++) {
		if (l[m] == n1) {
			l[m] = p1;
		}
		assert(l[m] == p1 || l[m] == p2 || l[m] == n2);
	}

	updateFaceNormal(e1);
	updateFaceNormal(e2);
	invalidateVertexNormals();

	// replace the edge
	delEdge(n1, n2);
	addEdge(p1, p2, e1);
	addEdge(p1, p2, e2);

	delEdgeElem(p1, n2, e1);
	delEdgeElem(n1, p2, e2);
	addEdge(p1, n2, e2);
	addEdge(n1, p2, e1);

	return (0);
}
//---------------------------------------------------------------------------
double
TriMeshLin::nodeAngle(unsigned int n1, unsigned int n2, unsigned int n3)
{
	double a2, b2, c2;

	a2 = (m_verts[n2] - m_verts[n1]).length2();
	b2 = (m_verts[n3] - m_verts[n1]).length2();
	c2 = (m_verts[n3] - m_verts[n2]).length2();

	return acos((a2 + b2 - c2) / (2 * sqrt(a2 * b2)));
}
//---------------------------------------------------------------------------
int
TriMeshLin::checkFlipEdge(Edge *e)
{
	unsigned int e1, e2;
	unsigned int *l;
	unsigned p1, p2, n1, n2;
	double a1, a2;
	double b1, b2;

	if (e->nelem != 2)
		return (0);

	e1 = e->elem[0];
	e2 = e->elem[1];

	const Point3 &v1 = getFaceNormal(e1);
	const Point3 &v2 = getFaceNormal(e2);

	double angle = acos(v1.dot(v2));

// 10 degrees
#define ANGLE_THRESH M_PI/18

	if (angle > ANGLE_THRESH)
		return (0);

	l = &m_tris[3 * e1];
	if (l[0] != e->node1 && l[0] != e->node2) {
		a1 = nodeAngle(l[0], l[1], l[2]);
		p1 = l[0];
		if (e->node1 == l[1]) {
			n1 = l[2];
			n2 = l[1];
		} else {
			n1 = l[1];
			n2 = l[2];
		}
	} else if (l[1] != e->node1 && l[1] != e->node2) {
		a1 = nodeAngle(l[1], l[2], l[0]);
		p1 = l[1];
		if (e->node1 == l[2]) {
			n1 = l[2];
			n2 = l[0];
		} else {
			n1 = l[2];
			n2 = l[0];
		}
	} else {
		a1 = nodeAngle(l[2], l[0], l[1]);
		p1 = l[2];
		if (e->node1 == l[0]) {
			n1 = l[1];
			n2 = l[0];
		} else {
			n1 = l[0];
			n2 = l[1];
		}
	}

	l = &m_tris[3 * e2];
	if (l[0] != e->node1 && l[0] != e->node2) {
		a2 = nodeAngle(l[0], l[1], l[2]);
		p2 = l[0];
	} else if (l[1] != e->node1 && l[1] != e->node2) {
		a2 = nodeAngle(l[1], l[2], l[0]);
		p2 = l[1];
	} else {
		a2 = nodeAngle(l[2], l[0], l[1]);
		p2 = l[2];
	}

	if (a1 + a2 <= M_PI)
		return (0);

	b1 = nodeAngle(n1, p1, p2);
	b2 = nodeAngle(n2, p2, p1);

	if ((a1 + a2) <= (b1 + b2))
		return (0);


	if (flipEdge(e))
		return (0);

	return (1);
}
//---------------------------------------------------------------------------
int
TriMeshLin::isElemNeighbor(int e1, int e2)
{
	for (int i = 0; i < 3; i++) {
		unsigned nd = getElemInd(e1, i);
		if (getFaceNbrs(nd).contains(e2))
			return 1;
	}
	return 0;
}
//---------------------------------------------------------------------------
void
TriMeshLin::replaceElements(const vector<unsigned> &faces)
{
	printf("Replacing element vector:\n");
	printf("Initial number of elements: %d\n", m_numtris);
	m_tris.clear();
	m_tris = faces;
	m_numtris = m_tris.size();

	assert((m_numtris % 3) == 0);
	m_numtris /= 3;

	printf("Final number of elements: %d\n", m_numtris);

	invalidateNormals();

	clearEdges();
	findEdges();
	calcNeighbors();
	classifyFaces();

	calcNormals();
}
//---------------------------------------------------------------------------
EdgeIter::EdgeIter(TriMeshLin &msh)
{
	m_nd = 0;
	m_edge = msh.m_edges[0];
	m_mesh = &msh;
}

Edge *
EdgeIter::value(void)
{
	return m_edge;
}

EdgeIter &
EdgeIter::next(void)
{
	if (m_edge == NULL)
		return *this;
	
	m_edge = m_edge->next;

	while (m_edge == NULL && ++m_nd < m_mesh->getNumVerts())
		m_edge = m_mesh->m_edges[m_nd];

	return *this;
}
//---------------------------------------------------------------------------
