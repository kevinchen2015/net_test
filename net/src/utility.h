#ifndef _UTILITY_H
#define _UTILITY_H


#include <uv.h>

typedef unsigned int conv_t;
typedef void(*on_recv_data)(char* buff, unsigned int size, void* conn);
typedef void(*on_connected)(void* conn, int status,void* user_data);
typedef void(*on_close)(void* conn);


struct send_req_s {
	uv_udp_send_t req;
	uv_buf_t buf;
};

struct msg_t
{
	unsigned int size;
	char* buff;
	void* conn;

	msg_t()
		:size(0)
		, buff((char*)0)
		, conn((void*)0)
	{

	}
};


void* malloc_buff(size_t size);
void  free_buff(void* buff);

#endif