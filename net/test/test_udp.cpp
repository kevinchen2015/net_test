

#include"../src/kcp_client.h"
#include"../src/kcp_server.h"
#include"../src/kcp_conn.h"

#pragma comment(lib,"winmm.lib")


static void on_server_recv(char* buff,unsigned int size,void* conn)
{
	KcpConn* c = static_cast<KcpConn*>(conn);
	c->Send(buff,size);
}

static void on_client_recv(char* buff, unsigned int size, void* conn)
{
	unsigned int time_now = timeGetTime();
	unsigned int send_time = 0;
	send_time = *(unsigned int*)buff;
	int ping = time_now - send_time;
	log_info("recv ping,time=%d,len=%d", ping,size);
}

#define DATA_LEN 256
//int main()
int test_kcp()
{
	conv_t conv = 1;		//服务端客户端必须配对

	KcpServer server;
	server.Listen("127.0.0.1", 6666);
	server.recv_cb_ = on_server_recv;
	server.CreateKcpConn(conv);

	KcpClient client;
	client.Init(conv,"127.0.0.1", 6666);
	client.recv_cb_ = on_client_recv;
	client.Connect();
	
	int send_counter = 0;
	unsigned int lastTime = timeGetTime();
	while (1)
	{
		unsigned int timeNow = timeGetTime();
		unsigned int dt = timeNow - lastTime;

		server.Update();
		client.Update();

		if (dt > 15)
		{
			lastTime = timeNow;
			if (send_counter < 10000)
			{
				char buff[DATA_LEN];
				memset(buff, 0x00, DATA_LEN);
				memcpy(buff, &timeNow, 4);
				memcpy(buff + 4, "helloworld!",sizeof("helloworld!"));
				client.Send(buff, DATA_LEN);
				++send_counter;
			}
		}
		Sleep(3);
	}

	client.Shutdown();
	server.Shutdown();

	return 0;
}