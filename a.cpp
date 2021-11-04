#include <stdio.h>
#include <stdlib.h>

int main()
{
	for(int i = 0; i < 10; ++i)
		malloc(256);
	return 0;
}
