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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "gluttext.h"
#include "gl2ps.h"

GlutText::GlutText()
{
	m_font  = FL_COURIER;
	m_align = LEFT;
	m_eps = false;
	m_eps_font = "Courier";
	m_eps_size = 12;
	m_eps_width  = 7;

	color();
}

void
GlutText::align(GlutText::TextAlign a)
{
	if (a < LEFT || a > RIGHT)
		return;

	m_align = a;
}

void
GlutText::color(double r, double g, double b, double a)
{
	m_color[0] = r;
	m_color[1] = g;
	m_color[2] = b;
	m_color[3] = a;
}

void
GlutText::adjustPos(double &x, double &y, const char *str)
{
	double w;

	if (m_align == LEFT)
		return;

	if (m_eps)
		w = m_eps_width * strlen(str);
	else
		w = gl_width(str);

	if (m_align == CENTER)
		w /= 2;

	x -= w;
}

void
GlutText::text(const char *str)
{
	if (m_eps) {
		gl2psText(str, m_eps_font, m_eps_size);
	} else {
		gl_font(m_font, m_eps_size);
		gl_draw(str);
	}
}

void
GlutText::out(double x, double y, const char *fmt, ...)
{
	static char str[4096];
	va_list ap;
	
	va_start(ap, fmt);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	adjustPos(x, y, str);
	
	glColor4fv(m_color);
	glRasterPos2f(x, y);

	text(str);
}

void
GlutText::out(double x, double y, double z, const char *fmt, ...)
{
	static char str[4096];
	va_list ap;
	
	va_start(ap, fmt);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	adjustPos(x, y, str);
	
	glColor4fv(m_color);
	glRasterPos3f(x, y, z);

	text(str);
}
