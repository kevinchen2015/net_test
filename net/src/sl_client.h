#ifndef _SL_CLIENT_H_
#define _SL_CLIENT_H_

class TcpClient;
class KcpClient;

#include<string>
#include"sl_common.h"
#include"network.h"

class SLClient
{
public:

	SLClient();
	~SLClient();

	void Init(const char* ip,int tcp_port,int udp_port);
	void SetConv(int conv) { conv_ = conv; }
	int Connect();
	void Shutdown();
	void Update();
	void SetMode(SL_MODE mode) { mode_ = mode; }
	int SendData(char* buff, unsigned int len);
	void SwitchToKcp();

	SL_CHANNEL_MODE GetChannelMode() {return channel_mode_;}

	sl_on_recv_data recv_data_cb_;
	sl_on_connected connected_cb_;
	sl_on_close		close_cb_;

	void*			user_data_;
private:
	std::string server_;
	int tcp_port_;
	int udp_port_;
	TcpClient* tcp_client_;
	KcpClient* kcp_client_;

	SL_MODE mode_;
	SL_CHANNEL_MODE channel_mode_;
	int conv_;

	uint32_t disconnect_tcp_delay_ = 0;

private:
	//tcp callback
	static void on_tcp_connect(void* conn, int status,void* user_data);
	static void on_tcp_close(void* conn);
	static void on_tcp_recv(char* buff, unsigned int size, void* conn);

	//kcp callback
	static void on_kcp_recv(char* buff, unsigned int size, void* conn);

private:
	void OnTcpConnected(void* conn, int status);
	void OnTcpRecv(char* buff, unsigned int size, void* conn);
	void OnTcpClose();
	void OnKcpRecv(char* buff, unsigned int size, void* conn);
	
};


#endif