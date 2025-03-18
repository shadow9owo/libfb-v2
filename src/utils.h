#pragma once

//source https://stackoverflow.com/questions/16455024/how-can-i-get-screen-resolution-in-c-operating-system-qnx-or-linux

#include <stdlib.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include "types.h"
#include "config.h"

Vector2I GetScreenSize()
{
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;
    int x = 0, y = 0;
    long int location = 0;

    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        if (Debug)
        {
        perror("Error: cannot open framebuffer device");
        }
        exit(-1);
    }

    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
         if (Debug)
         {
            perror("Error reading fixed information");
         }
         exit(-1);
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
      if (Debug)
      {
          perror("Error reading variable information");
      }
      exit(-1);
    }
  
    Vector2I a;
    a.X = vinfo.xres;
    a.Y = vinfo.yres;
    return a;
}
