/* $Id: meshbase.h,v 1.3 2008/02/11 07:45:19 canacar Exp $ */
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

#ifndef _MESHBASE_H_
#define _MESHBASE_H_
#include <list>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "point3.h"

#define MESH_LOG(...) fprintf(stderr, __VA_ARGS__)
#define MESH_LOG_ADD(...) fprintf(stderr, __VA_ARGS__)


#define MAX_NEIGHBOR 30
using namespace std;

class FSFile {
public:
	FSFile():m_fp(NULL){};
	FSFile(FILE *fp):m_fp(fp){};
	~FSFile(){};

	inline void setFP(FILE *fp) {m_fp = fp;}
	inline FILE *getFP(void)  {return m_fp;}

	int readI2(uint16_t &v);
	int readI3(uint32_t &v);
	int readI4(uint32_t &v);
	int readF4(double &v);

	size_t readString(char *dst, size_t dstsize);

private:
	FILE *m_fp;
};

class Neighbor{
 public:
	Neighbor(){ clear(); }
	Neighbor(const Neighbor &n);

	inline const int *getVec(void) const {return m_nbrs;}
	inline int &operator[](int i){return m_nbrs[i];}
	inline int operator[](int i) const {return m_nbrs[i];}
	inline void clear(void) {
		for(int n=0; n<MAX_NEIGHBOR; n++)
			m_nbrs[n]=-1;
		m_count = 0;
	}
	Neighbor &operator=(const Neighbor &n);
	bool operator==(const Neighbor &n) const;

	int add(int nbr);
	int del(int nbr);
	bool contains(int nbr) const;
	inline int count(void) const { return m_count; }

 private:
	int m_nbrs[MAX_NEIGHBOR];
	int m_count;
};

struct Vertex{
	Vertex(){id=-1;ncount=0;}
	Vertex(const Vertex &v){
		id=v.id;

		ncount=v.ncount;
		coord=v.coord;
		normal=v.normal;
	}
	
	Vertex &operator=(const Vertex &v){
		id=v.id;
		ncount=v.ncount;
		coord=v.coord;
		normal=v.normal;
		return *this;
	}
	int id,ncount;
	Point3 coord;
	Point3 normal;
};

#define NUM_BUCKETS 1000
class VertexStore{
 public:
	typedef list<Vertex *> VpList;
	typedef vector<Vertex *> VArray;
	
	VertexStore();
	~VertexStore();
	
	int addVertex(float x, float y, float z,
		      float nx,float ny,float nz);
	
	int dumpBuckets(char *fname);
	inline int numVertices(void){return m_vert.size();}
	
	inline Vertex *getVertex(int n){
		return (n>=0 && n<numVertices()) ?  m_vert[n]:0; }
	
	inline float minX(void) const {return m_minx;}
	inline float minY(void) const {return m_miny;}
	inline float minZ(void) const {return m_minz;}
	
	inline float maxX(void) const {return m_maxx;}
	inline float maxY(void) const {return m_maxy;}
	inline float maxZ(void) const {return m_maxz;}
	
	inline float midX(void) const {return (m_minx+m_maxx)/2;}
	inline float midY(void) const {return (m_miny+m_maxy)/2;}
	inline float midZ(void) const {return (m_minz+m_maxz)/2;}
	inline Point3 getMid(void) const {return Point3(midX(),midY(),midZ());}
	
	inline float delX(void) const {return m_maxx-m_minx;}
	inline float delY(void) const {return m_maxy-m_miny;}
	inline float delZ(void) const {return m_maxz-m_minz;}
	
 private:
	inline int conv(double x){
		if(x==0) return 0;
		double t=log10(fabs(x));
		return (int) floor(pow(10,t-floor(t))-0.5); }
	inline double convf(double x){
		if(x==0) return 0;
		double t=log10(fabs(x));
		return t-floor(t);
	}
	inline int getBucket(float x, float y, float z){
		return (int) floor((convf(x*x+y*y+z*z)*1000));
	}
	
	VpList *m_buckets[NUM_BUCKETS];
	VArray m_vert;
	
	float m_minx, m_miny, m_minz;
	float m_maxx, m_maxy, m_maxz;
};

#define MAX_EDGE_ELEM 10
struct Edge{
	Edge(unsigned n1, unsigned n2):
		node1(n1),node2(n2),nelem(0),next(0){}
	unsigned node1;
	unsigned node2;
	unsigned store;	// temporary storage
	int nelem;
	unsigned elem[MAX_EDGE_ELEM];
	Edge *next;
};

#endif
