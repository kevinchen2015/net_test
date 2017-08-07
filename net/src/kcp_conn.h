#ifndef KCP_CONN_H
#define KCP_CONN_H


#include "header.h"

#include "ikcp.h"
#include "utility.h"

class KcpServer;

class KcpConn 
{
public:
	KcpConn(KcpServer* server);
	~KcpConn();

	int		Init(conv_t conv, uv_udp_t* udp);
	void	Shutdown();
	void	Update(uint64_t ms);
	int		Send(const char* buf, uint32_t len);
	conv_t	GetConv() { return conv_; }

	void  SetUserData(void* data) { user_data_ = data; }
	void* GetUserData() { return user_data_; }

private:
	conv_t		conv_;
	ikcpcb*		kcp_;
	KcpServer*	server_;
	uv_udp_t*	udp_;
	struct sockaddr addr_;

	bool		need_update_;
	uint32_t    next_update_time_;

	void*		user_data_;
public:
	//internal
	int  send_udp(const char* buf, uint32_t len);
	void on_recv_udp(const char* buf, ssize_t size, const struct sockaddr* addr);

private:
	int recv_kcp(char** buf, uint32_t& size);

};

#endif