/*
alphaclock - transparent desktop clock
Copyright (C) 2016  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "opengl.h"
#include "image.h"

static unsigned int next_pow2(unsigned int x);

Image::Image()
{
	width = height = 0;
	tex_width = tex_height = 0;
	pixels = 0;
	texture = 0;
	own_pixels = 0;
	bpp = 24;
}

Image::~Image()
{
	destroy();
}

void Image::create(int xsz, int ysz, unsigned char *pix)
{
	destroy();

	pixels = own_pixels = new unsigned char[xsz * ysz * 3];
	if(pix) {
		memcpy(pixels, pix, xsz * ysz * 3);
	} else {
		memset(pixels, 0, xsz * ysz * 3);
	}
	width = xsz;
	height = ysz;
}

void Image::destroy()
{
	if(texture) {
		glDeleteTextures(1, &texture);
		texture = 0;
	}

	delete [] own_pixels;
	own_pixels = pixels = 0;

	width = height = tex_width = tex_height = 0;
}

bool Image::save(const char *fname) const
{
	FILE *fp = fopen(fname, "wb");
	if(!fp) {
		return false;
	}

	fprintf(fp, "P6\n%d %d\n255\n", width, height);
	unsigned char *pptr = pixels;
	for(int i=0; i<width * height; i++) {
		fputc(pptr[0], fp);
		fputc(pptr[1], fp);
		fputc(pptr[2], fp);
		pptr += bpp / 8;
	}
	fclose(fp);
	return true;
}

unsigned int Image::gen_texture()
{
	if(!pixels || !width || !height) {
		return 0;
	}

	if(!texture) {
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	} else {
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	tex_width = next_pow2(width);
	tex_height = next_pow2(height);

	if(tex_width == width && tex_height == height) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0,
				bpp == 32 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pixels);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
				bpp == 32 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pixels);
	}

	return texture;
}

static unsigned int next_pow2(unsigned int x)
{
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}
