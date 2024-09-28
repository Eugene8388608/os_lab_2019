#include "revert_string.h"
#include <string.h>

void RevertString(char *str)
{
	for (char *l = str, *r = str + strlen(str) - 1; l < r; l++, r--)
	{
		*l ^= *r;
		*r ^= *l;
		*l ^= *r;
	}
}

