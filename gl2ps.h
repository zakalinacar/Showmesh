/*
 * GL2PS, an OpenGL to PostScript Printing Library
 * Copyright (C) 1999-2002  Christophe Geuzaine
 *
 * $Id: gl2ps.h,v 1.3 2003/03/21 12:26:47 canacar Exp $
 *
 * E-mail: geuz@geuz.org
 * URL: http://www.geuz.org/gl2ps/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __GL2PS_H__
#define __GL2PS_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* To generate a Windows dll, you have to define GL2PSDLL at compile
   time */

#ifdef WIN32
#include <windows.h>
#ifdef GL2PSDLL
#ifdef GL2PSDLL_EXPORTS
#define GL2PSDLL_API __declspec(dllexport)
#else
#define GL2PSDLL_API __declspec(dllimport)
#endif /* GL2PSDLL_EXPORTS */
#else
#define GL2PSDLL_API
#endif /* GL2PSDLL */
#else
#define GL2PSDLL_API
#endif /* WIN32 */

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif /* __APPLE__ */


#define GL2PS_VERSION                    0.71
#define GL2PS_NONE                       0

/* Output file format */

#define GL2PS_PS                         1
#define GL2PS_EPS                        2
#define GL2PS_TEX                        3

/* Sorting algorithms */

#define GL2PS_NO_SORT                    1
#define GL2PS_SIMPLE_SORT                2
#define GL2PS_BSP_SORT                   3

/* Options for gl2psBeginPage */

#define GL2PS_DRAW_BACKGROUND            (1<<0)
#define GL2PS_SIMPLE_LINE_OFFSET         (1<<1)
#define GL2PS_SILENT                     (1<<2)
#define GL2PS_BEST_ROOT                  (1<<3)
#define GL2PS_OCCLUSION_CULL             (1<<4)
#define GL2PS_NO_TEXT                    (1<<5)
#define GL2PS_LANDSCAPE                  (1<<6)
#define GL2PS_NO_PS3_SHADING             (1<<7)
#define GL2PS_GRAY                       (1<<8)

/* Arguments for gl2psEnable/gl2psDisable */

#define GL2PS_POLYGON_OFFSET_FILL        1
#define GL2PS_POLYGON_BOUNDARY           2
#define GL2PS_LINE_STIPPLE               3

/* Magic numbers */

#define GL2PS_EPSILON                    5.e-3
#define GL2PS_DEPTH_FACT                 1000.0
#define GL2PS_SIMPLE_OFFSET              0.05
#define GL2PS_SIMPLE_OFFSET_LARGE        1.0
#define GL2PS_ZERO(arg)                  (fabs(arg)<1.e-20)
/*#define GL2PS_ZERO(arg)                ((arg)==0.0)*/

/* Message levels */

#define GL2PS_INFO                       1
#define GL2PS_WARNING                    2
#define GL2PS_ERROR                      3

/* Error codes */

#define GL2PS_SUCCESS                    0
#define GL2PS_NO_FEEDBACK               -1
#define GL2PS_OVERFLOW                  -2

/* Primitive types */

#define GL2PS_TEXT                       1
#define GL2PS_POINT                      2
#define GL2PS_LINE                       3
#define GL2PS_QUADRANGLE                 4
#define GL2PS_TRIANGLE                   5

/* BSP tree primitive comparison */

#define GL2PS_COINCIDENT                 1
#define GL2PS_IN_FRONT_OF                2
#define GL2PS_IN_BACK_OF                 3
#define GL2PS_SPANNING                   4

/* 2D BSP tree primitive comparison */

#define GL2PS_POINT_COINCIDENT           0
#define GL2PS_POINT_INFRONT              1
#define GL2PS_POINT_BACK                 2

/* Pass through options */

#define GL2PS_BEGIN_POLYGON_OFFSET_FILL  1
#define GL2PS_END_POLYGON_OFFSET_FILL    2
#define GL2PS_BEGIN_POLYGON_BOUNDARY     3
#define GL2PS_END_POLYGON_BOUNDARY       4
#define GL2PS_BEGIN_LINE_STIPPLE         5
#define GL2PS_END_LINE_STIPPLE           6
#define GL2PS_SET_POINT_SIZE             7
#define GL2PS_SET_LINE_WIDTH             8

typedef GLfloat GL2PSrgba[4];
typedef GLfloat GL2PSxyz[3];
typedef GLfloat GL2PSplane[4];

typedef struct _GL2PSbsptree2d GL2PSbsptree2d;

struct _GL2PSbsptree2d {
  GL2PSplane plane;
  GL2PSbsptree2d *front, *back;
};

typedef struct {
  GLint nmax, size, incr, n;
  char *array;
} GL2PSlist;

typedef struct _GL2PSbsptree GL2PSbsptree;

struct _GL2PSbsptree {
  GL2PSplane plane;
  GL2PSlist *primitives;
  GL2PSbsptree *front, *back;
};

typedef struct {
  GL2PSxyz xyz;
  GL2PSrgba rgba;
} GL2PSvertex;

typedef struct {
  GLshort fontsize;
  char *str, *fontname;
} GL2PSstring;

typedef struct {
  GLshort type, numverts;
  char boundary, dash, culled;
  GLfloat width, depth;
  GL2PSvertex *verts;
  GL2PSstring *text;
} GL2PSprimitive;

typedef struct {
  GLint format, sort, options, colorsize, colormode, buffersize, maxbestroot;
  const char *title, *producer, *filename;
  GLboolean shade, boundary;
  GLfloat *feedback, offset[2];
  GL2PSrgba *colormap, lastrgba, threshold;
  float lastlinewidth;
  GL2PSlist *primitives;
  GL2PSbsptree2d *image;
  FILE *stream;
} GL2PScontext;


/* public functions */

#ifdef __cplusplus
extern "C" {
#endif

GL2PSDLL_API void  gl2psBeginPage(const char *title, const char *producer, 
				  GLint format, GLint sort, GLint options, 
				  GLint colormode, GLint colorsize, 
				  GL2PSrgba *colormap, GLint buffersize, 
				  FILE *stream, const char *filename);
GL2PSDLL_API GLint gl2psEndPage(void);
GL2PSDLL_API void  gl2psText(const char *str, const char *fontname, GLint size);
GL2PSDLL_API void  gl2psEnable(GLint mode);
GL2PSDLL_API void  gl2psDisable(GLint mode);
GL2PSDLL_API void  gl2psPointSize(GLfloat value);
GL2PSDLL_API void  gl2psLineWidth(GLfloat value);
GL2PSDLL_API void  gl2psNumShadeColors(GLint nr, GLint ng, GLint nb);

#ifdef __cplusplus
};
#endif

#endif /* __GL2PS_H__ */
