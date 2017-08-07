
#include "network.h"
#include "header.h"
#include "sl_client.h"

struct sl_client_s
{
	SLClient client;
};

bool sl_client_create(sl_client_t** client, void* user_data)
{
	*client = NEW(sl_client_t);
	(*client)->client.user_data_ = user_data;
	return true;
}

void sl_client_set_mode(sl_client_t* client, SL_MODE mode)
{
	client->client.SetMode(mode);
}

int sl_client_init(sl_client_t* client, const char* ip, int tcp_port, int udp_port)
{
	 client->client.Init(ip, tcp_port, udp_port);
	 return 0;
}

int sl_client_connect(sl_client_t* client)
{
	return client->client.Connect();
}

void sl_client_switch_kcp(sl_client_t* client, sl_conv_t conv)
{
	client->client.SetConv(conv);
	client->client.SwitchToKcp();
}

int sl_client_send_data(sl_client_t* client, char* buff, unsigned int size)
{
	return client->client.SendData(buff, size);
}

void sl_client_update(sl_client_t* client)
{
	client->client.Update();
}

void sl_client_shutdown(sl_client_t* client)
{
	client->client.Shutdown();
}

void sl_client_release(sl_client_t* client)
{
	SAFE_DELETE(client);
}

void sl_client_cb(sl_client_t* client, sl_on_connected conn_cb, sl_on_recv_data recv_cb, sl_on_close close_cb)
{
	client->client.connected_cb_ = conn_cb;
	client->client.recv_data_cb_ = recv_cb;
	client->client.close_cb_ = close_cb;
}

SL_CHANNEL_MODE sl_client_get_channel_mode(sl_client_t* client)
{
	return client->client.GetChannelMode();
}