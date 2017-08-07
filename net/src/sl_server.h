#ifndef _SL_SERVER_H_
#define _SL_SERVER_H_


class KcpServer;
class TcpServer;



#include"sl_common.h"
#include"network.h"

#include<list>
#include<map>


class SLServer;
class TcpConn;
class KcpConn;
class SLConn
{
public:
	SLConn()
		:server_(NULL)
		, kcp_conn_(NULL)
		, tcp_conn_(NULL)
		, conv_(0)
		, user_data_(NULL)
	{
		channel_mode_ = TCP;
	}

	int GetConv() { return conv_; }

	void Init(SLServer* server)
	{
		server_ = server;
	}

	void Reset();

	TcpConn* GetTcpConn()
	{
		return tcp_conn_;
	}

	void SetTcpConn(void* conn);

	void SetKcpConn(int conv);

	KcpConn* GetKcpConn()
	{
		return kcp_conn_;
	}

	void SetChannelMode(SL_CHANNEL_MODE mode)
	{
		channel_mode_ = mode;
	}

	SL_CHANNEL_MODE GetChannelMode()
	{
		return channel_mode_;
	}

	int SendData(char* buff, unsigned int len);

	unsigned int last_recv_;
	SLServer* GetServer() { return server_; }
	void*	 user_data_;
private:

	SLServer*  server_;
	int		   conv_;

	SL_CHANNEL_MODE channel_mode_;
	KcpConn*   kcp_conn_;
	TcpConn*   tcp_conn_;
	unsigned int last_recv_time_;
};

//-------------------------------------------

class SLServer
{

public:
	SLServer();
	~SLServer();

	int  Start(const char* ip, int tcp_port, int udp_port,int max_conn_num);
	void Update();
	void Shutdown();
	void SetMode(SL_MODE mode) { mode_ = mode; }

	void SetConv(void* conn,int conv);
	int  SendData(void* conn, char* buff, unsigned int len);
	SL_CHANNEL_MODE GetChannelMode(void* conn)
	{
		return static_cast<SLConn*>(conn)->GetChannelMode();
	}

	sl_on_recv_data recv_data_cb_;
	sl_on_connected connected_cb_;
	sl_on_close		close_cb_;
	void*			user_data_;
private:
	friend SLConn;
	int max_conn_num_;
	KcpServer* kcp_server_;
	TcpServer* tcp_server_;

	SL_MODE	   mode_;
	SLConn*  sl_conn_array_;
	unsigned int tick_;

	
private:
	//tcp callback
	static void on_tcp_connect(void* conn, int status,void* user_data);
	static void on_tcp_close(void* conn);
	static void on_tcp_recv(char* buff, unsigned int size, void* conn);

	//kcp callback
	static void on_kcp_recv(char* buff, unsigned int size, void* conn);
private:
	void OnTcpConnected(void* conn,int status);
	void OnTcpDisconnected(SLConn* conn);
	void OnTcpRecv(char* buff, unsigned int size, SLConn* conn);
	void OnKcpRecv(char* buff, unsigned int size, SLConn* conn);
	
};



#endif