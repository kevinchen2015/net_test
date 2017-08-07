
#include"tcp_server.h"
#include"tcp_conn.h"
#include"kcp_server.h"
#include"kcp_conn.h"
#include"sl_server.h" 
#include"header.h"
#include"tm.h"

//----------------------------------------------------------------------------

void SLConn::Reset()
{
	conv_ = 0;
	kcp_conn_ = NULL;
	tcp_conn_ = NULL;
	channel_mode_ = TCP;

	if (tcp_conn_)
		tcp_conn_->SetUserData(NULL);
	tcp_conn_ = NULL;
	if (kcp_conn_)
	{
		kcp_conn_->SetUserData(NULL);
		server_->kcp_server_->ReleaseKcpConn(kcp_conn_);
	}
	kcp_conn_ = NULL;
}

void SLConn::SetTcpConn(void* conn)
{
	if (tcp_conn_)
		tcp_conn_->SetUserData(NULL);
	tcp_conn_ = static_cast<TcpConn*>(conn);
	if (tcp_conn_)
		tcp_conn_->SetUserData(this);
}


void SLConn::SetKcpConn(int conv)
{
	if (kcp_conn_)
	{
		kcp_conn_->SetUserData(NULL);
		server_->kcp_server_->ReleaseKcpConn(kcp_conn_);
		kcp_conn_ = NULL;
	}
	conv_ = conv;
	kcp_conn_ = server_->kcp_server_->CreateKcpConn(conv_);
	if (kcp_conn_)
		kcp_conn_->SetUserData(this);
}

int SLConn::SendData(char* buff, unsigned int len)
{
	if (channel_mode_ == TCP)
	{
		return tcp_conn_->Send(buff, len);
	}
	else
	{
		return kcp_conn_->Send(buff, len);
	}
	return 0;
}
//-----------------------------------------------------------------------------

void SLServer::on_tcp_connect(void* conn, int status,void* user_data)
{
	SLServer* server = static_cast<SLServer*>(user_data);
	server->OnTcpConnected(conn,status);
}

void SLServer::on_tcp_close(void* conn)
{
	TcpConn* tcp_conn = static_cast<TcpConn*>(conn);
	SLConn* sl_conn = static_cast<SLConn*>(tcp_conn->GetUserData());
	if (sl_conn)
	{
		//notify
		sl_conn->GetServer()->OnTcpDisconnected(sl_conn);
		sl_conn->SetTcpConn(NULL);
	}
}

//tcp
void SLServer::on_tcp_recv(char* buff, unsigned int size, void* conn)
{
	TcpConn* tcp_conn = static_cast<TcpConn*>(conn);
	SLConn* c = static_cast<SLConn*>(tcp_conn->GetUserData());
	c->GetServer()->OnTcpRecv(buff, size, c);
}

//kcp
void SLServer::on_kcp_recv(char* buff, unsigned int size, void* conn)
{
	KcpConn* tcp_conn = static_cast<KcpConn*>(conn);
	SLConn* c = static_cast<SLConn*>(tcp_conn->GetUserData());
	c->GetServer()->OnKcpRecv(buff, size, c);
}

//------------------------------------------------------------------------------

SLServer::SLServer()
	:sl_conn_array_(NULL)
{
	kcp_server_ = NEW(KcpServer());
	kcp_server_->recv_cb_ = SLServer::on_kcp_recv;
	tcp_server_ = NEW(TcpServer());
	tcp_server_->conn_cb_ = SLServer::on_tcp_connect;
	tcp_server_->close_cb_ = SLServer::on_tcp_close;
	tcp_server_->recv_cb_ = SLServer::on_tcp_recv;
	tcp_server_->user_data_ = this;
	mode_ = SMART_MODE;

	recv_data_cb_ = NULL;
	connected_cb_ = NULL;
	close_cb_ = NULL;

	user_data_ = NULL;
}

SLServer::~SLServer()
{
	SAFE_DELETE(kcp_server_);
	SAFE_DELETE(tcp_server_);
	SAFE_DELETE_ARRAY(sl_conn_array_);
}

int SLServer::Start(const char* ip, int tcp_port, int udp_port, int max_conn_num)
{
	max_conn_num_ = max_conn_num;
	sl_conn_array_ = NEW(SLConn[max_conn_num_]);
	for (int i = 0; i < max_conn_num_; ++i)
	{
		sl_conn_array_[i].Init(this);
	}
	int r = -1;
	int backlog = 10;
	r = tcp_server_->Listen(ip, tcp_port, backlog);
	PROC_ERR(r);
	r = kcp_server_->Listen(ip, udp_port);
	PROC_ERR(r);
	return 0;
Exit0:
	return r;
}

void SLServer::Shutdown()
{
	for (int i = 0; i < max_conn_num_; ++i)
	{
		sl_conn_array_[i].Reset();
	}

	kcp_server_->Shutdown();
	tcp_server_->Shutdown();
}

void SLServer::Update()
{
	tick_ = (uint32_t)get_tick_ms();
	kcp_server_->Update();
	tcp_server_->Update();
}

void SLServer::OnTcpConnected(void* conn,int status)
{
	if (status != 0)
	{
		log_err("SLServer::OnTcpConnected() status:%d", status);
		return;
	}

	SLConn* sl_conn = NULL;
	for (int i = 0; i < max_conn_num_; ++i)
	{
		if (sl_conn_array_[i].GetTcpConn() == conn)
		{
			sl_conn_array_[i].SetChannelMode(TCP);
			sl_conn_array_[i].SetTcpConn(conn);
			break;
		}
	}

	if (!sl_conn)
	{
		for (int i = 0; i < max_conn_num_; ++i)
		{
			if (sl_conn_array_[i].GetConv() == 0)
			{
				sl_conn = &sl_conn_array_[i];
				sl_conn->SetChannelMode(TCP);
				sl_conn->SetTcpConn(conn);
				break;
			}
		}
	}

	if (!sl_conn)
	{
		for (int i = 0; i < max_conn_num_; ++i)
		{
			if (sl_conn_array_[i].GetTcpConn() == NULL)
			{
				if (sl_conn == NULL)
				{
					sl_conn = &sl_conn_array_[i];
				}
				else
				{
					sl_conn = (sl_conn->last_recv_ < sl_conn_array_[i].last_recv_) ?
						sl_conn : &sl_conn_array_[i];
				}
			}
		}

		if (sl_conn)
		{
			sl_conn->Reset();
			sl_conn->SetChannelMode(TCP);
			sl_conn->SetTcpConn(conn);
		}
	}

	if (!sl_conn)
	{
		log_err("sl_conn is null!");
		return;
	}

	//notify
	if (connected_cb_)
	{
		connected_cb_(sl_conn, status,TCP, user_data_);
	}	
}

void SLServer::OnTcpDisconnected(SLConn* conn)
{
	if (close_cb_)
	{
		close_cb_(conn, TCP, user_data_);
	}
}

void SLServer::OnTcpRecv(char* buff, unsigned int size, SLConn* conn)
{
	SLConn* sl_conn = conn;
	if (!sl_conn)
	{
		log_err("SLServer::OnTcpRecv() is error 1!");
		return;
	}
	sl_conn->last_recv_ = tick_;

	//if (sl_conn->GetChannelMode() == TCP)
	{
		//notify
		if (recv_data_cb_)
		{
			recv_data_cb_(buff, size, sl_conn, TCP, user_data_);
		}
	}
}

void SLServer::OnKcpRecv(char* buff, unsigned int size, SLConn* conn)
{
	SLConn* sl_conn = conn;
	if (!sl_conn)
	{
		log_err("SLServer::OnTcpRecv() is error 1!");
		return;
	}

	sl_conn->last_recv_ = tick_;
	if (sl_conn->GetChannelMode() == KCP)
	{
		//notify
		if (recv_data_cb_)
		{
			recv_data_cb_(buff, size, sl_conn, KCP, user_data_);
		}
	}
	else
	{
		char channel_mode = TCP;
		int  conv = 0;
		char cmd = 0;
		unsigned int time = 0;
		char param = 0;
		parse_negotiate_msg(buff, (int)size, cmd, channel_mode, conv, time, param);

		if (cmd == CMD_HELLO && conv == sl_conn->GetConv() && 
			mode_ != TCP_MODE && channel_mode == KCP)
		{
			if (param >= 3)
			{
				sl_conn->SetChannelMode((SL_CHANNEL_MODE)channel_mode);
				//notify
				if (connected_cb_)
				{
					connected_cb_(sl_conn, 0, KCP, user_data_);
				}
			}
			if(param < 5)
			{
				++param;
				char buff[NEGOTIATE_BUFF_MAX] = { 0x00 };
				int len = NEGOTIATE_BUFF_MAX;
				make_negotiate_msg(cmd, channel_mode, conv, time, param, buff, len);
				sl_conn->GetKcpConn()->Send(buff, len);
			}
		}
	}
}

void SLServer::SetConv(void* conn, int conv)
{
	SLConn* c = static_cast<SLConn*>(conn);
	if (c)
	{
		c->SetKcpConn(conv);
	}
}

int SLServer::SendData(void* conn, char* buff, unsigned int len)
{
	SLConn* c = static_cast<SLConn*>(conn);
	if (c)
	{
		return c->SendData(buff, len);
	}
	return 0;
}

