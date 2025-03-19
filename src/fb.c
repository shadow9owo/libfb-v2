#include "fb.h"
#include <math.h>

static void render_circle(SPRITE *sprite, FB *frame_buffer);
static void render_rect(SPRITE *sprite, FB *frame_buffer);
static void render_bitmap(SPRITE *sprite, FB *frame_buffer);

FB *fb_init(int width, int height, int x, int y)
{
    FB *frame_buffer = (FB *)malloc(sizeof(FB));
    if (!frame_buffer)
    {
        perror("Failed to allocate framebuffer structure");
        return NULL;
    }

    frame_buffer->dev = open("/dev/fb0", O_RDWR);
    if (frame_buffer->dev == -1)
    {
        perror("Error opening framebuffer device");
        free(frame_buffer);
        return NULL;
    }

    ioctl(frame_buffer->dev, FBIOGET_FSCREENINFO, &frame_buffer->finfo);
    ioctl(frame_buffer->dev, FBIOGET_VSCREENINFO, &frame_buffer->vinfo);

    frame_buffer->screensize = frame_buffer->vinfo.yres_virtual * frame_buffer->finfo.line_length;
    frame_buffer->buffer = (uint32_t *)mmap(0, frame_buffer->screensize, PROT_READ | PROT_WRITE, MAP_SHARED, frame_buffer->dev, 0);
    if (frame_buffer->buffer == MAP_FAILED)
    {
        perror("Error mapping framebuffer device to memory");
        close(frame_buffer->dev);
        free(frame_buffer);
        return NULL;
    }

    frame_buffer->back_buffer = (uint32_t *)malloc(frame_buffer->screensize);
    if (!frame_buffer->back_buffer)
    {
        perror("Failed to allocate back buffer");
        munmap(frame_buffer->buffer, frame_buffer->screensize);
        close(frame_buffer->dev);
        free(frame_buffer);
        return NULL;
    }

    frame_buffer->width = frame_buffer->vinfo.xres;
    frame_buffer->height = frame_buffer->vinfo.yres;
    frame_buffer->x = x;
    frame_buffer->y = y;
    frame_buffer->sprite = NULL;
    frame_buffer->sprite_count = 0;

    return frame_buffer;
}

void fb_remove(FB *frame_buffer)
{
    if (!frame_buffer) return;

    munmap(frame_buffer->buffer, frame_buffer->screensize);
    free(frame_buffer->back_buffer);
    close(frame_buffer->dev);
    free(frame_buffer);
}

SPRITE *fb_add_sprite(FB *frame_buffer)
{
    frame_buffer->sprite_count++;
    frame_buffer->sprite = (SPRITE *)realloc(frame_buffer->sprite, frame_buffer->sprite_count * sizeof(SPRITE));
    return &frame_buffer->sprite[frame_buffer->sprite_count - 1];
}

CIRCLE *fb_init_circle(int x, int y, int radius, int colour, FB *frame_buffer)
{
    CIRCLE *circle = (CIRCLE *)malloc(sizeof(CIRCLE));
    circle->x = x;
    circle->y = y;
    circle->radius = radius;
    circle->colour = colour;
    circle->parent = fb_add_sprite(frame_buffer);
    circle->parent->type = TYPE_CIRCLE;
    circle->parent->circle = circle;
    return circle;
}

RECT *fb_init_rect(int x, int y, int width, int height, int colour, FB *frame_buffer)
{
    RECT *rect = (RECT *)malloc(sizeof(RECT));
    rect->x = x;
    rect->y = y;
    rect->width = width;
    rect->height = height;
    rect->colour = colour;
    rect->parent = fb_add_sprite(frame_buffer);
    rect->parent->type = TYPE_RECT;
    rect->parent->rect = rect;
    return rect;
}

BITMAP *fb_init_bitmap(int x, int y, const char *image, FB *frame_buffer)
{
    BITMAP *bitmap = (BITMAP *)malloc(sizeof(BITMAP));
    bitmap->x = x;
    bitmap->y = y;
    bitmap->width = 0;
    bitmap->height = 0;
    bitmap->buffer = NULL;
    bitmap->parent = fb_add_sprite(frame_buffer);
    bitmap->parent->type = TYPE_BITMAP;
    bitmap->parent->b_map = bitmap;
    return bitmap;
}

void fb_render(FB *frame_buffer)
{
    memset(frame_buffer->back_buffer, 0, frame_buffer->screensize);

    for (unsigned int i = 0; i < frame_buffer->sprite_count; i++)
    {
        SPRITE *sprite = &frame_buffer->sprite[i];

        if (!sprite->visible)
            continue;

        switch (sprite->type)
        {
        case TYPE_CIRCLE:
            render_circle(sprite, frame_buffer);
            break;
        case TYPE_RECT:
            render_rect(sprite, frame_buffer);
            break;
        case TYPE_BITMAP:
            render_bitmap(sprite, frame_buffer);
            break;
        }
    }

    memcpy(frame_buffer->buffer, frame_buffer->back_buffer, frame_buffer->screensize);
}

static void put_pixel(FB *frame_buffer, int x, int y, uint32_t colour)
{
    if (x >= 0 && x < frame_buffer->width && y >= 0 && y < frame_buffer->height)
    {
        frame_buffer->back_buffer[y * (frame_buffer->finfo.line_length / 4) + x] = colour;
    }
}

static void render_circle(SPRITE *sprite, FB *frame_buffer)
{
    CIRCLE *circle = sprite->circle;
    int x0 = circle->x;
    int y0 = circle->y;
    int radius = circle->radius;
    int colour = circle->colour;

    int x = radius, y = 0;
    int radiusError = 1 - x;

    while (x >= y)
    {
        put_pixel(frame_buffer, x0 + x, y0 + y, colour);
        put_pixel(frame_buffer, x0 - x, y0 + y, colour);
        put_pixel(frame_buffer, x0 + x, y0 - y, colour);
        put_pixel(frame_buffer, x0 - x, y0 - y, colour);
        put_pixel(frame_buffer, x0 + y, y0 + x, colour);
        put_pixel(frame_buffer, x0 - y, y0 + x, colour);
        put_pixel(frame_buffer, x0 + y, y0 - x, colour);
        put_pixel(frame_buffer, x0 - y, y0 - x, colour);
        y++;

        if (radiusError < 0)
        {
            radiusError += 2 * y + 1;
        }
        else
        {
            x--;
            radiusError += 2 * (y - x) + 1;
        }
    }
}

static void render_rect(SPRITE *sprite, FB *frame_buffer)
{
    RECT *rect = sprite->rect;
    for (int y = rect->y; y < rect->y + rect->height; y++)
    {
        for (int x = rect->x; x < rect->x + rect->width; x++)
        {
            put_pixel(frame_buffer, x, y, rect->colour);
        }
    }
}

static void render_bitmap(SPRITE *sprite, FB *frame_buffer)
{
    BITMAP *bitmap = sprite->b_map;
    if (!bitmap->buffer)
        return;

    for (int y = 0; y < bitmap->height; y++)
    {
        for (int x = 0; x < bitmap->width; x++)
        {
            uint32_t colour = bitmap->buffer[y * bitmap->width + x];
            put_pixel(frame_buffer, bitmap->x + x, bitmap->y + y, colour);
        }
    }
}
