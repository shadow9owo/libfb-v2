#include <fb.h>

FB *fb_init(int width, int height, int x, int y)
{
	FB *ret = (FB*)malloc(sizeof(FB));

	ret->dev = open("/dev/fb0", O_RDWR);
	if (ret->dev == -1)
	        goto fail;

	if (ioctl(ret->dev, FBIOGET_FSCREENINFO, &ret->finfo) == -1)
	        goto fail;

	if (ioctl(ret->dev, FBIOGET_VSCREENINFO, &ret->vinfo) == -1)
	        goto fail;

	ret->screensize = ret->vinfo.xres * ret->vinfo.yres * ret->vinfo.bits_per_pixel / 8;

        ret->buffer = (int*)mmap(0, ret->screensize, PROT_READ | PROT_WRITE, MAP_SHARED, ret->dev, 0);
	ret->back_buffer = (int*)malloc(ret->screensize);

	if ((int)(ret->buffer) == -1)
		goto fail;

	if (ret->vinfo.bits_per_pixel != 32)
		goto fail;

	ret->flags = 0;

	if (width < 0 || height < 0 || x < 0 || y < 0)
	{
		ret->flags |= FULLSCREEN;
		ret->width = ret->vinfo.xres;
		ret->height = ret->vinfo.yres;
		ret->x = ret->y = 0;
	}
	else
	{
		ret->width = width;
		ret->height = height;
		ret->x = x;
		ret->y = y;
	}

	ret->sprite = 0;
	ret->sprite_count = 0;

	return ret;
fail:
	free(ret);
	return NULL;
}

void fb_remove(FB *frame_buffer)
{
	munmap(frame_buffer->buffer, frame_buffer->screensize);
	close(frame_buffer->dev);
	free(frame_buffer->back_buffer);
	free(frame_buffer->sprite);
	free(frame_buffer);
}

static SPRITE *fb_add_sprite(FB *frame_buffer)
{
	SPRITE *ret;
        frame_buffer->sprite = (SPRITE*)realloc((void*)frame_buffer->sprite, frame_buffer->sprite_count+1);
	ret = &frame_buffer->sprite[frame_buffer->sprite_count++];
	return ret;
}

CIRCLE *fb_init_circle(int x, int y, int radius, int colour, FB *frame_buffer)
{
	CIRCLE *ret;
	SPRITE *sprite = fb_add_sprite(frame_buffer);
	sprite->type = TYPE_CIRCLE;
	ret = sprite->circle = (CIRCLE*)malloc(sizeof(CIRCLE));
        ret->x = x;
	ret->y = y;
	ret->radius = radius;
        ret->colour = colour;
	/*ret->colour.blue = colour & 0xF;
	ret->colour.green = (colour >> GREEN_SHIFT) & 0xF;
	ret->colour.red = (colour >> RED_SHIFT) & 0xF;
	ret->colour.transparency = (colour >> TRANS_SHIFT) & 0xF;*/
	return ret;
}

RECT *fb_init_rect(int x, int y, int width, int height, int colour, FB *frame_buffer)
{
	
}

void fb_render(FB *frame_buffer)
{
	//Write to back-buffer
	memset(frame_buffer->back_buffer, 0, frame_buffer->screensize);
	for (int i = 0; i < frame_buffer->sprite_count; ++i)
	{
		switch(frame_buffer->sprite[i].type)
		{
		case TYPE_CIRCLE:
		{
			CIRCLE *circle = frame_buffer->sprite[i].circle;
			for (int y = -circle->radius; y < circle->radius; ++y)
			{
				if (circle->y + y < 0)
					continue;
				if (circle->y + y >= frame_buffer->height)
					break;
				for (int x = -circle->radius; x < circle->radius; ++x)
				{
					if (circle->x + x < 0)
						continue;
					if (circle->x + x >= frame_buffer->width)
						break;
					if (x*x + y*y <= circle->radius*circle->radius)
					{
						//location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8)+(y+vinfo.yoffset) * finfo.line_length;
					        frame_buffer->back_buffer[(circle->y+y)*frame_buffer->finfo.line_length+circle->x+x+frame_buffer->vinfo.xoffset]=circle->colour;
					}
				}
			}
			break;
		}
		case TYPE_RECT:
		{
			RECT *rect = frame_buffer->sprite[i].rect;
			for (int y = 0; y < rect->height; ++y)
			{
				if (y + rect->y >= frame_buffer->height)
					break;
				for (int x = 0; x < rect->width; ++x)
				{
					if (x + rect->x >= frame_buffer->width)
						break;
				        frame_buffer->back_buffer[(rect->y+y)*frame_buffer->finfo.line_length+rect->x+x+frame_buffer->vinfo.xoffset]=rect->colour;
				}
			}
			break;
		}
		default:
			exit(-1);
			break;
		}
	}
	//Copy to front-buffer
	memcpy(frame_buffer->buffer, frame_buffer->back_buffer, frame_buffer->screensize);
}
