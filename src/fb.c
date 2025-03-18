#include <fb.h>

FB *fb_init(int width, int height, int x, int y)
{
    FB *fb = (FB*)malloc(sizeof(FB));
    if (!fb) {
        perror("Failed to allocate memory for framebuffer");
        return NULL;
    }

    fb->dev = open("/dev/fb0", O_RDWR);
    if (fb->dev == -1) {
        perror("Failed to open framebuffer device");
        free(fb);
        return NULL;
    }

    if (ioctl(fb->dev, FBIOGET_FSCREENINFO, &fb->finfo) == -1 || 
        ioctl(fb->dev, FBIOGET_VSCREENINFO, &fb->vinfo) == -1) {
        perror("Failed to get screen info");
        close(fb->dev);
        free(fb);
        return NULL;
    }

    fb->screensize = fb->vinfo.xres * fb->vinfo.yres * fb->vinfo.bits_per_pixel / 8;
    fb->buffer = (int*)mmap(0, fb->screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb->dev, 0);
    if (fb->buffer == MAP_FAILED) {
        perror("Failed to map framebuffer memory");
        close(fb->dev);
        free(fb);
        return NULL;
    }

    fb->back_buffer = (int*)malloc(fb->screensize);
    if (!fb->back_buffer) {
        perror("Failed to allocate back buffer");
        munmap(fb->buffer, fb->screensize);
        close(fb->dev);
        free(fb);
        return NULL;
    }

    if (fb->vinfo.bits_per_pixel != 32) {
        fprintf(stderr, "Only 32-bit color depth is supported.\n");
        munmap(fb->buffer, fb->screensize);
        free(fb->back_buffer);
        close(fb->dev);
        free(fb);
        return NULL;
    }

    if (width < 0 || height < 0 || x < 0 || y < 0) {
        fb->flags |= FULLSCREEN;
        fb->width = fb->vinfo.xres;
        fb->height = fb->vinfo.yres;
        fb->x = fb->y = 0;
    } else {
        fb->width = width;
        fb->height = height;
        fb->x = x;
        fb->y = y;
    }

    fb->sprite = NULL;
    fb->sprite_count = 0;

    return fb;
}

void fb_remove(FB *frame_buffer)
{
    if (frame_buffer) {
        munmap(frame_buffer->buffer, frame_buffer->screensize);
        free(frame_buffer->back_buffer);
        close(frame_buffer->dev);

        // Free sprites
        for (int i = 0; i < frame_buffer->sprite_count; ++i) {
            if (frame_buffer->sprite[i].type == TYPE_BITMAP) {
                free(frame_buffer->sprite[i].b_map->buffer);
            }
        }

        free(frame_buffer->sprite);
        free(frame_buffer);
    }
}

static SPRITE *fb_add_sprite(FB *frame_buffer)
{
    frame_buffer->sprite = (SPRITE*)realloc(frame_buffer->sprite, sizeof(SPRITE) * (frame_buffer->sprite_count + 1));
    if (!frame_buffer->sprite) {
        perror("Failed to realloc memory for sprites");
        return NULL;
    }
    return &frame_buffer->sprite[frame_buffer->sprite_count++];
}

CIRCLE *fb_init_circle(int x, int y, int radius, int colour, FB *frame_buffer)
{
    SPRITE *sprite = fb_add_sprite(frame_buffer);
    if (!sprite) return NULL;

    sprite->type = TYPE_CIRCLE;
    CIRCLE *circle = (CIRCLE*)malloc(sizeof(CIRCLE));
    if (!circle) {
        perror("Failed to allocate memory for circle");
        return NULL;
    }

    circle->x = x;
    circle->y = y;
    circle->radius = radius;
    circle->colour = colour;

    sprite->circle = circle;
    sprite->visible = 1;
    circle->parent = sprite;

    return circle;
}

RECT *fb_init_rect(int x, int y, int width, int height, int colour, FB *frame_buffer)
{
    SPRITE *sprite = fb_add_sprite(frame_buffer);
    if (!sprite) return NULL;

    sprite->type = TYPE_RECT;
    RECT *rect = (RECT*)malloc(sizeof(RECT));
    if (!rect) {
        perror("Failed to allocate memory for rectangle");
        return NULL;
    }

    rect->x = x;
    rect->y = y;
    rect->width = width;
    rect->height = height;
    rect->colour = colour;

    sprite->rect = rect;
    sprite->visible = 1;
    rect->parent = sprite;

    return rect;
}

BITMAP *fb_init_bitmap(int x, int y, const char *image, FB *frame_buffer)
{
    SPRITE *sprite = fb_add_sprite(frame_buffer);
    if (!sprite) return NULL;

    sprite->type = TYPE_BITMAP;
    BITMAP *bitmap = (BITMAP*)malloc(sizeof(BITMAP));
    if (!bitmap) {
        perror("Failed to allocate memory for bitmap");
        return NULL;
    }

    sprite->b_map = bitmap;
    bitmap->x = x;
    bitmap->y = y;

    FILE *fp = fopen(image, "rb");
    if (!fp) {
        perror("Failed to open image file");
        free(bitmap);
        return NULL;
    }

    char tmp[16];
    if (!fgets(tmp, sizeof(tmp), fp)) {
        fclose(fp);
        free(bitmap);
        return NULL;
    }

    if (tmp[0] != 'P' || tmp[1] != '6') {
        fprintf(stderr, "Invalid PPM format\n");
        fclose(fp);
        free(bitmap);
        return NULL;
    }

    int c = getc(fp);
    while (c == '#') {
        while (getc(fp) != '\n');
        c = getc(fp);
    }
    ungetc(c, fp);

    if (fscanf(fp, "%d %d", &bitmap->width, &bitmap->height) != 2) {
        fclose(fp);
        free(bitmap);
        return NULL;
    }

    int max_color;
    if (fscanf(fp, "%d", &max_color) != 1 || max_color != 255) {
        fclose(fp);
        free(bitmap);
        return NULL;
    }

    while (fgetc(fp) != '\n');

    bitmap->buffer = (int*)malloc(bitmap->width * bitmap->height * sizeof(int));
    if (!bitmap->buffer) {
        perror("Failed to allocate memory for bitmap pixels");
        fclose(fp);
        free(bitmap);
        return NULL;
    }

    for (int i = 0; i < bitmap->width * bitmap->height; ++i) {
        bitmap->buffer[i] = 0;
        bitmap->buffer[i] |= (c = fgetc(fp)) << RED_SHIFT;
        bitmap->buffer[i] |= (c = fgetc(fp)) << GREEN_SHIFT;
        bitmap->buffer[i] |= c;
    }

    fclose(fp);
    sprite->visible = 1;
    bitmap->parent = sprite;

    return bitmap;
}

void fb_render(FB *frame_buffer)
{
    memset(frame_buffer->back_buffer, 0, frame_buffer->screensize);

    for (int i = 0; i < frame_buffer->sprite_count; ++i) {
        if (!frame_buffer->sprite[i].visible) continue;

        switch (frame_buffer->sprite[i].type) {
        case TYPE_CIRCLE:
            render_circle(&frame_buffer->sprite[i], frame_buffer);
            break;
        case TYPE_RECT:
            render_rect(&frame_buffer->sprite[i], frame_buffer);
            break;
        case TYPE_BITMAP:
            render_bitmap(&frame_buffer->sprite[i], frame_buffer);
            break;
        default:
            fprintf(stderr, "Unknown sprite type\n");
            exit(-1);
        }
    }

    memcpy(frame_buffer->buffer, frame_buffer->back_buffer, frame_buffer->screensize);
}

static void render_circle(SPRITE *sprite, FB *frame_buffer)
{
    CIRCLE *circle = sprite->circle;
    for (int y = -circle->radius; y < circle->radius; ++y) {
        if (circle->y + y < 0 || circle->y + y >= frame_buffer->height) continue;
        for (int x = -circle->radius; x < circle->radius; ++x) {
            if (circle->x + x < 0 || circle->x + x >= frame_buffer->width) continue;
            if (x * x + y * y <= circle->radius * circle->radius) {
                frame_buffer->back_buffer[(circle->y + y) * frame_buffer->finfo.line_length + circle->x + x + frame_buffer->vinfo.xoffset] = circle->colour;
            }
        }
    }
}

static void render_rect(SPRITE *sprite, FB *frame_buffer)
{
    RECT *rect = sprite->rect;
    for (int y = 0; y < rect->height; ++y) {
        if (y + rect->y >= frame_buffer->height) break;
        for (int x = 0; x < rect->width; ++x) {
            if (x + rect->x >= frame_buffer->width) break;
            frame_buffer->back_buffer[(rect->y + y) * frame_buffer->finfo.line_length + rect->x + x + frame_buffer->vinfo.xoffset] = rect->colour;
        }
    }
}

static void render_bitmap(SPRITE *sprite, FB *frame_buffer)
{
    BITMAP *bitmap = sprite->b_map;
    for (int y = 0; y < bitmap->height; ++y) {
        if (y + bitmap->y >= frame_buffer->height) break;
        for (int x = 0; x < bitmap->width; ++x) {
            if (x + bitmap->x >= frame_buffer->width) break;
            frame_buffer->back_buffer[(bitmap->y + y) * frame_buffer->finfo.line_length + bitmap->x + x + frame_buffer->vinfo.xoffset] = bitmap->buffer[y * bitmap->width + x];
        }
    }
}
