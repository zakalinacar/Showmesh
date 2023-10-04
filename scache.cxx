//	$Id: scache.cxx,v 1.2 2008/02/17 22:46:02 canacar Exp $
/*
 * This file is part of the EMSI Tools Package developed at the
 * Brain Research Laboratory, Middle East Technical University
 * Department of Electrical and Electronics Engineering.
 *
 * Copyright (C) 2008 Zeynep Akalin Acar
 * Copyright (C) 2008 Can Erkin Acar
 * Copyright (C) 2008 Nevzat G. Gencer
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

#include "scache.h"


#define DIV_MIN	2
#define DIV_MAX 64

SCache::SCache(const Point3 &origin, const Point3 &size, int div) :
	m_div(div), m_origin(origin), m_step(size)
{
	if (m_div < DIV_MIN)
		m_div = DIV_MIN;

	if (m_div > DIV_MAX)
		m_div = DIV_MAX;

	m_step /= m_div;

	if (m_step.getX() <= 0)
		m_step.setX(1);

	if (m_step.getY() <= 0)
		m_step.setY(1);

	if (m_step.getZ() <= 0)
		m_step.setZ(1);

	m_shash.resize(m_div * m_div * m_div);
#if 0
	printf("SCache: o(%g %g %g), st(%g %g %g)\n",
	       m_origin.getX(), m_origin.getY(), m_origin.getZ(),
	       m_step.getX(), m_step.getY(), m_step.getZ());
#endif
}

// returns the grid index that contain the point
int
SCache::hashPoint(const Point3 &s) const
{
	Point3 p(s);
	gridCoord(p);

	int idx = gridIndex((int)p.getX(), (int)p.getY(), (int)p.getZ());

	return idx;
}

// maps point 'p' to discrete grid coordinates
void
SCache::gridCoord(Point3 &p) const
{
	p -= m_origin;

	double x, y, z;
	p.getCoord(x, y, z);
	
	x = floor(x / m_step.getX());
	if (x < 0)
		x = 0;
	if (x >= m_div)
		x = m_div - 1;

	y = floor(y / m_step.getY());
	if (y < 0)
		y = 0;
	if (y >= m_div)
		y = m_div - 1;

	z = floor(z / m_step.getZ());
	if (z < 0)
		z = 0;
	if (z >= m_div)
		z = m_div - 1;

	p.setCoord(x, y, z);
}

// returns the grid indices that intersect/contain the sphere
void
SCache::hashSphere(const Sphere3 &s, shset_t &shs) const
{
	double r = s.getRadius();
	Point3 rad(r, r, r);
 
	Point3 p0 = s.getCenter() - rad;
	Point3 p1 = s.getCenter() + rad;

#if 0
	printf("hashSphere: (%g %g %g / %g)\n",
		       s.getCenter().getX(), s.getCenter().getY(),
		       s.getCenter().getZ(), s.getRadius());

	printf("  p0(%g %g %g), p1(%g %g %g)\n",
	       p0.getX(), p0.getY(), p0.getZ(),
	       p1.getX(), p1.getY(), p1.getZ());
#endif
	gridCoord(p0);
	gridCoord(p1);
#if 0
	dprintf("  g0(%g %g %g), g1(%g %g %g)\n",
	       p0.getX(), p0.getY(), p0.getZ(),
	       p1.getX(), p1.getY(), p1.getZ());
#endif
	// XXX just use the bounding rectange of the sphere for now
	for (int i = (int)p0.getX(); i <= p1.getX(); i++) {
		for (int j = (int)p0.getY(); j <= p1.getY(); j++) {
			for (int k = (int)p0.getZ(); k <= p1.getZ(); k++) {
				int id = gridIndex(i, j, k);
				shs.insert(id);
			}
		}
	}
}

void
SCache::addSphere(unsigned int i, const Sphere3 &s)
{
	SCache::shset_t shs;
	SCache::shset_t::iterator it;

	hashSphere(s, shs);

	m_spheres[i] = s;
#if 0
	if (shs.empty())
		printf("Failed to add sphere: %d (%g %g %g / %g)\n", i,
		       s.getCenter().getX(), s.getCenter().getY(),
		       s.getCenter().getZ(), s.getRadius());
#endif
	for (it = shs.begin(); it != shs.end(); it++) {
		int bucket = *it;
//		printf("Adding sphere %d to bucket %d\n", i, bucket);
		(m_shash[bucket]).insert(i);
	}
}

void
SCache::removeSphere(unsigned int i)
{
	SCache::shset_t shs;
	SCache::shset_t::iterator it;

	Sphere3 s = m_spheres[i];

	hashSphere(s, shs);

	for (it = shs.begin(); it != shs.end(); it++) {
		int bucket = *it;
		m_shash[bucket].erase(i);
	}
}

void
SCache::intersect(const Sphere3 &s, spset_t &sps) const
{
	SCache::shset_t shs;
	SCache::shset_t::iterator it;

	hashSphere(s, shs);
#if 0
	if (shs.empty())
		printf("Failed hash sphere: (%g %g %g / %g)\n",
		       s.getCenter().getX(), s.getCenter().getY(),
		       s.getCenter().getZ(), s.getRadius());
#endif
	for (it = shs.begin(); it != shs.end(); it++) {
		const SCache::shset_t &spb = m_shash[*it];
		SCache::shset_t::iterator jt;
		if (spb.empty()) {
			continue;
//			printf("Empty bucket %d\n", *it);
		}
		for (jt = spb.begin(); jt != spb.end(); jt++) {
			int sid = *jt;
			const Sphere3 &se = m_spheres[sid];
#if 0
			if (se.getRadius() <= 0) {
				printf("Invalid sphere %d in bucket %d\n",
				       sid, *it);
			}
#endif
			if (se.intersect(s))
				sps.insert(sid);
		}
	}
}
