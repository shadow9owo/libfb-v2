#include <fb.h>

int main(void)
{
	FB *test = fb_init(0,0,0,0);
	if (test)
		printf("Success\n");
	else
		exit(-1);
	fb_remove(test);
	return 0;
}
