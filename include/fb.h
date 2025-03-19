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
#include <stdint.h>

#define FULLSCREEN 1

#define BLUE_SHIFT 0
#define GREEN_SHIFT 8
#define RED_SHIFT 16
#define TRANS_SHIFT 24

typedef struct _pixel
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t transparency;
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
    int x, y;
    int radius;
    int colour;
    struct _sprite *parent;
} CIRCLE;

typedef struct _rect
{
    int x, y;
    int width, height;
    int colour;
    struct _sprite *parent;
} RECT;

typedef struct _bitmap
{
    int x, y;
    int width, height;
    uint32_t *buffer;
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
    uint32_t *buffer;
    uint32_t *back_buffer;
    long screensize;
    int width, height, x, y;
    unsigned char flags;
    SPRITE *sprite;
    unsigned int sprite_count;
} FB;

FB *fb_init(int width, int height, int x, int y);
void fb_remove(FB *frame_buffer);

SPRITE *fb_add_sprite(FB *frame_buffer);

CIRCLE *fb_init_circle(int x, int y, int radius, int colour, FB *frame_buffer);
RECT *fb_init_rect(int x, int y, int width, int height, int colour, FB *frame_buffer);
BITMAP *fb_init_bitmap(int x, int y, const char *image, FB *frame_buffer);

void fb_render(FB *frame_buffer);

#endif
