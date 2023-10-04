//	$Id: meshquad.cxx,v 1.1.1.1 2007/12/30 04:44:24 canacar Exp $
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
#include <math.h>
#include "meshquad.h"
#include <GL/gl.h>

//---------------------------------------------------------------------------
TriMeshQuad::TriMeshQuad(void)
{

}
//---------------------------------------------------------------------------
TriMeshQuad::~TriMeshQuad()
{
}
//---------------------------------------------------------------------------
int
TriMeshQuad::loadMesh(char *name)
{
}
//---------------------------------------------------------------------------
int
TriMeshQuad::save(char *name)
{
	fprintf(stderr, "Saving Not implemented for quad meshes\n");
	return 1;
}
//---------------------------------------------------------------------------
void
TriMeshQuad::smooth(int n)
{
	fprintf(stderr, "Smoothing Not implemented for quad meshes\n");
	return;
}
//---------------------------------------------------------------------------
void
TriMeshQuad::render(int cls, bool interp, int quad)
{
}
//---------------------------------------------------------------------------
void
TriMeshQuad::calcLimits(void)
{
}
//---------------------------------------------------------------------------
void
TriMeshQuad::calcFaceNorm(void)
{
}
//---------------------------------------------------------------------------
void
TriMeshQuad::calcVertNorm(void)
{
}
//---------------------------------------------------------------------------
