//	$Id: scache.h,v 1.1 2008/02/16 08:43:15 canacar Exp $
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

#ifndef _SCACHE_H_
#define _SCACHE_H_

#include <set>
#include <vector>
#include "sphere3.h"

using namespace std;

class SCache
{
public:
	SCache(const Point3 &origin, const Point3 &size, int div = 8);
	~SCache(void) {};

	// set of hash indices
	typedef set<int> shset_t;
	// set of sphere indices
	typedef set<unsigned int> spset_t;

	void resize(unsigned int size) {
		if (size < m_spheres.size())
			return;
		m_spheres.resize(size);
	}

	unsigned int addSphere(const Sphere3 &s) {
		unsigned int idx = m_spheres.size();
		m_spheres.resize(idx + 1);
		addSphere(idx, s);
		return idx;
	}
	void addSphere(unsigned int i, const Sphere3 &s);
	void removeSphere(unsigned int i);
	void intersect(const Sphere3 &s, spset_t &sps) const;

protected:

	// converts the grid coordinates to the grid index
	inline int gridIndex(int x, int y, int z) const {
		return (x * m_div + y) * m_div + z;
	}

	void gridCoord(Point3 &p) const;

	int hashPoint(const Point3 &s) const;
	void hashSphere(const Sphere3 &s, shset_t &shs) const;

private:
	vector<shset_t> m_shash;
	vector<Sphere3> m_spheres;

	int m_div, m_count, m_capacity;
	Point3 m_origin;
	Point3 m_step;
};
#endif
