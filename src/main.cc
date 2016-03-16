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
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include "app.h"

static void cleanup();
static bool create_glwin(int xsz, int ysz);
static bool handle_event(XEvent *ev);
static void set_window_title(const char *title);
static void set_no_decoration(Window win);

static int win_width, win_height;
static bool quit, redraw_pending;
static Display *dpy;
static Window win;
static GLXContext ctx;
static Atom xa_wm_proto, xa_del_window;

int main(int argc, char **argv)
{
	if(!(dpy = XOpenDisplay(0))) {
		fprintf(stderr, "failed to connect to the X server.\n");
		return 1;
	}
	xa_wm_proto = XInternAtom(dpy, "WM_PROTOCOLS", False);
	xa_del_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

	if(!create_glwin(800, 400)) {
		cleanup();
		return 1;
	}

	if(!app_init()) {
		cleanup();
		return 1;
	}

	for(;;) {
		while(XPending(dpy) || !redraw_pending) {
			XEvent ev;
			XNextEvent(dpy, &ev);
			if(!handle_event(&ev) || quit) {
				goto break_main_loop;
			}
		}

		if(redraw_pending) {
			app_draw();
			glXSwapBuffers(dpy, win);
		}
	}
break_main_loop:

	cleanup();
	return 0;
}

void app_quit()
{
	quit = true;
}

void app_redisplay()
{
	redraw_pending = true;
}

static void cleanup()
{
	if(!dpy) return;
	if(ctx) {
		glXMakeCurrent(dpy, 0, 0);
		glXDestroyContext(dpy, ctx);
	}
	if(win) {
		XDestroyWindow(dpy, win);
	}
	XCloseDisplay(dpy);
}

static bool create_glwin(int xsz, int ysz)
{
	static int glx_attr[] = {
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DOUBLEBUFFER, True,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 16,
		None
	};

	int scr = DefaultScreen(dpy);
	Window root_win = RootWindow(dpy, scr);

	GLXFBConfig *fb_configs, *fbcfg = 0;
	XVisualInfo *vis_info;
	int num_fb_configs;
	if(!(fb_configs = glXChooseFBConfig(dpy, scr, glx_attr, &num_fb_configs))) {
		fprintf(stderr, "failed to find matching GLX fbconfig\n");
		return false;
	}

	/* search for an fbconfig with depth 32 */
	for(int i=0; i<num_fb_configs; i++) {
		vis_info = glXGetVisualFromFBConfig(dpy, fb_configs[i]);
		if(!vis_info) {
			fprintf(stderr, "failed to get visual from fbconfig\n");
			XFree(fb_configs);
			return false;
		}

		if(vis_info->depth == 32) {
			fbcfg = fb_configs + i;
			break;
		}
		XFree(vis_info);
	}
	if(!fbcfg) {
		fprintf(stderr, "failed to find 32bpp visual\n");
		XFree(fb_configs);
		return false;
	}

	int rsize, gsize, bsize, asize, zsize, ssize;
	glXGetFBConfigAttrib(dpy, *fbcfg, GLX_RED_SIZE, &rsize);
	glXGetFBConfigAttrib(dpy, *fbcfg, GLX_GREEN_SIZE, &gsize);
	glXGetFBConfigAttrib(dpy, *fbcfg, GLX_BLUE_SIZE, &bsize);
	glXGetFBConfigAttrib(dpy, *fbcfg, GLX_ALPHA_SIZE, &asize);
	glXGetFBConfigAttrib(dpy, *fbcfg, GLX_DEPTH_SIZE, &zsize);
	glXGetFBConfigAttrib(dpy, *fbcfg, GLX_STENCIL_SIZE, &ssize);
	printf("got visual %lu: %d bpp (%d%d%d%d), %d zbuffer, %d stencil\n", vis_info->visualid,
			rsize + gsize + bsize + asize, rsize, gsize, bsize, asize, zsize, ssize);

	if(!(ctx = glXCreateContext(dpy, vis_info, 0, True))) {
		fprintf(stderr, "failed to create OpenGL context\n");
		XFree(vis_info);
		XFree(fb_configs);
		return false;
	}

	XSetWindowAttributes xattr;
	xattr.border_pixel = xattr.backing_pixel = BlackPixel(dpy, scr);
	xattr.colormap = XCreateColormap(dpy, root_win, vis_info->visual, AllocNone);
	unsigned int xattr_mask = CWColormap | CWBorderPixel | CWBackPixel;

	win = XCreateWindow(dpy, root_win, 0, 0, xsz, ysz, 0, vis_info->depth, InputOutput,
			vis_info->visual, xattr_mask, &xattr);
	if(!win) {
		fprintf(stderr, "failed to create window\n");
		XFree(vis_info);
		XFree(fb_configs);
		return false;
	}
	XFree(vis_info);
	XFree(fb_configs);
	XSelectInput(dpy, win, ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask);
	XMapWindow(dpy, win);

	XSetWMProtocols(dpy, win, &xa_del_window, 1);
	set_window_title("alphaclock");
	set_no_decoration(win);

	glXMakeCurrent(dpy, win, ctx);

	win_width = xsz;
	win_height = ysz;
	app_reshape(win_width, win_height);
	return true;
}

static bool handle_event(XEvent *ev)
{
	static bool mapped;

	switch(ev->type) {
	case Expose:
		if(mapped) {
			redraw_pending = true;
		}
		break;

	case MapNotify:
		mapped = true;
		redraw_pending = true;
		break;

	case UnmapNotify:
		mapped = false;
		redraw_pending = false;
		break;

	case ConfigureNotify:
		if(win_width != (int)ev->xconfigure.width || win_height != (int)ev->xconfigure.height) {
			win_width = ev->xconfigure.width;
			win_height = ev->xconfigure.height;
			app_reshape(win_width, win_height);
		}
		break;

	case KeyPress:
	case KeyRelease:
		app_keyboard(XLookupKeysym(&ev->xkey, 0) & 0xff, ev->type == KeyPress);
		break;

	case ClientMessage:
		if(ev->xclient.message_type == xa_wm_proto &&
				(Atom)ev->xclient.data.l[0] == xa_del_window) {
			return false;
		}
		break;

	default:
		break;
	}
	return true;
}

static void set_window_title(const char *title)
{
	XTextProperty text_prop;
	XStringListToTextProperty((char**)&title, 1, &text_prop);
	XSetWMName(dpy, win, &text_prop);
	XSetWMIconName(dpy, win, &text_prop);
	XFree(text_prop.value);
}

struct mwm_hints{
    uint32_t flags;
    uint32_t functions;
    uint32_t decorations;
    int32_t input_mode;
    uint32_t status;
};

#define MWM_HINTS_DEC	(1 << 1)
#define MWM_DECOR_NONE	0

static void set_no_decoration(Window win)
{
	Atom wm_hints;

	if((wm_hints = XInternAtom(dpy, "_MOTIF_WM_HINTS", True)) != None) {
		struct mwm_hints hints = {MWM_HINTS_DEC, 0, MWM_DECOR_NONE, 0, 0};
		XChangeProperty(dpy, win, wm_hints, wm_hints, 32, PropModeReplace,
				(unsigned char*)&hints, 4);
	}
}