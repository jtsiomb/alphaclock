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
#ifndef IMAGE_H_
#define IMAGE_H_

#include "vec3.h"

class Image {
private:
	unsigned char *own_pixels;

public:
	int width, height;
	unsigned char *pixels;
	unsigned int texture;
	int tex_width, tex_height;
	int bpp;

	Image();
	~Image();

	void create(int xsz, int ysz, unsigned char *pix = 0);
	void destroy();

	bool save(const char *fname) const;

	unsigned int gen_texture();
};

#endif	// IMAGE_H_
