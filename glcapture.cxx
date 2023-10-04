// $Id: glcapture.cxx,v 1.1.1.1 2007/12/30 04:44:24 canacar Exp $
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
#include "glcapture.h"
#include <limits.h>
#include <png.h>
#include <string.h>

#include <GL/gl.h>

// define pixel size correctly for a given type & format
#define PIXEL_FORMAT GL_RGBA
#define PIXEL_TYPE GL_UNSIGNED_BYTE
#define PIXEL_SIZE 4


GLCapture::GLCapture()
{
	m_name = NULL;
	m_seq = 0;
	m_x=m_y=m_w=m_h=0;
	m_stream = NULL;
	m_buf = NULL;
	m_buflen = 0;
}

GLCapture::~GLCapture()
{
	if (m_buf)
		delete[] m_buf;
	if (m_name)
		delete[] m_name;

	if (m_stream) {
		m_stream = 0;
		fclose(m_stream);
	}

}

int
GLCapture::setWindow(int x, int y, int w, int h)
{
	if (x < 0 || y < 0 || w < 0 || h < 0)
		return 1;

	int len = w * h * PIXEL_SIZE;
	if (len < 0) return 1;

	m_w = w;
	m_h = h;
	m_x = x;
	m_y = y;

	if (len > m_buflen) {
		delete[] m_buf;
		m_buf = new unsigned char[len];
		m_buflen = len;
	}


	return 0;
}

int
GLCapture::snapshot(const char *fn)
{
	FILE *tmp = m_stream;

	if (m_buf == NULL || fn == NULL)
		return 1;
	
	m_stream = fopen(fn, "wb");

	if (m_stream == NULL) {
		m_stream = tmp;
		return 1;
	}
	
	glReadPixels(m_x, m_y, m_w, m_h, PIXEL_FORMAT, PIXEL_TYPE, m_buf);

	int ret = 0;
	if (writePNG()) {
		printf("Error writing PNG file\n");
		ret=1;
	}

	fclose(m_stream);
	m_stream = tmp;
	return ret;
}

int
GLCapture::captureFrame()
{
	if (beginCapture())
		return 1;
	if (m_stream == NULL)
		return 1;
	
	if (m_buf == NULL) return 1;
	
	glReadPixels(m_x, m_y, m_w, m_h, PIXEL_FORMAT, PIXEL_TYPE, m_buf);

	if (endCapture())
		return 1;
	return 0;
}

int
GLCapture::newCapture(const char *name)
{
	if (name == 0) return 1;
	int len = strlen(name);
	if (len < 0 || len > PATH_MAX)
		return 1;
	if (m_name)
		delete[] m_name;
	m_name = new char[len+1];
	memcpy(m_name, name, len + 1);

	m_seq = 1;

	return 0;
}

int
GLCapture::beginCapture(void)
{
	static char fn[PATH_MAX];
	if (m_name == NULL) return 1;

	snprintf(fn, PATH_MAX, "%s%03d.png", m_name, m_seq);
	if (m_stream)
		fclose(m_stream);
	m_stream = fopen(fn, "wb");

	if (m_stream == NULL)
		return 1;

	return 0;
}

int
GLCapture::endCapture(void)
{

	if (m_stream == NULL) return 1;
	if (m_buf == NULL) return 1;

/*
  unsigned len = m_w * m_h * PIXEL_SIZE;
  
  if (fwrite(m_buf, 1, len, m_stream) != len) {
  perror("Error writing stream");
  return 1;
  }
*/
	if (writePNG()) {
		printf("Error writing PNG file\n");
		return 1;
	}

	fclose(m_stream);
	m_stream = 0;
	m_seq ++;
	return 0;
}


int
GLCapture::writePNG(void)
{
	if (m_stream == NULL || m_buf == NULL)
		return 1;	
	
	png_structp png_ptr = png_create_write_struct
		(PNG_LIBPNG_VER_STRING, NULL,
		 NULL, NULL);

	if (!png_ptr)
		return (1);
	
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return (1);
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return (1);
	}

	png_init_io(png_ptr, m_stream);

	png_set_IHDR(png_ptr, info_ptr, m_w, m_h,
		     8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
		     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);

	png_byte **row_pointers = new png_byte*[m_h];
	unsigned char *pos = m_buf;
	for (int n=m_h-1; n>=0; n--) {
		row_pointers[n] = pos;
		pos += m_w * PIXEL_SIZE;
	}

	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	delete[] row_pointers;

	return 0;
}


