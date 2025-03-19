#include <fb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//custom libs
#include "utils.h"
#include "types.h"
#include "config.h"

extern int RenderCallbacks(FB *framebuffer);
extern int HandleInput();

bool Isrunning = true;

int main(void)
{
    FB *test;
    if (RawCLI)
    {
        test = fb_init(GetScreenSize().X, GetScreenSize().Y, 0, 0);  // fullscreen
    }
    else
    {
        test = fb_init(0, 0, 0, 0);
    }

    if (!test)
    {
        fprintf(stderr, "Failed to initialize framebuffer\n");
        exit(EXIT_FAILURE);
    }

    if (Debug)
    {
        printf("Framebuffer initialized successfully\n");
    }

    while (Isrunning)
    {
        RenderCallbacks(test);
        HandleInput();
    }

    fb_remove(test);
    return 0;
}

int HandleInput()
{
    return 0;
}

int RenderCallbacks(FB *framebuffer)
{
    RECT *rect = fb_init_rect(100, 100, 200, 150, WHITE, framebuffer);
    if (!rect)
    {
        fprintf(stderr, "Failed to create rectangle\n");
        return -1;
    }

    while (Isrunning)
    {
        fb_render(framebuffer);
    }

    Isrunning = false;
    return 0;
}
