#include "swap.h"

void Swap(char *left, char *right)
{
	*left ^= *right;
	*right ^= *left;
	*left ^= *right;
}
