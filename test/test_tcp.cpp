


#include"../src/tcp_client.h"
#include"../src/tcp_server.h"
#include"../src/tcp_conn.h"


#define DATA_LEN 256
static void on_client_connected(void* conn,int status,void* user_data)
{
	log_info("on_client_connected %d",status);

}

static void on_server_recv(char* buff, unsigned int size, void* conn)
{
	log_info("on_server_recv");

	TcpConn* c = static_cast<TcpConn*>(conn);
	c->Send(buff, size);
}

static void on_client_close(void* conn)
{
	log_info("on_client_close");
}

static void on_server_connected(void* conn,int status,void* user_data)
{
	log_info("on_server_connected,%d", status);

	unsigned int timeNow = timeGetTime();
	char buff[DATA_LEN];
	memset(buff, 0x00, DATA_LEN);
	memcpy(buff, &timeNow, 4);
	memcpy(buff + 4, "helloworld!", sizeof("helloworld!"));
	static_cast<TcpClient*>(conn)->Send(buff, DATA_LEN);
}

static void on_client_recv(char* buff, unsigned int size, void* conn)
{
	unsigned int time_now = timeGetTime();
	unsigned int send_time = 0;
	send_time = *(unsigned int*)buff;
	int ping = time_now - send_time;
	log_info("recv ping,time=%d,len=%d", ping, size);
}

static void on_server_close(void* conn)
{
	log_info("on_server_close");
}


//int main()
int test_tcp()
{
	TcpServer server;
	server.recv_cb_ = on_server_recv;
	server.conn_cb_ = on_client_connected;
	server.close_cb_ = on_client_close;
	server.Listen("127.0.0.1", 6668, 6);

	TcpClient client;
	client.connect_cb_ = on_server_connected;
	client.close_cb_ = on_server_close;
	client.recv_cb_ = on_client_recv;
	client.Init("127.0.0.1", 6668);
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
				if (client.IsConnected())
				{
					unsigned int timeNow = timeGetTime();
					char buff[DATA_LEN];
					memset(buff, 0x00, DATA_LEN);
					memcpy(buff, &timeNow, 4);
					memcpy(buff + 4, "helloworld!", sizeof("helloworld!"));
					client.Send(buff, DATA_LEN);

					++send_counter;
				}
			}
		}
		Sleep(10);
	}

	client.Shutdown();
	server.Shutdown();

	return 0;
}



