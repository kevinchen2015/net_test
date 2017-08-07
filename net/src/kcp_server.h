#ifndef KCP_SERVER_H
#define KCP_SERVER_H



#include<map>
#include<list>
#include<uv.h>
#include"ikcp.h"

#include"utility.h"

class KcpConn;
class KcpServer 
{
public:
	KcpServer();
	~KcpServer();

	int		 Listen(const char* local_addr, int port);
	void	 Shutdown();
	void	 Update();
	KcpConn* CreateKcpConn(conv_t conv);
	void	 ReleaseKcpConn(KcpConn* conn);
	KcpConn* GetKcpConnByConv(conv_t conv);
	on_recv_data recv_cb_;
private:

	uv_loop_t*  uv_loop_;
	uv_udp_t	uv_udp_;
	
	std::map<conv_t, KcpConn*> kcp_conn_map_;
	std::list<msg_t> msg_list_;

public:
	//internal
	void on_recv_udp(const char* buf, ssize_t size, const struct sockaddr* addr);
	void on_recv_kcp(KcpConn* conn, char* buf, uint32_t size);

private:
	void process_msg();

};

#endif