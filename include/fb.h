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
#define GREEN_SHIFT 8
#define RED_SHIFT 16
#define TRANS_SHIFT 24

typedef struct _pixel
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char transparency;
} PIXEL;

#define TYPE_CIRCLE 0
#define TYPE_RECT 1
#define TYPE_BITMAP 2


#define RED 0x00FF0000
#define GREEN 0x0000FF00
#define BLUE 0x000000FF
#define YELLOW 0x00FFFF00
#define ORANGE 0x00FFA500
#define WHITE 0x00FFFFFF
#define BLACK 0x00000000
#define CYAN 0x0000FFFF
#define PINK 0x00FF00FF

struct _sprite;

typedef struct _circle
{
	int x, y; //Center
	int radius;
	//PIXEL colour;
	int colour;
	struct _sprite *parent;
} CIRCLE;

typedef struct _rect
{
	int x, y; //Top left
	int width, height;
	//PIXEL colour;
	int colour;
	struct _sprite *parent;
} RECT;


typedef struct _bitmap
{
	int x, y;
	int width, height;
	int *buffer;
	struct _sprite *parent;
} BITMAP;

typedef struct _sprite
{
	char visible;
	int type;
	union
	{
		CIRCLE *circle;
		RECT *rect;
		BITMAP *b_map;
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
BITMAP *fb_init_bitmap(int x, int y, char *image /*.ppm format*/, FB *frame_buffer);

void fb_render(FB *frame_buffer);

#endif
