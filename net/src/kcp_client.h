#ifndef KCP_CLIENT_H
#define KCP_CLIENT_H


#include"ikcp.h"
#include <uv.h>
#include<list>
#include"utility.h"

class KcpClient 
{
public:
	KcpClient();
	~KcpClient();

	int		Init(conv_t conv,const char* server, int port);
	int		Connect();
	void	Shutdown();

	void	Update();
	int		Send(const char* buf, uint32_t len);
	conv_t	GetConv() { return conv_; }

	on_recv_data recv_cb_;
	void*   user_data_;
private:

	uv_loop_t* loop_;
	uv_udp_t   udp_;
	struct sockaddr_in addr_;
	conv_t conv_;
	ikcpcb* kcp_;
	std::list<msg_t> msg_list_;

public:
	//internal
	int  send_udp(const char* buf, uint32_t len);
	void on_recv_udp(const char* buf, ssize_t size, const struct sockaddr* addr);

private:
	int recv_kcp(char** buf, uint32_t& size);
	void process_msg();
};

#endif