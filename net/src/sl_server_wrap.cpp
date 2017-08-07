
#include "network.h"
#include "header.h"
#include "sl_server.h"

struct sl_server_s
{
	SLServer server;
};

bool sl_server_create(sl_server_t** server, void* user_data)
{
	*server = NEW(sl_server_t);
	(*server)->server.user_data_ = user_data;
	return true;
}

void sl_server_release(sl_server_t* server)
{
	SAFE_DELETE(server);
}

void sl_server_set_mode(sl_server_t* server, SL_MODE mode)
{
	server->server.SetMode(mode);
}

void sl_server_start(sl_server_t* server,const char* ip,
	int tcp_port, int udp_port, int max_conn_num)
{
	server->server.Start(ip, tcp_port, udp_port, max_conn_num);
}

void sl_server_set_conv(sl_server_t* server, void* conn, sl_conv_t conv)
{
	server->server.SetConv(conn, conv);
}

int sl_server_send_data(sl_server_t* server, void* conn, char* buff, unsigned int size)
{
	return server->server.SendData(conn, buff, size);
}

void sl_server_set_conn_user_data(void* conn,void* user_data)
{
	SLConn* c = static_cast<SLConn*>(conn);
	if (c)
	{
		c->user_data_ = user_data;
	}
}

void* sl_server_get_conn_user_data(void* conn)
{
	SLConn* c = static_cast<SLConn*>(conn);
	if (c)
	{
		return c->user_data_;
	}
	return NULL;
}

void sl_server_update(sl_server_t* server)
{
	server->server.Update();
}

void sl_server_shutdown(sl_server_t* server)
{
	server->server.Shutdown();
}

void sl_server_cb(sl_server_t* server, sl_on_connected conn_cb, sl_on_recv_data recv_cb, sl_on_close close_cb)
{
	server->server.connected_cb_ = conn_cb;
	server->server.recv_data_cb_ = recv_cb;
	server->server.close_cb_ = close_cb;
}

SL_CHANNEL_MODE sl_server_get_channel_mode(sl_server_t* server, void* conn)
{
	return server->server.GetChannelMode(conn);
}

