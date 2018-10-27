#ifndef __FB_H
#define __FB_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>

#define FULLSCREEN 1

#define BLUE_SHIFT 0
#define GREEN_SHIFT 1
#define RED_SHIFT 2
#define TRANS_SHIFT 3

typedef struct _pixel
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char transparency;
} PIXEL;

#define TYPE_CIRCLE 0
#define TYPE_RECT 1

typedef struct _circle
{
	int x, y; //Center
	int radius;
	//PIXEL colour;
	int colour;
} CIRCLE;

typedef struct _rect
{
	int x, y; //Top left
	int width, height;
	//PIXEL colour;
	int colour;
} RECT;

typedef struct _sprite
{
	int type;
	union
	{
		CIRCLE *circle;
		RECT *rect;
	};
} SPRITE;

typedef struct _fb
{
	int dev;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
        //PIXEL *buffer;
	int *buffer;
	//PIXEL *back_buffer; //For double-buffering
	int *back_buffer;
	long screensize;
	int width, height, x, y;
	unsigned char flags;
	SPRITE *sprite; //stack
	unsigned int sprite_count;
} FB;

FB *fb_init(int width, int height, int x, int y);
void fb_remove(FB *frame_buffer);

static SPRITE *fb_add_sprite(FB *frame_buffer);

CIRCLE *fb_init_circle(int x, int y, int radius, int colour, FB *frame_buffer);
RECT *fb_init_rect(int x, int y, int width, int height, int colour, FB *frame_buffer);

#endif
