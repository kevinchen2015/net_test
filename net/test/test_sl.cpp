
#include"../include/network.h"

#include"statistic.h"
#include<Windows.h>
#include<iostream>
using namespace std;

static statistic gs_tcp_stat;
static statistic gs_kcp_stat;

sl_client_t* client;
sl_server_t* server;
void s_on_recv_data(char* buff, unsigned int size, void* conn, int channel,void* user_data)
{
	//cout << "s_on_recv_data ,channel:" << channel << " size:" << size << endl;

	if (channel == TCP)
	{
		if (size == 4)
		{
			int conv = 1;  //just for test!
			sl_server_set_conv(server, conn, conv);
			sl_server_send_data(server, conn, (char*)&conv, 4);
			return;
		}
	}

	int send_size = sl_server_send_data(server, conn, buff, size);
	if (send_size < 0)
	{
		cout << "server send error!" << endl;
	}
}

void s_on_connected(void* conn, int status, int channel, void* user_data)
{
	cout << "s_on_connected ,channel:" << channel << " status:" << status << endl;

}

void s_on_close(void* conn, int channel, void* user_data)
{
	cout << "s_on_close ,channel:" << channel << endl;

}
//----------------------------------------------------------------------------
void c_on_recv_data(char* buff, unsigned int size, void* conn, int channel, void* user_data)
{
	
	if (channel == TCP)
	{
		if (size == 4)
		{
			int conv = 0;
			memcpy(&conv, buff, 4);

			if (conv != 0)
			{
				sl_client_switch_kcp(client,conv);
			}
			return;
		}
	}
	unsigned int time = 0;
	memcpy( &time,buff, 4);
	unsigned int time_now = timeGetTime();
	int dt = time_now - time;
	
	if (channel == TCP)
	{
		on_recv(&gs_tcp_stat, dt);
	}
	else if (channel == KCP)
	{
		on_recv(&gs_kcp_stat, dt);
	}
	
	//cout << "c_on_recv_data ,channel:" << channel << " size:" << size << " dt:" << dt <<endl;
	
	memcpy(buff, &time_now, 4);
	int send_size = sl_client_send_data(client, buff, 10);
	if (send_size < 0)
	{
		cout << "client send error!" << endl;
	}

	SL_CHANNEL_MODE mode = sl_client_get_channel_mode(client);
	if (mode == TCP)
	{
		on_send(&gs_tcp_stat);
	}
	else if (mode == KCP)
	{
		on_send(&gs_kcp_stat);
	}
}

void c_on_connected(void* conn, int status, int channel, void* user_data)
{
	cout << "c_on_connected ,channel:" << channel << " status:" << status << endl;
	if (channel == TCP)
	{
		int conv = 1;  //just for test!
		sl_client_send_data(client, (char*)&conv, 4);
	}
	else
	{
		char buf[10] = { 0x00 };
		unsigned int time = timeGetTime();
		memcpy(buf, &time, 4);
		sl_client_send_data(client, buf, 10);
	}
}

void c_on_close(void* conn, int channel, void* user_data)
{
	cout << "c_on_close ,channel:" << channel << endl;
}


int main()
{
	sl_client_create(&client,NULL);

	sl_client_cb(client, c_on_connected, c_on_recv_data, c_on_close);
	sl_client_init(client, "127.0.0.1", 6000, 6668);
	
	sl_server_create(&server, NULL);
	sl_server_cb(server, s_on_connected, s_on_recv_data, s_on_close);
	sl_server_start(server, "127.0.0.1", 6000, 6668,1024);
	
	sl_client_connect(client);

	unsigned int lastTime = timeGetTime();
	unsigned int stat_time = lastTime;
	while (1)
	{
		unsigned int timeNow = timeGetTime();
		unsigned int dt = timeNow - lastTime;

		sl_server_update(server);
		sl_client_update(client);

		if (timeNow - stat_time > 1000)
		{
			stat_time = timeNow;
			calculate(&gs_tcp_stat);
			output("tcp",&gs_tcp_stat);

			calculate(&gs_kcp_stat);
			output("kcp",&gs_kcp_stat);
		}

		Sleep(10);
	}

	sl_client_shutdown(client);
	sl_client_release(client);

	sl_server_shutdown(server);
	sl_server_release(server);

	return 0;
}



