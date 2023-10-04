//	$Id: meshbase.cxx,v 1.2 2008/01/10 05:27:22 canacar Exp $
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
#include <math.h>
//#include <arpa/inet.h>
#include <string.h>
#include "meshbase.h"

//---------------------------------------------------------------------------
// FSFile: Freesurfer file access functions as described in 
// http://www.grahamwideman.com/gw/brain/fs/surfacefileformats.htm
// All functions return 0 on success except readString that returns the
// total size of the string that would have created.
//---------------------------------------------------------------------------
int
FSFile::readI2(uint16_t &v)
{
	uint16_t var;

	if (m_fp == NULL)
		return 1;

	if (fread(&var, sizeof(var), 1, m_fp) != 1)
		return 1;
	v = ntohs(var);

	return 0;
}
//---------------------------------------------------------------------------
int
FSFile::readI3(uint32_t &v)
{
	uint32_t var = 0;
	char *vp = (char *) &var;

	if (m_fp == NULL)
		return 1;

	if (fread(vp + 1, sizeof(var) - 1, 1, m_fp) != 1)
		return 1;

	v = ntohl(var);

	return 0;
}
//---------------------------------------------------------------------------
int
FSFile::readI4(uint32_t &v)
{
	uint32_t var = 0;

	if (m_fp == NULL)
		return 1;

	if (fread(&var, sizeof(var), 1, m_fp) != 1)
		return 1;

	v = ntohl(var);

	return 0;
}
//---------------------------------------------------------------------------
int
FSFile::readF4(double &v)
{
	uint32_t var = 0;
	float fv;

	assert(sizeof(var) == sizeof(fv));

	if (m_fp == NULL)
		return 1;

	if (fread(&var, sizeof(var), 1, m_fp) != 1)
		return 1;

	var = ntohl(var);

	memcpy(&fv, &var, sizeof(fv));

	v = fv;
	return 0;
}
//---------------------------------------------------------------------------
// FS strings are terminated by '\n\n' 
// This function returns size of the source string.
// Always NULL terminates the output as long as dstsize > 0
// Replacing the final \n with \0
// It will read (and skip) the string if dst == NULL or dstsize == 0
size_t
FSFile::readString(char *dst, size_t dstsize)
{
	int i;

	if (dst == NULL)
		dstsize = 0;

	int pf = 0;
	for (i = 0; ; i++) {
		int ch = fgetc(m_fp);
		if (ch == EOF)
			return 1;
		if (ch == '\n') {
			if (pf)
				break;
			pf=1;
		} else
			pf=0;
		
		if (i < dstsize)
			dst[i] = ch;
	}

	if (i < 1)
		return 1;

	if (--i < dstsize)
		dst[i] = '\0';
	else if (dstsize > 0)
		dst[dstsize - 1] = '\0';

	return 0;
}
//---------------------------------------------------------------------------
VertexStore::VertexStore()
{
	for(int n=0; n<NUM_BUCKETS; n++)
		m_buckets[n]=new VpList();
}
//---------------------------------------------------------------------------
VertexStore::~VertexStore()
{
	for(int n=0; n<NUM_BUCKETS; n++)
		if(m_buckets[n]) delete m_buckets[n];
}
//---------------------------------------------------------------------------
int
VertexStore::addVertex(float x, float y, float z,
		       float nx,float ny,float nz)
{
	int hash=getBucket(x,y,z);
	if(hash<0 || hash>=NUM_BUCKETS)
		return -1;
	VpList *b=m_buckets[hash];
//	printf("bucket=%d list=%x\n",hash,b);
//	printf("size=%d\n",b->size());
	VpList::iterator i;

	for(i=b->begin(); i!=b->end(); i++){
		if((*i)->coord.getX()!=x) continue;
		if((*i)->coord.getY()!=y) continue;
		if((*i)->coord.getZ()!=z) continue;
		(*i)->normal+=Point3(nx,ny,nz);
		(*i)->ncount++;
		return (*i)->id;
	}
//	printf("not found\n");

	if(m_vert.size()==0){
		m_minx=m_maxx=x;
		m_miny=m_maxy=y;
		m_minz=m_maxz=z;
	}else{
		if(m_minx>x) m_minx=x;
		else if(m_maxx<x) m_maxx=x;
		if(m_miny>y) m_miny=y;
		else if(m_maxy<y) m_maxy=y;
		if(m_minz>z) m_minz=z;
		else if(m_maxz<z) m_maxz=z;
	}

	Vertex *v=new Vertex();
	v->id=numVertices();
	v->ncount=1;
	v->coord.setCoord(x,y,z);
	v->normal.setCoord(nx, ny, nz);
	b->push_back(v);
	m_vert.push_back(v);
	return v->id;

/*	
	Vertex v;
	v.id=numVertices();
	v.ncount=1;
	v.coord[0]=x;
	v.coord[1]=y;
	v.coord[2]=z;
	v.normal[0]=nx;
	v.normal[1]=ny;
	v.normal[2]=nz;
	m_vert.push_back(v);
	b->push_back(&(m_vert.back()));
	return v.id;
*/

}
//---------------------------------------------------------------------------
int
VertexStore::dumpBuckets(char *fname)
{
	FILE *bf=fopen(fname,"w");
	if(bf==0) return 1;
	for(int n=0; n<NUM_BUCKETS; n++)
		fprintf(bf,"%lu\n",(m_buckets[n])->size());
	fclose(bf);
	return 0;
}
//---------------------------------------------------------------------------
int
Neighbor::add(int nbr)
{
	if (nbr < 0)
		return -1;

	for (int n = 0; n < MAX_NEIGHBOR; n++) {
		if (m_nbrs[n] == nbr)
			return 1;
		if (m_nbrs[n] >= 0)
			continue;
		m_nbrs[n] = nbr;
		m_count++;
		return 0;
	}

	return -1;
}
//---------------------------------------------------------------------------
int
Neighbor::del(int nbr)
{
	if (nbr < 0)
		return -1;

	int dest = 0;
	int n;

	for (n = 0; n < MAX_NEIGHBOR; n++) {
		if (m_nbrs[n] == nbr)
			continue;
		if (dest != n)
			m_nbrs[dest] = m_nbrs[n];
		dest++;
	}

	if (dest == n)
		return 1;

	m_nbrs[dest] = -1;
	m_count--;
	
	return 0;
}
//---------------------------------------------------------------------------
Neighbor &
Neighbor::operator=(const Neighbor &n)
{
	int i;
	m_count = n.count();
	for (i = 0; i < m_count; i++)
		m_nbrs[i] = n[i];

	for (; i < MAX_NEIGHBOR; i++)
		m_nbrs[i] = -1;

	return *this;
}
//---------------------------------------------------------------------------
Neighbor::Neighbor(const Neighbor &n)
{
	int i;
	m_count = n.count();
	for (i = 0; i < m_count; i++)
		m_nbrs[i] = n[i];
	for (; i < MAX_NEIGHBOR; i++)
		m_nbrs[i] = -1;
}
//---------------------------------------------------------------------------
bool
Neighbor::contains(int nbr) const
{
	for (int i = 0; i < m_count; i++)
		if (m_nbrs[i] == nbr)
			return true;
	return false;
}
//---------------------------------------------------------------------------
bool
Neighbor::operator==(const Neighbor &n) const
{
	if (m_count != n.count())
		return false;

	for (int i = 0; i < m_count; i++)
		if (!n.contains(m_nbrs[i]))
			return false;
	return true;
}
//---------------------------------------------------------------------------
