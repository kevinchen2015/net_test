#include"utility.h"
#include"header.h"

#if defined(PLATFORM_WINDOWS)
#pragma comment(lib ,"libuv.lib")
#endif

void* malloc_buff(size_t size)
{
	return (char*)malloc(size);
}

void  free_buff(void* buff)
{
	if(buff)
		free(buff);
}

