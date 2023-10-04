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
#ifndef _GLUTTEXT_H_
#define _GLUTTEXT_H_

#include <FL/gl.h>

class GlutText {
 public:
	enum TextAlign {LEFT, CENTER, RIGHT};

	GlutText();
	virtual ~GlutText() {}

	void color(double r = 1, double g = 1, double b = 1, double a = 1);
	void align(TextAlign a);
	void out(double x, double y, const char *fmt, ...);
	void out(double x, double y, double z, const char *fmt, ...);

	inline void epsOn(void) { m_eps = true; }
	inline void epsOff(void) { m_eps = false; }
	inline int size(void) {return m_eps_size; }

 protected:
	void adjustPos(double &x, double &y, const char *str);
	void text(const char *str);

 private:
	TextAlign m_align;
	GLfloat   m_color[4];
	int       m_font;
	const char	  *m_eps_font;
	int	  m_eps_size, m_eps_width;
	bool	  m_eps;
};

#endif
