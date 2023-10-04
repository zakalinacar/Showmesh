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

//---------------------------------------------------------------------------
#ifndef rect3H
#define rect3H
#include "point3.h"
//---------------------------------------------------------------------------

class Rect3 {
public:

    Rect3 (double x=0, double y=0, double z=0,
           double dx=1, double dy=1, double dz=1):
        m_p1(x,y,z), m_p2(x+dx, y+dy, z+dz) {init();}

    Rect3 (const Point3 &p1, const Point3 &p2):
        m_p1(p1), m_p2(p2) {init();}

    Rect3 (const Rect3 &r):
        m_p1(r.getP1()), m_p2(r.getP2()) {init();}

    inline const Point3 &getP1(void) const { return m_p1; }
    inline const Point3 &getP2(void) const { return m_p2; }
    inline Point3 getMid(void) const {return (m_p1 + m_p2) * 0.5; }
    inline Point3 getDel(void) const {return (m_p2 - m_p1); }

    inline double minX(void) const { return m_p1.getX(); }
    inline double minY(void) const { return m_p1.getY(); }
    inline double minZ(void) const { return m_p1.getZ(); }

    inline double maxX(void) const { return m_p2.getX(); }
    inline double maxY(void) const { return m_p2.getY(); }
    inline double maxZ(void) const { return m_p2.getZ(); }

    inline void setMinX(double v) {m_p1.setX(v); if (maxX()<v) m_p2.setX(v); }
    inline void setMinY(double v) {m_p1.setY(v); if (maxY()<v) m_p2.setY(v); }
    inline void setMinZ(double v) {m_p1.setZ(v); if (maxZ()<v) m_p2.setZ(v); }

    inline void setMaxX(double v) {m_p2.setX(v); if (minX()>v) m_p1.setX(v); }
    inline void setMaxY(double v) {m_p2.setY(v); if (minY()>v) m_p1.setY(v); }
    inline void setMaxZ(double v) {m_p2.setZ(v); if (minZ()>v) m_p1.setZ(v); }

    inline void assign(const Point3 &p1, const Point3 &p2) {
	    m_p1 = p1;
	    m_p2 = p2;
	    init();
    }

    inline int isInside(const Point3 &p) const {
        if (p.getX() < minX() || p.getX() > maxX() ||
            p.getY() < minY() || p.getY() > maxY() ||
            p.getZ() < minZ() || p.getZ() > maxZ())
            return 0;
        return 1;
    }



protected:
    Point3 m_p1, m_p2;

private:
    void init(void);

};

#endif
