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
#include <vector>
#include <set>
#include "meshproc.h"

#define INT_EPS 1e-8


//---------------------------------------------------------------------------
/*
  from: http://astronomy.swin.edu.au/pbourke/geometry/linefacet/

  Determine whether or not the line segment p1,p2
  Intersects the 3 vertex facet bounded by pa,pb,pc
  Return true/false and the intersection point p

  The equation of the line is p = p1 + mu (p2 - p1)
  The equation of the plane is a x + b y + c z + d = 0
  n.x x + n.y y + n.z z + d = 0

  int LineFacet(p1,p2,pa,pb,pc,p)
  XYZ p1,p2,pa,pb,pc,*p;

*/
int
MeshProc::intLineTri(int tri, const Point3 &p1, const Point3 &p2,
		     Point3 &pi) const
{
	int verbose = 0;

	assert(tri >= 0 && tri < m_mesh->getNumTris());

#if 0
	if (tri == 91)
		verbose = 1;
#endif
	const Point3 &pa = m_mesh->getElemVert(tri, 0);
	const Point3 &pb = m_mesh->getElemVert(tri, 1);
	const Point3 &pc = m_mesh->getElemVert(tri, 2);


	if (verbose) {
		printf("  p1: <%g % g %g>\n", p1.getX(), p1.getY(), p1.getZ());
		printf("  p2: <%g % g %g>\n", p2.getX(), p2.getY(), p2.getZ());

		printf("  pa: <%g % g %g>\n", pa.getX(), pa.getY(), pa.getZ());
		printf("  pb: <%g % g %g>\n", pb.getX(), pb.getY(), pb.getZ());
		printf("  pc: <%g % g %g>\n", pc.getX(), pc.getY(), pc.getZ());
	}

	Point3 v1(pb), v2(pc), v3(p2);
	v1-=pa;
	v2-=pa;
	v3-=p1;

	Point3 n(Cross(v1, v2));
	n.normalize();

	double denom = n.dot(v3);

	if(fabs(denom) < INT_EPS)
		return 0; // Line and plane don't intersect

	double d = - n.dot(pa);
	double mu = - (d + n.dot(p1)) / denom;

	if (mu < 0 || mu > 1)
		return 0;  // Intersection not along line segment

#if 0
	if (mu < 0 || mu > 1)
		MESH_LOG("Warning: intersection out of bounds!\n");
#endif

	pi = p1 + (mu * v3);

	if (verbose) {
		printf("  d: %g, mu: %g,\n", d, mu);
		printf("  pi: <%g % g %g>\n", pi.getX(), pi.getY(), pi.getZ());
	}

	double test = n.dot(pi);

	if (fabs(test + d) > INT_EPS)
		return 0;

	// Determine whether or not the intersection point is bounded
	// by pa,pb,pc

	Point3 pa1(pa - pi);
	pa1.normalize();

	Point3 pa2(pb - pi);
	pa2.normalize();

	Point3 pa3(pc - pi);
	pa3.normalize();

	if (verbose) {
		printf("  pa1: <%g % g %g>\n",
		       pa1.getX(), pa1.getY(), pa1.getZ());
		printf("  pa2: <%g % g %g>\n",
		       pa2.getX(), pa2.getY(), pa2.getZ());
		printf("  pa3: <%g % g %g>\n",
		       pa3.getX(), pa3.getY(), pa3.getZ());

		printf("  pa1.dot(pa2): %g, acos(): %g\n",
		       pa1.dot(pa2), acos(pa1.dot(pa2)));
		printf("  pa2.dot(pa3): %g, acos(): %g\n",
		       pa2.dot(pa3), acos(pa2.dot(pa3)));
		printf("  pa3.dot(pa1): %g, acos(): %g\n",
		       pa3.dot(pa1), acos(pa3.dot(pa1)));
	}

	double d12 = pa1.dot(pa2);
	double d23 = pa2.dot(pa3);
	double d31 = pa3.dot(pa1);
	double total = acos(d12 < 1 ? d12 : 1) +
		acos(d23 < 1 ? d23 : 1) +
		acos(d31 < 1 ? d31 : 1);

	if (verbose)
		printf("  total: %g, 2pi: %g, eps: %g\n",
		       total, 2*M_PI, INT_EPS);
	if (fabs(total - 2*M_PI) > INT_EPS)
		return 0;

	return 1;
}
//---------------------------------------------------------------------------
int
MeshProc::edgeNearest(const Point3 &p1, const Point3 &p2, const Point3 &pt,
		     Point3 &pi) const
{
	Point3 v = p2 - p1;
	double len = v.length();
	v.normalize();

	Point3 n = pt - p1;
	double d = n.dot(v);

	if (d < 0 || d > len)
		return 0;

	pi = p1 + (v * d);

	return 1;
}
//---------------------------------------------------------------------------
double 
MeshProc::triNearest (int tri, const Point3 &p1, Point3 &pi) const
{

	assert(tri >= 0 && tri < m_mesh->getNumTris());

	const Point3 &pa = m_mesh->getElemVert(tri, 0);
	const Point3 &pb = m_mesh->getElemVert(tri, 1);
	const Point3 &pc = m_mesh->getElemVert(tri, 2);

	Point3 v1(pb), v2(pc);
	v1-=pa;
	v2-=pa;

	Point3 n(Cross(v1, v2));
	n.normalize();

	double d= - n.dot(pa);
	double mu = - (d + n.dot(p1));
	pi = p1 + (mu * n);

// test pi:
	double test = n.dot(pi);
	if (fabs(test + d) > INT_EPS)
		return 0;

	// Determine whether or not the intersection point is bounded by pa,pb,pc

	Point3 pa1(pa - pi);
	Point3 pa2(pb - pi);
	Point3 pa3(pc - pi);

	pa1.normalize();
	pa2.normalize();
	pa3.normalize();

	double total = acos(pa1.dot(pa2)) + acos(pa2.dot(pa3)) +
		acos(pa3.dot(pa1));

	if (fabs(total - 2*M_PI) < INT_EPS)
		return 0;

	// test edges
	double dmin = -1;
	Point3 pt;

	if (edgeNearest(pa, pb, p1, pt)) {
		double d = (pt - p1).length();
		if (dmin < 0 || dmin > d) {
			dmin = d;
			pi = pt;
		}
	}

	if (edgeNearest(pa, pc, p1, pt)) {
		double d = (pt - p1).length();
		if (dmin < 0 || dmin > d) {
			dmin = d;
			pi = pt;
		}
	}

	if (edgeNearest(pb, pc, p1, pt)) {
		double d = (pt - p1).length();
		if (dmin < 0 || dmin > d) {
			dmin = d;
			pi = pt;
		}
	}

	if (dmin >= 0)
		return 0;

// test points
	dmin = -1;
	d = (pa - p1).length();
	if (dmin < 0 || dmin > d) {
		dmin = d;
		pi = pt;
	}

	d = (pb - p1).length();
	if (dmin < 0 || dmin > d) {
		dmin = d;
		pi = pt;
	}

	d = (pc - p1).length();
	if (dmin < 0 || dmin > d) {
		dmin = d;
		pi = pt;
	}

	return dmin;
}
//---------------------------------------------------------------------------
double
MeshProc::averageEdgeDistance(void)
{
	double sum = 0;
	unsigned ne = 0;

	for (EdgeIter it(*m_mesh); it.value(); it.next()) {
		Edge *e = it.value();
		Point3 p1(m_mesh->getVertex(e->node1));
		Point3 p2(m_mesh->getVertex(e->node2));
		p1 -= p2;
		sum += p1.length();
		ne++;
	}

	return (sum / ne);

}
//---------------------------------------------------------------------------
double
MeshProc::minimumEdgeDistance(void)
{
	double min = -1;
	unsigned ne = 0;

	for (EdgeIter it(*m_mesh); it.value(); it.next()) {
		Edge *e = it.value();
		Point3 p1(m_mesh->getVertex(e->node1));
		Point3 p2(m_mesh->getVertex(e->node2));
		p1 -= p2;
		double d = p1.length();
		if (d < min || min < 0)
			min = d;
	}

	return (min);

}
//---------------------------------------------------------------------------
double
MeshProc::minimumEdgeDistance(int node)
{
	double min = -1;
	Neighbor &nd = m_mesh->getNodeNbrs(node);
	const Point3 &p0 = m_mesh->getVertex(node);	

	for (int n = 0; n < nd.count(); n++) {
		Point3 p1 = m_mesh->getVertex(nd[n]);
		p1 -= p0;
		double d = p1.length();
		if (d < min || min < 0)
			min = d;
	}

	return (min);

}
//---------------------------------------------------------------------------
// Split the element and add the resulting elements to the faces vector
#define V_INVAL 0xFFFFFFFF
int
MeshProc::splitElement(unsigned int e, vector<unsigned int> &faces)
{
	int nf = 0;
	unsigned int p1 = m_mesh->getElemInd(e, 0);
	unsigned int p2 = m_mesh->getElemInd(e, 1);
	unsigned int p3 = m_mesh->getElemInd(e, 2);

	Edge *e1 = m_mesh->getEdge(p1, p2);
	Edge *e2 = m_mesh->getEdge(p2, p3);
	Edge *e3 = m_mesh->getEdge(p3, p1);

	unsigned int c1 = e1->store;
	unsigned int c2 = e2->store;
	unsigned int c3 = e3->store;

	if (c1 == V_INVAL && c2 == V_INVAL && c3 == V_INVAL) {
		faces.push_back(p1);
		faces.push_back(p2);
		faces.push_back(p3);
		return 1;
	}

	// If only one edge is divided, split into two
	if (c1 == V_INVAL && c2 == V_INVAL) {
		// [p1 p2 c3]
		faces.push_back(p1);
		faces.push_back(p2);
		faces.push_back(c3);
		// [c3 p2 p3]
		faces.push_back(c3);
		faces.push_back(p2);
		faces.push_back(p3);
		return 2;
	}
	if (c1 == V_INVAL && c3 == V_INVAL) {
		// [p1 p2 c2]
		faces.push_back(p1);
		faces.push_back(p2);
		faces.push_back(c2);
		// [p1 c2 p3]
		faces.push_back(p1);
		faces.push_back(c2);
		faces.push_back(p3);
		return 2;
	}
	if (c2 == V_INVAL && c3 == V_INVAL) {
		// [p1 c1 p3]
		faces.push_back(p1);
		faces.push_back(c1);
		faces.push_back(p3);
		// [c1 p2 p3]
		faces.push_back(c1);
		faces.push_back(p2);
		faces.push_back(p3);
		return 2;
	}

	if (c1 == V_INVAL) {
		splitElement3(faces, p1, p2, p3, c2, c3);
		return 3;
	}
	if (c2 == V_INVAL) {
		splitElement3(faces, p2, p3, p1, c3, c1);
		return 3;
	}
	if (c3 == V_INVAL) {
		splitElement3(faces, p3, p1, p2, c1, c2);
		return 3;
	}

	// [p1 c1 c3]
	faces.push_back(p1);
	faces.push_back(c1);
	faces.push_back(c3);
	// [c1 p2 c2]
	faces.push_back(c1);
	faces.push_back(p2);
	faces.push_back(c2);
	// [c3 c2 p3]
	faces.push_back(c3);
	faces.push_back(c2);
	faces.push_back(p3);
	// [c1 c2 c3]
	faces.push_back(c1);
	faces.push_back(c2);
	faces.push_back(c3);

	return 4;
}
//---------------------------------------------------------------------------
// Assume edges 2 and 3 need to be split, and split the element by
// using the shortest edge of |p1 c2| and |p2 c3|
void
MeshProc::splitElement3(vector<unsigned int> &faces,
			unsigned int p1, unsigned int p2, unsigned int p3,
			unsigned int c2, unsigned int c3)
{
	Point3 v1(m_mesh->getVertex(p1));
	Point3 v2(m_mesh->getVertex(p2));

	v1 -= m_mesh->getVertex(c2);
	v2 -= m_mesh->getVertex(c3);

	// [c3 c2 p3] common to all
	faces.push_back(c3);
	faces.push_back(c2);
	faces.push_back(p3);

	if (v1.length2() < v2.length2()) {
		// [p1 c2 c3]
		faces.push_back(p1);
		faces.push_back(c2);
		faces.push_back(c3);
		// [p1 p2 c2]
		faces.push_back(p1);
		faces.push_back(p2);
		faces.push_back(c2);
	} else {
		// [p2 c2 c3]
		faces.push_back(p2);
		faces.push_back(c2);
		faces.push_back(c3);
		// [p1 p2 c3]
		faces.push_back(p1);
		faces.push_back(p2);
		faces.push_back(c3);
	}
}
//---------------------------------------------------------------------------
void
MeshProc::splitEdges(double thresh)
{
	for (EdgeIter it(*m_mesh); it.value(); it.next()) {
		Edge *e = it.value();
		Point3 p1(m_mesh->getVertex(e->node1));
		Point3 p2(m_mesh->getVertex(e->node2));
		Point3 vl = p2 - p1;

		double dist = vl.length();
		vl /= dist;
		if (dist < thresh) {
			e->store = V_INVAL;
			continue;
		}

		Point3 pm = (p1 + p2)/2;
#ifdef NOTYET
		// XXX try to calculate a 'better' midpoint
		
		Point3 vn = (m_mesh->getVertexNormal(e->node1) +
			     m_mesh->getVertexNormal(e->node2)) / 2;
		Point3 vp = Cross(vl, vn);
#endif
		e->store = m_mesh->addVertex(pm);
	}

	unsigned int ne = m_mesh->getNumTris();

#if 0
	for (unsigned int e = 0; e < ne; e++) {
		vector<unsigned> faces;
		int ne = splitElement(e, faces);
		if (ne == 1)
			continue;

		MESH_LOG("Splitting element %d by %d:\n", e, ne);
		// replace the element
		m_mesh->changeElem(e, faces[0], faces[1], faces[2]);
		int p = 3;
		
		// and add the remaining elements
		for (int i = 1; i < ne; i++, p += 3) {
			int e = m_mesh->addElem(faces[p], faces[p+1], faces[p+2]);
			MESH_LOG("Added element %d\n", e);
		}
	}
#else
	vector<unsigned> faces;
	for (unsigned int e = 0; e < ne; e++)
		int ne = splitElement(e, faces);

	m_mesh->replaceElements(faces);
#endif
}

SCache *
MeshProc::createElementCache(void)
{
	SCache *sc = new SCache(m_mesh->getMin(), m_mesh->getSize(), 16);
	unsigned int ne = m_mesh->getNumTris();
	vector<nodeset_t> elemnodes;

	elemnodes.resize(ne);
	sc->resize(ne);

	printf("Constructing element cache\n");
	// construct bounding spheres
	for (unsigned int e = 0; e < ne; e++) {
		nodeset_t &nl = elemnodes[e];
		unsigned int ia = m_mesh->getElemInd(e, 0);
		unsigned int ib = m_mesh->getElemInd(e, 1);
		unsigned int ic = m_mesh->getElemInd(e, 2);
	
		nl.insert(ia);
		nl.insert(ib);
		nl.insert(ic);

		const Point3 &p0 = m_mesh->getVertex(ia);
		const Point3 &p1 = m_mesh->getVertex(ib);
		const Point3 &p2 = m_mesh->getVertex(ic);
		Point3 c;
		double r;

		if (elementBoundingSphere(p0, p1, p2, c, r)) {
			printf("Skipping element %d\n", e);
			continue;
		}
		r *= 1.01;
		Sphere3 s(c, r);

		int err = 0;
		if (!s.isInside(p0)) {
			printf("point %d (%g %g %g) not inside BS %d (%g)\n",
			       ia, p0.getX(), p0.getY(), p0.getZ(), e,
			       (c - p0).length());
			err++;
		}
		if (!s.isInside(p1)) {
			printf("point %d (%g %g %g) not inside BS %d (%g)\n",
			       ib, p1.getX(), p1.getY(), p1.getZ(), e,
			       (c - p0).length());
			err++;
		}
		if (!s.isInside(p2)) {
			printf("point %d (%g %g %g) not inside BS %d (%g)\n",
			       ic, p2.getX(), p2.getY(), p2.getZ(), e,
			       (c - p0).length());
			err++;
		}
		if (err)
			printf("  BS: %g %g %g / %g\n",
			       s.getCenter().getX(), s.getCenter().getY(),
			       s.getCenter().getZ(), s.getRadius());

		sc->addSphere(e, s);
	}

	return (sc);
}
//---------------------------------------------------------------------------
struct _pv {
	_pv(int n, const Point3 &v):_n(n), _v(v){};
	Point3 _v;
	int _n;
};

int
MeshProc::pushIntersecting(void)
{
	int num_intersect = 0;
	SCache *sc;

	vector<unsigned> faces;

	unsigned int ne = m_mesh->getNumTris();

	sc = createElementCache();

	list<struct _pv> pushvec;

	printf("Intersecting edges with elements\n");
	// iterate over all edges
	for (EdgeIter it(*m_mesh); it.value(); it.next()) {
		Edge *ed = it.value();
		int ee;

		if (ed->nelem == 0)
			printf("Skipping edge [%d %d] with no neighbors!\n",
			       ed->node1, ed->node2);

		Point3 p1(m_mesh->getVertex(ed->node1));
		Point3 p2(m_mesh->getVertex(ed->node2));

		if (p1 == p2) {
			printf("Skipping zero-length edge [%d %d]\n",
			       ed->node1, ed->node2);
			for (ee = 0; ee < ed->nelem; ee++) {
				unsigned int ei = ed->elem[ee];
				printf("  edge neighbor %d\n", ei);
			}
			continue;
		}

		SCache::spset_t eset;

		// test sphere centered on the edge
		Sphere3 se((p1 + p2)/2, (p2 - p1).length()/2);

		sc->intersect(se, eset);
		if (eset.empty())
			printf("Edge: [%d %d], does not intersect!\n",
			       ed->node1, ed->node2);

		SCache::spset_t::iterator itx;
		for (itx = eset.begin(); itx != eset.end(); itx++) {
			unsigned int e = *itx;
			Point3 pi;

			unsigned int ia = m_mesh->getElemInd(e, 0);
			unsigned int ib = m_mesh->getElemInd(e, 1);
			unsigned int ic = m_mesh->getElemInd(e, 2);
		
			if (ed->node1 == ia || ed->node2 == ia ||
			    ed->node1 == ib || ed->node2 == ib ||
			    ed->node1 == ic || ed->node2 == ic)
				continue;

#if 0
			for (ee = 0; ee < ed->nelem; ee++)
				if (ed->elem[ee] == e)
					break;
			if (ee != ed->nelem)
				continue;
#endif
			if (intLineTri(e, p1, p2, pi) == 0)
				continue;

			const Point3 &pa = m_mesh->getVertex(ia);
			const Point3 &pb = m_mesh->getVertex(ib);
			const Point3 &pc = m_mesh->getVertex(ic);

			if (pa == pi || pb == pi || pc == pi)
				continue;

			printf("Edge [%d %d] intersected with element %d\n",
			       ed->node1, ed->node2, e);

			num_intersect++;

			list<struct _pv>::iterator vi;
			int f1, f2;
			f1 = f2 = 1;
			for (vi = pushvec.begin(); vi != pushvec.end(); vi++) {
				struct _pv &v = *vi;
				if (v._n == ed->node1) {
					v._v += m_mesh->getFaceNormal(e);
					f1 = 0;
				}
				if (v._n == ed->node2) {
					v._v += m_mesh->getFaceNormal(e);
					f2 = 0;
				}
				if (f1 == 0 && f2 == 0)
					break;
			}

			if (f1) {
				struct _pv v1(ed->node1,
					      m_mesh->getFaceNormal(e));
				pushvec.push_front(v1);
			}
			if (f2) {
				struct _pv v2(ed->node2,
					      m_mesh->getFaceNormal(e));
				pushvec.push_front(v2);
			}
		}
	}

	delete sc;

	list<struct _pv>::iterator vi;
	for (vi = pushvec.begin(); vi != pushvec.end(); vi++) {
		struct _pv &v = *vi;
		printf ("Push %d <%g %g %g> -> ",
			v._n, v._v.getX(), v._v.getY(), v._v.getZ());

		v._v.normalize();
		v._v *= minimumEdgeDistance(v._n) * 0.01;
		printf ("%d <%g %g %g>\n",
			v._n, v._v.getX(), v._v.getY(), v._v.getZ());

		v._v += m_mesh->getVertex(v._n);
		m_mesh->moveVertex(v._n, v._v);
	}

	return num_intersect;
}
//---------------------------------------------------------------------------
int
MeshProc::printIntersecting(void)
{
	int num_intersect = 0;
	SCache *sc;

	vector<unsigned> faces;

	unsigned int ne = m_mesh->getNumTris();

	sc = createElementCache();

	printf("Intersecting edges with elements\n");
	// iterate over all edges
	for (EdgeIter it(*m_mesh); it.value(); it.next()) {
		Edge *ed = it.value();
		int ee;

		if (ed->nelem == 0)
			printf("Skipping edge [%d %d] with no neighbors!\n",
			       ed->node1, ed->node2);

		Point3 p1(m_mesh->getVertex(ed->node1));
		Point3 p2(m_mesh->getVertex(ed->node2));

		if (p1 == p2) {
			printf("Skipping zero-length edge [%d %d]\n",
			       ed->node1, ed->node2);
			for (ee = 0; ee < ed->nelem; ee++) {
				unsigned int ei = ed->elem[ee];
				printf("  edge neighbor %d\n", ei);
			}
			continue;
		}

		SCache::spset_t eset;

		// test sphere centered on the edge
		Sphere3 se((p1 + p2)/2, (p2 - p1).length()/2);

		sc->intersect(se, eset);
		if (eset.empty())
			printf("Edge: [%d %d], does not intersect!\n",
			       ed->node1, ed->node2);

		SCache::spset_t::iterator itx;
		for (itx = eset.begin(); itx != eset.end(); itx++) {
			unsigned int e = *itx;
			Point3 pi;

			unsigned int ia = m_mesh->getElemInd(e, 0);
			unsigned int ib = m_mesh->getElemInd(e, 1);
			unsigned int ic = m_mesh->getElemInd(e, 2);
		
			if (ed->node1 == ia || ed->node2 == ia ||
			    ed->node1 == ib || ed->node2 == ib ||
			    ed->node1 == ic || ed->node2 == ic)
				continue;

#if 0
			for (ee = 0; ee < ed->nelem; ee++)
				if (ed->elem[ee] == e)
					break;
			if (ee != ed->nelem)
				continue;
#endif
			if (intLineTri(e, p1, p2, pi) == 0)
				continue;

			const Point3 &pa = m_mesh->getVertex(ia);
			const Point3 &pb = m_mesh->getVertex(ib);
			const Point3 &pc = m_mesh->getVertex(ic);

			if (pa == pi || pb == pi || pc == pi)
				continue;

			printf("Edge [%d %d] intersected with element %d\n",
			       ed->node1, ed->node2, e);

			num_intersect++;
		}
	}
	delete sc;
	return num_intersect;
}
//---------------------------------------------------------------------------
void
MeshProc::splitIntersecting(void)
{
	SCache *sc;

	vector<nodeset_t> elemnodes;
	vector<unsigned> faces;

	unsigned int ne = m_mesh->getNumTris();

	elemnodes.resize(ne);

	sc = createElementCache();

	printf("Intersecting edges with elements\n");
	// iterate over all edges
	for (EdgeIter it(*m_mesh); it.value(); it.next()) {
		Edge *ed = it.value();
		int ee;

		if (ed->nelem == 0)
			printf("Skipping edge [%d %d] with no neighbors!\n",
			       ed->node1, ed->node2);

		Point3 p1(m_mesh->getVertex(ed->node1));
		Point3 p2(m_mesh->getVertex(ed->node2));

		if (p1 == p2) {
			printf("Skipping zero-length edge [%d %d]\n",
			       ed->node1, ed->node2);
			for (ee = 0; ee < ed->nelem; ee++) {
				unsigned int ei = ed->elem[ee];
				printf("  edge neighbor %d\n", ei);
			}
			continue;
		}

		SCache::spset_t eset;

		// test sphere centered on the edge
		Sphere3 se((p1 + p2)/2, (p2 - p1).length()/2);

		sc->intersect(se, eset);
		if (eset.empty())
			printf("Edge: [%d %d], does not intersect!\n",
			       ed->node1, ed->node2);

		SCache::spset_t::iterator itx;
		for (itx = eset.begin(); itx != eset.end(); itx++) {
			unsigned int e = *itx;
			Point3 pi;

			unsigned int ia = m_mesh->getElemInd(e, 0);
			unsigned int ib = m_mesh->getElemInd(e, 1);
			unsigned int ic = m_mesh->getElemInd(e, 2);
		
			if (ed->node1 == ia || ed->node2 == ia ||
			    ed->node1 == ib || ed->node2 == ib ||
			    ed->node1 == ic || ed->node2 == ic)
				continue;

#if 0
			for (ee = 0; ee < ed->nelem; ee++)
				if (ed->elem[ee] == e)
					break;
			if (ee != ed->nelem)
				continue;
#endif
			if (intLineTri(e, p1, p2, pi) == 0)
				continue;

			const Point3 &pa = m_mesh->getVertex(ia);
			const Point3 &pb = m_mesh->getVertex(ib);
			const Point3 &pc = m_mesh->getVertex(ic);

			if (pa == pi || pb == pi || pc == pi)
				continue;

			printf("Edge [%d %d] intersected with element %d\n",
			       ed->node1, ed->node2, e);

			nodeset_t &nl = elemnodes[e];

			if (pi == p1) {
				printf("Adding p1: %d to %d\n", ed->node1, e);
				nl.insert(ed->node1);
			} else if (pi == p2) {
				printf("Adding p2: %d to %d\n", ed->node2, e);
				nl.insert(ed->node2);
			} else {
				unsigned int idx = m_mesh->addVertex(pi);
				printf("Adding vertex %d: %g %g %g ...\n",
				       idx, pi.getX(), pi.getY(), pi.getZ());
				printf("  to element %d\n", e);
				nl.insert(idx);
				for (ee = 0; ee < ed->nelem; ee++) {
					unsigned int ei = ed->elem[ee];
					printf("  to edge neighbor %d\n", ei);
					elemnodes[ei].insert(idx);
				}
			}
		}
	}

	for (unsigned int e = 0; e < ne; e++) {
		nodeset_t &nl = elemnodes[e];

		if (nl.size() == 0) {
			faces.push_back(m_mesh->getElemInd(e, 0));
			faces.push_back(m_mesh->getElemInd(e, 1));
			faces.push_back(m_mesh->getElemInd(e, 2));
		} else {
			nl.insert(m_mesh->getElemInd(e, 0));
			nl.insert(m_mesh->getElemInd(e, 1));
			nl.insert(m_mesh->getElemInd(e, 2));

			printf("Triangulating element %d\n", e);
			triangulateElement(e, faces, nl);
		}
	}

	delete sc;

	m_mesh->replaceElements(faces);
}
//---------------------------------------------------------------------------
static inline Point3 triNormal(const Point3 &a, const Point3 &b, const Point3 &c)
{
	return Cross(a - b, a - c);
}
//---------------------------------------------------------------------------
void
MeshProc::triangulateElement(unsigned int e, nodelist_t &faces,
			     const nodeset_t nl)
{
	// XXX this is the worst possible brute force algorithm, O(n^4)
	int np = nl.size();
	assert (np >= 3);
	vector<unsigned int> ids;
	vector<Point3 *> p;
	nodeset_t::iterator it;
	int id;

	ids.resize(np);
	p.resize(np);

	Point3 en = triNormal(m_mesh->getElemVert(e, 0),
			      m_mesh->getElemVert(e, 1),
			      m_mesh->getElemVert(e, 2));

	// move the points to a vertex
	printf("Triangulating vertices:");
	for (id = 0, it = nl.begin(); it != nl.end(); it++, id++) {
		int idx = *it;
		printf(" %d", idx);
		ids[id] = idx;
		p[id] = &m_mesh->getVertex(idx);
	}
	printf("\n");

	for (int i = 0; i < np - 2; i++) {
		for (int j = i + 1; j < np - 1; j++) {
			for (int k = j + 1; k < np; k++) {
				Point3 pc;
				double r;
				int l;

				if (elementBoundingSphere(*p[i], *p[j], *p[k],
							  pc, r)) {
					printf("BS failed!\n");
					continue;
				}

				double min = r * 2;
				double max = 0;
				for (l = 0; l < np; l++) {
					if (l == i || l == j || l == k)
						continue;
					Point3 pd = pc - *p[l];
					double rx = pd.length();
					if (rx < min)
						min = rx;
					if (rx > max)
						max = rx;
				}

//				printf("r:%g, mi:%g, mx:%g\n", r, min, max);
				if (min < r)
					continue;

				Point3 et = triNormal(*p[i], *p[j], *p[k]);

				faces.push_back(ids[i]);
				if (en.dot(et) > 0) {
					faces.push_back(ids[j]);
					faces.push_back(ids[k]);
				} else {
					faces.push_back(ids[k]);
					faces.push_back(ids[j]);
				}
//				printf("Add element: %d %d %d\n",
//				       ids[i], ids[j], ids[k]);
			}
		}
	}
}
//---------------------------------------------------------------------------
int
MeshProc::elementBoundingSphere(Point3 p1, Point3 p2, Point3 p3,
				  Point3 &center, double &r)
{
	Point3 n1 = p1 - p2;
	Point3 n2 = p2 - p3;
	Point3 n3 = p3 - p1;
	Point3 d1 = Cross(n1, n2);

	double l1 = n1.length2();
	double l2 = n2.length2();
	double l3 = n3.length2();
	double ld = d1.length2();

	if (ld < INT_EPS) {
#if 0
		if (l1 > l2) {
			if (l1 > l3) {
				center = (p1 + p2) / 2;
				r = sqrt(l1) / 2;
			} else {
				center = (p1 + p3) / 2;
				r = sqrt(l3) / 2;
			}
		} else {
			if (l2 > l3) {
				center = (p3 + p2) / 2;
				r = sqrt(l2) / 2;
			} else {
				center = (p1 + p3) / 2;
				r = sqrt(l3) / 2;
			}
		}
#endif
		return 1;
	}

	double a = n1.dot(p1 - p3) * l2 / ld / 2;
	double b = n2.dot(p2 - p1) * l3 / ld / 2;
	double c = n3.dot(p3 - p2) * l1 / ld / 2;

	r = sqrt(l1 * l2 * l3 / ld) / 2;

	center = (a * p1) + (b * p2) + (c * p3);

	return 0;
}
//---------------------------------------------------------------------------
TriMeshLin *
MeshProc::extractClass(int cls)
{
	if (cls < 0 || cls >= m_mesh->getNumClasses())
		return NULL;

	MESH_LOG("Extracting class %d\n", cls);

	unsigned int nv = m_mesh->getNumVerts();

	int *fmap = new int[nv];
	int *rmap = new int[nv];

	for (unsigned n = 0; n < nv; n++)
		fmap[n] = rmap[n] = -1;

	int start = 0;
	for (int c = 0; c < cls; c++)
		start += m_mesh->getNumTris(c);
	int size = m_mesh->getNumTris(cls);

	// map nodes
	int numv = 0;
	for (int f = 0; f < size; f++) {
		for (int m = 0; m < 3; m++) {
			int nd = m_mesh->getElemInd(start + f, m);
			if (fmap[nd] < 0) {
				fmap[nd] = numv;
				rmap[numv++] = nd;
			}
		}
	}

	MESH_LOG("Class contains %d faces, %d vertices\n", size, numv);

	TriMeshLin *msh = new TriMeshLin();

	msh->capacity(numv, size);

	const Point3 mean = m_mesh->getMean();
	for (int m = 0; m < numv; m++) {
		int n = rmap[m];
		Point3 v = m_mesh->getVertex(n) + mean;
		msh->addVertex(v);
	}

	for (int n = 0; n < size; n++) {
		unsigned int n0 = fmap[m_mesh->getElemInd(start + n, 0)];
		unsigned int n1 = fmap[m_mesh->getElemInd(start + n, 1)];
		unsigned int n2 = fmap[m_mesh->getElemInd(start + n, 2)];
		msh->addElem(n0, n1, n2);
	}

	msh->recalculateEdges();

	delete[] fmap;
	delete[] rmap;

	return msh;
}
//---------------------------------------------------------------------------
void
MeshProc::mergeVertices(double dist)
{
	SCache sc(m_mesh->getMin(), m_mesh->getSize(), 16);

	if (dist <= 0)
		dist = averageEdgeDistance() / 100;

	unsigned int nv = m_mesh->getNumVerts();

	sc.resize(nv);

	printf("Checking close vertices, eps: %g\n", dist);

	for (unsigned int v = 0; v < nv; v++) {
		const Point3 &pv = m_mesh->getVertex(v);
		Sphere3 vs(pv, dist);
		SCache::spset_t eset;

		sc.intersect(vs, eset);
		if (eset.empty()) {
			sc.addSphere(v, vs);
			continue;
		}
		
//		printf("Vertex %d close to %d vertices:\n", v, eset.size());
		SCache::spset_t::iterator it;
		for (it = eset.begin(); it != eset.end(); it++) {
			unsigned int vi = *it;
			const Point3 &pvi = m_mesh->getVertex(vi);
			double dist = (pvi - pv).length();
//			printf("  %d (%g)\n", vi, dist);
			if (dist > INT_EPS)
				continue;
			printf("Vertex %d identical to %d (%g):\n",
			       v, vi, dist);
			Edge *ed = m_mesh->getEdge(v, vi);
			if (ed != NULL)
				printf("  Edge between identical vertices!\n");

			Neighbor &nf = m_mesh->getFaceNbrs(v);
			while (nf.count()) {
				unsigned e = nf[0];
				unsigned v0 = m_mesh->getElemInd(e, 0);
				unsigned v1 = m_mesh->getElemInd(e, 1);
				unsigned v2 = m_mesh->getElemInd(e, 2);
				printf("  Elem: %d (%d %d %d)",
				       e, v0, v1, v2);
				if (v0 == v)
					v0 = vi;
				if (v1 == v)
					v1 = vi;
				if (v2 == v)
					v2 = vi;

				printf(" -> (%d %d %d)\n", v0, v1, v2);
				
				m_mesh->changeElem(e, v0, v1, v2);
				printf("  modified element %d\n", e);
			}
		}
	}
	m_mesh->recalculateEdges();
}
//---------------------------------------------------------------------------
void
MeshProc::mergeElements(double *ef)
{
	SCache sc(m_mesh->getMin(), m_mesh->getSize(), 16);

	unsigned int ne = m_mesh->getNumTris();

	sc.resize(ne);	

	double dist = averageEdgeDistance() / 100;

	printf("Checking for identical elements, eps: %g\n", dist);
	list<unsigned int> eset;
	set<unsigned int> vset;

	for (unsigned int e = 0; e < ne; e++) {
		unsigned int ia = m_mesh->getElemInd(e, 0);
		unsigned int ib = m_mesh->getElemInd(e, 1);
		unsigned int ic = m_mesh->getElemInd(e, 2);

		const Point3 &p0 = m_mesh->getVertex(ia);
		const Point3 &p1 = m_mesh->getVertex(ib);
		const Point3 &p2 = m_mesh->getVertex(ic);

		Sphere3 es((p0 + p1 + p2)/2, dist);
		SCache::spset_t iset;

		if (ef)
			ef[e] = 0;

		sc.intersect(es, iset);
		if (iset.empty()) {
			sc.addSphere(e, es);
			continue;
		}

		SCache::spset_t::iterator it;
		for (it = iset.begin(); it != iset.end(); it++) {

			unsigned int ei = *it;
			unsigned int ia2 = m_mesh->getElemInd(e, 0);
			unsigned int ib2 = m_mesh->getElemInd(e, 1);
			unsigned int ic2 = m_mesh->getElemInd(e, 2);

			if (((ia2 == ia) || (ia2 == ib) || (ia2 == ic)) &&
			    ((ib2 == ia) || (ib2 == ib) || (ib2 == ic)) &&
			    ((ic2 == ia) || (ic2 == ib) || (ic2 == ic))) {
				printf("Elements %d and %d are identical\n",
				       e, ei);

				// delete the element and all the nbrs
				vset.insert(e);
//				vset.insert(ei);
				for (int m = 0; m < 3; m++) {
					int v = m_mesh->getElemInd(e, m);
					Neighbor &n1 = m_mesh->getFaceNbrs(v);
					printf("e: %d, v: %d nb: %d\n",
					       e, v, n1.count());
//					for (int n = 0; n < n1.count(); n++)
//						vset.insert(n1[n]);

					v = m_mesh->getElemInd(ei, m);
					Neighbor &n2 = m_mesh->getFaceNbrs(v);
					printf("e: %d, v: %d nb: %d\n",
					       ei, v, n2.count());
//					for (int n = 0; n < n2.count(); n++)
//						vset.insert(n2[n]);

				}

				if (ef) {
//					ef[e]++;
					ef[ei]++;
				}
			}
		}
	}

	set<unsigned int>::iterator vi;
	for (vi = vset.begin(); vi != vset.end(); vi++) {
		unsigned int e = *vi;
		eset.push_front(e);
	}
	while(!eset.empty()) {
		unsigned int e = eset.front();;
		m_mesh->delElem(e);
		eset.pop_front();
	}

	m_mesh->recalculateEdges();
}
//---------------------------------------------------------------------------
#define SHARP_THRESH -0.98	// 168.5 deg
int
MeshProc::checkSharpEdge(
	unsigned int el, unsigned int v1, unsigned int v2,
	set<unsigned int> &eset)
{
	Edge *e = m_mesh->getEdge(v1, v2);
	int sharp = 0;

	if (e == NULL) {
		printf ("Invalid edge <%u %u> on element %u\n", v1, v2, el);
		return 0;
	}

	const Point3 &n1 = m_mesh->getFaceNormal(el);

	for (int n = 0; n < e->nelem; n++) {
		if (e->elem[n] == el)
			continue;
		
		double dot = n1.dot(m_mesh->getFaceNormal(e->elem[n]));

		if (dot < SHARP_THRESH) {
			printf("Sharp edge <%u %u> between %u and %u\n",
			       v1, v2, el, e->elem[n]);
			eset.insert(e->elem[n]);
			sharp++;
		}
	}
	return sharp;
}

//---------------------------------------------------------------------------
int
MeshProc::printSharpEdges(void)
{
	int se, rf;

	printf("Checking for sharp edges\n");

	unsigned int ne = m_mesh->getNumTris();
	se = rf = 0;
	for (unsigned int e = 0; e < ne; e++) {
		unsigned int v1 = m_mesh->getElemInd(e, 0);
		unsigned int v2 = m_mesh->getElemInd(e, 1);
		unsigned int v3 = m_mesh->getElemInd(e, 2);
		unsigned int nv[3];
		unsigned int ne[3];
		unsigned int idx = 0;
			
		set<unsigned int> eset;
		set<unsigned int>::iterator vi;

		checkSharpEdge(e, v1, v2, eset);
		checkSharpEdge(e, v1, v3, eset);
		checkSharpEdge(e, v3, v2, eset);

		if (eset.size() == 0)
			continue;

		printf ("Element %u has %lu sharp edges:",
			e, eset.size());
		se++;
		for (vi = eset.begin(); vi != eset.end(); vi++)
			printf(" %u", *vi);
		printf("\n");
	}

	return se;
}
//---------------------------------------------------------------------------
int
MeshProc::flipSharpEdges(void)
{
	int se, rf, nm = 0;

	printf("Flipping sharp edges\n");


	do {
		unsigned int ne = m_mesh->getNumTris();
		se = rf = 0;
		for (unsigned int e = 0; e < ne; e++) {
			unsigned int v1 = m_mesh->getElemInd(e, 0);
			unsigned int v2 = m_mesh->getElemInd(e, 1);
			unsigned int v3 = m_mesh->getElemInd(e, 2);
			unsigned int nv[3];
			unsigned int ne[3];
			unsigned int idx = 0;

			
			set<unsigned int> eset;
			set<unsigned int>::iterator vi;

			checkSharpEdge(e, v1, v2, eset);
			checkSharpEdge(e, v1, v3, eset);
			checkSharpEdge(e, v3, v2, eset);

			if (eset.size() == 0)
				continue;

			printf ("Element %u has %lu sharp edges:",
				e, eset.size());
			se++;
			for (vi = eset.begin(); vi != eset.end(); vi++)
				printf(" %u", *vi);
			printf("\n");

			if (eset.size() < 2)
				continue;

			if (eset.size() > 3)
				continue;

			printf("Trying to collapse edge\n");
			
			for (vi = eset.begin(); vi != eset.end(); vi++) {
				unsigned int v;
				for (int i = 0; i < 3; i++) {
					v = m_mesh->getElemInd(*vi, i);
					if (v != v1 && v != v2 && v != v3)
						break;
				}
				ne[idx] = *vi;
				nv[idx] = v;
				idx++;
			}

			if (idx == 2) {
				if (nv[0] == nv[1])
					rf = flipSharp(e, ne[0], ne[1], nv[0]);
			} else {
				if (nv[0] == nv[1]) 
					rf = flipSharp(e, ne[0], ne[1], nv[0]);
				else if (nv[0] == nv[2])
					rf = flipSharp(e, ne[0], ne[2], nv[2]);
				else if (nv[1] == nv[2])
					rf = flipSharp(e, ne[1], ne[2], nv[1]);
			}
			if (rf) {
				printf("Elements modified, restart search\n");
				nm++;
				break;
			}
		}
	} while(rf);

	printf("flipSharpEdges: %d flipped, %d remaining\n", nm, se);

	return nm;
}
//---------------------------------------------------------------------------
int
MeshProc::flipSharp(unsigned int e0, unsigned int e1, unsigned int e2, unsigned int v0)
{
	unsigned int vl[3];
	int vi = 0, vx = -1;

	for (int i = 0; i < 3; i++) {
		unsigned int v = m_mesh->getElemInd(e0, i);
		int match = 0;
		for (int j = 0; j < 3; j++) {
			if (v == m_mesh->getElemInd(e1, j)) {
				match++;
				break;
			}
		}
		for (int j = 0; j < 3; j++) {
			if (v == m_mesh->getElemInd(e2, j)) {
				match++;
				break;
			}
		}
		if (match == 1)
			vl[vi++] = v;
		if (match == 2)
			vx = v;
	}

	if (vi != 2 || vx == -1) {
		printf("flipSharp: OOPS!\n");
		return 0;
	}

	if (e1 > e2)
		m_mesh->delElem(e1);

	m_mesh->delElem(e2);

	if (e1 < e2)
		m_mesh->delElem(e1);

	m_mesh->changeElem(e0, v0, vl[0], vl[1]);

	printf("v0:%u, vl0:%u, vl1:%u, vx:%u [", v0, vl[0], vl[1], vx);

	Neighbor &nb = m_mesh->getNodeNbrs(vx);
	for (int i = 0; i < nb.count(); i++)
		printf(" %u", nb[i]);
	printf("]\n");

	m_mesh->getNodeNbrs(vx).del(v0);

	printf("Recompute ...\n");
	m_mesh->recalculateEdges();
	return 1;
}
