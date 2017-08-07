
#include"tcp_client.h"
#include"kcp_client.h"
#include "sl_client.h"
#include "header.h"
#include"tm.h"

//--------------------------------------------------------------------
//tcp callback
void SLClient::on_tcp_connect(void* conn, int status,void* user_data)
{
	TcpClient* client = static_cast<TcpClient*>(conn);
	SLClient* c = static_cast<SLClient*>(client->user_data_);
	c->OnTcpConnected(conn, status);
}

void SLClient::on_tcp_close(void* conn)
{
	TcpClient* client = static_cast<TcpClient*>(conn);
	SLClient* c = static_cast<SLClient*>(client->user_data_);
	c->OnTcpClose();
}

void SLClient::on_tcp_recv(char* buff, unsigned int size, void* conn)
{
	TcpClient* client = static_cast<TcpClient*>(conn);
	SLClient* c = static_cast<SLClient*>(client->user_data_);
	c->OnTcpRecv(buff, size, conn);
}

void SLClient::on_kcp_recv(char* buff, unsigned int size, void* conn)
{
	KcpClient* client = static_cast<KcpClient*>(conn);
	SLClient* c = static_cast<SLClient*>(client->user_data_);
	c->OnKcpRecv(buff, size, conn);
}

//--------------------------------------------------------------------

SLClient::SLClient()
{
	kcp_client_ = NEW(KcpClient);
	kcp_client_->recv_cb_ = SLClient::on_kcp_recv;
	kcp_client_->user_data_ = this;
	tcp_client_ = NEW(TcpClient);
	tcp_client_->connect_cb_ = SLClient::on_tcp_connect;
	tcp_client_->close_cb_ = SLClient::on_tcp_close;
	tcp_client_->recv_cb_ = SLClient::on_tcp_recv;
	tcp_client_->user_data_ = this;
	mode_ = SMART_MODE;
	conv_ = 0;
	channel_mode_ = TCP;
	disconnect_tcp_delay_ = 0;

	recv_data_cb_ = NULL;
	connected_cb_ = NULL;
	close_cb_ = NULL;

	user_data_ = NULL;
}

SLClient::~SLClient()
{
	SAFE_DELETE(kcp_client_);
	SAFE_DELETE(tcp_client_);
}

void SLClient::Init(const char* ip, int tcp_port, int udp_port)
{
	server_ = ip;
	tcp_port_ = tcp_port;
	udp_port_ = udp_port;

	tcp_client_->Init(ip, tcp_port);
}

void SLClient::Shutdown()
{
	disconnect_tcp_delay_ = 0;
	kcp_client_->Shutdown();
	tcp_client_->Shutdown();
	channel_mode_ = TCP;
}

int SLClient::Connect()
{
	Shutdown();
	return tcp_client_->Connect();
}

void SLClient::Update()
{
	uint32_t time_now = (uint32_t)get_tick_ms();

	kcp_client_->Update();
	tcp_client_->Update();

	if (disconnect_tcp_delay_ > 0)
	{
		if (disconnect_tcp_delay_ < time_now)
		{
			disconnect_tcp_delay_ = 0;
			if (tcp_client_->IsConnected())
			{
				tcp_client_->Shutdown();
			}
		}
	}
}


int SLClient::SendData(char* buff, unsigned int len)
{
	if (channel_mode_ == TCP)
	{
		return tcp_client_->Send(buff, len);
	}
	else
	{
		return kcp_client_->Send(buff, len);
	}
	return 0;
}

void SLClient::OnTcpConnected(void* conn, int status)
{
	disconnect_tcp_delay_ = 0;
	if (status == 0)
	{
		channel_mode_ = TCP;
	}
	//notify
	if (connected_cb_)
	{
		connected_cb_(this, status, TCP,user_data_);
	}
}

void SLClient::OnTcpRecv(char* buff, unsigned int size, void* conn)
{
	//if (channel_mode_ == TCP)
	{
		//notify!!!
		if (recv_data_cb_)
		{
			recv_data_cb_(buff, size, this, TCP, user_data_);
		}
	}
}

void SLClient::OnTcpClose()
{
	disconnect_tcp_delay_ = 0;
	//notify!!!
	if (close_cb_)
	{
		close_cb_(this, TCP, user_data_);
	}
}

void SLClient::SwitchToKcp()
{
	if (channel_mode_ == KCP) return;
	if (!tcp_client_->IsConnected()) return;
	if (conv_ == 0) return;

	char buff[NEGOTIATE_BUFF_MAX] = { 0x00 };
	int len = NEGOTIATE_BUFF_MAX;
	unsigned int time = (unsigned int)get_tick_ms();
	char param = 0;
	SL_CHANNEL_MODE channel_mode = KCP;
	char cmd = CMD_HELLO;
	make_negotiate_msg(cmd, channel_mode, conv_, time, param, buff, len);
	
	kcp_client_->Shutdown();
	kcp_client_->Init(conv_, server_.c_str(), udp_port_);
	kcp_client_->Connect();
	kcp_client_->Send(buff, len);
}

void SLClient::OnKcpRecv(char* buff, unsigned int size, void* conn)
{
	if (channel_mode_ == KCP)
	{
		//notify 
		if (recv_data_cb_)
		{
			recv_data_cb_(buff, size, this, KCP, user_data_);
		}
	}
	else
	{
		char channel_mode = TCP;
		int  conv = 0;
		char cmd = 0;
		unsigned int time = 0;
		char param = 0;
		parse_negotiate_msg(buff, (int)size, cmd, channel_mode, conv,time,param);

		if (cmd == CMD_HELLO && conv == conv_)
		{
			if (param >= 4)
			{
				channel_mode_ = KCP;  //5秒后断开tcp，3秒内udp没数据，超时检测tcp链接还在，就切回tcp
				disconnect_tcp_delay_ = (uint32_t)get_tick_ms();
				disconnect_tcp_delay_ += 5000;

				//notify 
				if (connected_cb_)
				{
					connected_cb_(this,0,KCP, user_data_);
				}
			}
			else
			{
				++param;
				char buff[NEGOTIATE_BUFF_MAX] = { 0x00 };
				int len = NEGOTIATE_BUFF_MAX;
				make_negotiate_msg(cmd, channel_mode, conv, time, param, buff, len);
				kcp_client_->Send(buff, len);
			}
		}
	}
}



