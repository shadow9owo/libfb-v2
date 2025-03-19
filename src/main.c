#include <fb.h>
#include <stdbool.h>

//custom libs
#include "utils.h"
#include "types.h"
#include "config.h"

extern int RenderCallbacks();
extern int HandleInput();

int main(void)
{
	FB test;
	if (RawCLI)
	{
		test = fb_init(GetScreenSize().X,GetScreenSize().Y,0,0);  //fullscreen
	}else 
	{
		test = fb_init(0,0,0,0);
	}
	if (test)
	{
		if (Debug)
		{
			printf("Success\n"); //initilized successfully
		}
	}
 	else
	{
		exit(-1);
	}
	fb_remove(test);
	while (Isrunning)
	{
		 RenderCallbacks(test);
		//infinite loop
	}
	return 0;
}

int HandleInput()
{
	while (1)
	{
			
	}
	Isrunning = false;
	return 0;
}

int RenderCallbacks(FB framebuffer)
{
	FB test = framebuffer;
	RECT *rect = fb_init_rect(100, 100, 200, 150, WHITE, test);
	while (1)
	{
		fb_render(test);
	}
	fb_remove(test);
	Isrunning = false;
	return 0;
}
