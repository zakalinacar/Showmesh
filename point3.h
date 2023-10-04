/* $Id: point3.h,v 1.3 2008/11/24 03:48:13 canacar Exp $ */
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
#ifndef point3H
#define point3H
#include <math.h>
//---------------------------------------------------------------------------

class Point3{
 public:
	Point3(double x=0, double y=0, double z=0) {
		m_x = x;
		m_y = y;
		m_z = z;
	}

	Point3(const Point3 &p) {
		setCoord(p);
	}

	~Point3() {}

	Point3(const double *p) {
		m_x = p[0];
		m_y = p[1];
		m_z = p[2];
	}

	inline double getX(void) const {return m_x;}
	inline double getY(void) const {return m_y;}
	inline double getZ(void) const {return m_z;}

	inline void getCoord(double &x, double &y, double &z) const {
		x = m_x;
		y = m_y;
		z = m_z;
	}

	inline void setX(double x) {m_x = x;}
	inline void setY(double y) {m_y = y;}
	inline void setZ(double z) {m_z = z;}

	inline double &X(void) {return m_x;}
	inline double &Y(void) {return m_y;}
	inline double &Z(void) {return m_z;}

	inline void clear(void) {
		m_x = m_y = m_z = 0;
	}

	inline void setCoord(double x=0, double y=0, double z=0) {
		m_x = x;
		m_y = y;
		m_z = z;
	}

	inline void setCoord(const Point3 &p) {
		p.getCoord(m_x, m_y, m_z);
	}

	inline double length2(void) const {
		return m_x * m_x + m_y * m_y + m_z * m_z;
	}

	inline double length(void) const {
		return sqrt(length2());
	}

	inline double dot(const Point3 &p) const {
		return m_x * p.getX() + m_y * p.getY() + m_z * p.getZ();
	}

	inline Point3 &Mul (const Point3 &p) {
		m_x *= p.getX();
		m_y *= p.getY();
		m_z *= p.getZ();
		return *this;
	}

	inline Point3 &Div (const Point3 &p) {
		m_x /= p.getX();
		m_y /= p.getY();
		m_z /= p.getZ();
		return *this;
	}

	inline Point3 &operator = (const Point3& p) {
		setCoord(p);
		return *this;
	}

	inline Point3 &operator += (const Point3& p) {
		m_x += p.getX();
		m_y += p.getY();
		m_z += p.getZ();
		return *this;
	}

	inline Point3 &operator -= (const Point3& p) {
		m_x -= p.getX();
		m_y -= p.getY();
		m_z -= p.getZ();
		return *this;
	}

	inline Point3 &operator *= (const double s) {
		m_x *= s;
		m_y *= s;
		m_z *= s;
		return *this;
	}

	inline Point3 &operator /= (const double s) {
		m_x /= s;
		m_y /= s;
		m_z /= s;
		return *this;
	}

	inline bool operator == (const Point3 &p) const {
		return (m_x == p.getX() &&
			m_y == p.getY() &&
			m_z == p.getZ());
	}

	inline bool operator < (const Point3 &p) const {
		return (m_x < p.getX() ||
			(m_x == p.getX() &&
			 (m_y < p.getY() ||
			  (m_y == p.getY() && m_z < p.getZ()))));

	}

	inline Point3 &normalize(void) {
		double d = length();
		m_x /= d;
		m_y /= d;
		m_z /= d;
		return *this;
	}

	inline Point3 &setCross(const Point3 &p1, const Point3 &p2) {
		m_x=(p1.getY() * p2.getZ()) - (p1.getZ() * p2.getY());
		m_y=(p1.getZ() * p2.getX()) - (p1.getX() * p2.getZ());
		m_z=(p1.getX() * p2.getY()) - (p1.getY() * p2.getX());
		return *this;
	}

 protected:
	double m_x, m_y, m_z;

};

Point3 operator * (double s, const Point3 &p);
Point3 operator * (const Point3 &p, double s);
Point3 operator / (const Point3 &p, double s);
Point3 operator + (const Point3 &p1, const Point3 &p2);
Point3 operator - (const Point3 &p1, const Point3 &p2);
Point3 operator - (const Point3 &p);
Point3 Cross(const Point3 &p1, const Point3 &p2);
Point3 Mult(const Point3 &p1, const Point3 &p2);
Point3 Div(const Point3 &p1, const Point3 &p2);
Point3 Sqrt(const Point3 &p1);

#endif
