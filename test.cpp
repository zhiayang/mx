#include <stdio.h>
#include <stdint.h>
#define INDEX(addr) ((((uintptr_t)(addr)) >> 39) & 0x1FF)

int main()
{
	uint64_t x = 0xFFFFFF1000000000;
	printf("%llx -> %ld\n", x, INDEX(x));
}
