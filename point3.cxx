/* $Id: point3.cc,v 1.3 2008/01/28 07:40:59 canacar Exp $ */
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
#include "point3.h"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
Point3 operator * (double s, const Point3 &p) {
	Point3 P(p);
	P*=s;
	return P;
}
//---------------------------------------------------------------------------
Point3 operator * (const Point3 &p, double s) {
	Point3 P(p);
	P*=s;
	return P;
}
//---------------------------------------------------------------------------
Point3 operator / (const Point3 &p, double s) {
	Point3 P(p);
	P/=s;
	return P;
}
//---------------------------------------------------------------------------
Point3 operator + (const Point3 &p1, const Point3 &p2) {
	Point3 P(p1);
	P+=p2;
	return P;
}
//---------------------------------------------------------------------------
Point3 operator - (const Point3 &p1, const Point3 &p2) {
	Point3 P(p1);
	P-=p2;
	return P;
}
//---------------------------------------------------------------------------
Point3 operator - (const Point3 &p) {
	Point3 P;
	P-=p;
	return P;
}
//---------------------------------------------------------------------------
Point3 Cross(const Point3 &p1, const Point3 &p2)
{
	Point3 P;
	P.setX((p1.getY() * p2.getZ()) - (p1.getZ() * p2.getY()));
	P.setY((p1.getZ() * p2.getX()) - (p1.getX() * p2.getZ()));
	P.setZ((p1.getX() * p2.getY()) - (p1.getY() * p2.getX()));
	return P;
}
//---------------------------------------------------------------------------
Point3 Mult(const Point3 &p1, const Point3 &p2)
{
	Point3 P;
	P.setX((p1.getX() * p2.getX()));
	P.setY((p1.getY() * p2.getY()));
	P.setZ((p1.getZ() * p2.getZ()));
	return P;
}
//---------------------------------------------------------------------------
Point3 Div(const Point3 &p1, const Point3 &p2)
{
	Point3 P;
	P.setX((p1.getX() / p2.getX()));
	P.setY((p1.getY() / p2.getY()));
	P.setZ((p1.getZ() / p2.getZ()));
	return P;
}
//---------------------------------------------------------------------------
Point3 Sqrt(const Point3 &p1)
{
	Point3 P;
	P.setX(sqrt(p1.getX()));
	P.setY(sqrt(p1.getY()));
	P.setZ(sqrt(p1.getZ()));
	return P;
}
//---------------------------------------------------------------------------

