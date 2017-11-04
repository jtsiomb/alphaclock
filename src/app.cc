/*
alphaclock - transparent desktop clock
Copyright (C) 2016-2017  John Tsiombikas <nuclear@member.fsf.org>

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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <GL/gl.h>
#include <unistd.h>
#include <sys/time.h>
#include <drawtext.h>
#include "app.h"
#include "psys.h"

#include "pimg.h"

#ifndef APP_NAME
#define APP_NAME "alphaclock"
#endif
#ifndef PREFIX
#define PREFIX "/usr/local"
#endif

static PSysParam ppflame;

static ParticleSystem psys;
static Image *pimg, *time_image;

static dtx_font *font;

static unsigned long get_msec();
static const char *find_data_file(const char *fname);


bool app_init()
{
	glClearColor(0, 0, 0, 0);

	time_image = new Image;
	time_image->width = 256;
	time_image->height = 128;
	time_image->bpp = 32;
	time_image->pixels = new unsigned char[time_image->width * time_image->height * 4];
	memset(time_image->pixels, 0, time_image->width * time_image->height * 4);


	if(!(font = dtx_open_font(find_data_file("urw_bookman.type1"), 55))) {
		fprintf(stderr, "failed to load font\n");
		return false;
	}
	dtx_target_raster(time_image->pixels, time_image->width, time_image->height);
	dtx_set(DTX_RASTER_THRESHOLD, 128);
	dtx_color(1, 1, 1, 1);

	pimg = new Image;
	pimg->pixels = (unsigned char*)img_particle.pixel_data;
	pimg->width = img_particle.width;
	pimg->height = img_particle.height;
	pimg->bpp = 32;

	// flame parameters
	ppflame.life = 0.45;
	ppflame.life_range = 0.25;
	ppflame.size = 0.12;
	ppflame.size_range = 0.01;
	ppflame.spawn_rate = 8000;
	ppflame.gravity = Vec3(0, 1.5, 0);
	ppflame.pimg = pimg;
	ppflame.spawn_map = time_image;

	ppflame.pcolor_start = Vec3(1.0, 0.7, 0.3) * 0.5;
	ppflame.pcolor_mid = Vec3(1.0, 0.25, 0.15) * 0.5;
	ppflame.pcolor_end = Vec3(0.1, 0.075, 0.02);
	//ppflame.pcolor_mid = lerp(ppflame.pcolor_start, ppflame.pcolor_end, 0.6);

	float alpha_scale = 0.8;
	ppflame.palpha_start = 1.0 * alpha_scale;
	ppflame.palpha_mid = 0.5 * alpha_scale;
	ppflame.palpha_end = 0.05 * alpha_scale;

	ppflame.pscale_start = 1.75;
	ppflame.pscale_mid = 2.0;
	ppflame.pscale_end = 3.5;

	psys.pp = ppflame;
	return true;
}

void app_cleanup()
{
	delete pimg;
}

void app_draw()
{
	static unsigned long prev_msec;
	unsigned long msec = get_msec();
	float dt = (msec - prev_msec) / 1000.0;
	prev_msec = msec;

	time_t t = time(0);
	struct tm *tm = localtime(&t);
	char buf[64];
	sprintf(buf, "%2d:%02d.%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);

	memset(time_image->pixels, 0, time_image->width * time_image->height * 4);
	dtx_position(0, dtx_line_height());
	dtx_string(buf);
	//dtx_string("88:88.88");
	psys.reset_spawnmap();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, -0.1, 0);
	glScalef(0.9, 0.9, 0.9);

	psys.update(dt);
	psys.draw();
}

void app_reshape(int x, int y)
{
	float aspect = (float)x / (float)y;

	glViewport(0, 0, x, y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0, 1.0 * aspect, 1.0);
}

void app_keyboard(int key, bool press)
{
	if(press) {
		switch(key) {
		case 27:
			exit(0);

		case 'f':
		case 'F':
			app_fullscreen_toggle();
			break;
		}
	}
}

void app_mouse_button(int bn, bool press, int x, int y)
{
}

void app_mouse_motion(int x, int y)
{
}

static unsigned long get_msec()
{
	static struct timeval tv0;
	struct timeval tv;

	gettimeofday(&tv, 0);
	if(tv0.tv_sec == 0 && tv0.tv_usec == 0) {
		tv0 = tv;
		return 0;
	}

	return (tv.tv_sec - tv0.tv_sec) * 1000 + (tv.tv_usec - tv0.tv_usec) / 1000;
}

static const char *find_data_file(const char *fname)
{
	static char buf[2048];
	const char *dirs[] = {
		PREFIX "/share/" APP_NAME,
		"/usr/local/share/" APP_NAME,
		"/usr/share/" APP_NAME,
		"data",
		0
	};

	for(int i=0; dirs[i]; i++) {
		FILE *fp;
		sprintf(buf, "%s/%s", dirs[i], fname);
		if((fp = fopen(buf, "rb"))) {
			fclose(fp);
			return buf;
		}
	}
	return fname;
}
