#ifndef TCP_SERVER_H
#define TCP_SERVER_H



#include<map>
#include<list>
#include<uv.h>
#include"utility.h"

class TcpConn;
class TcpServer 
{
public:
	TcpServer();
	~TcpServer();

	int		 Listen(const char* local_addr, int port,int backlog);
	void	 Shutdown();
	void	 Update();

	on_connected conn_cb_;
	on_recv_data recv_cb_;
	on_close close_cb_;

	void*    user_data_;
private:

	uv_loop_t*  uv_loop_;
	uv_tcp_t	uv_tcp_;

	std::list<msg_t>	msg_list_;
	std::list<TcpConn*> conn_list_;

public:
	//internal
	uv_loop_t* get_uvloop() { return uv_loop_; }

	void on_connect(uv_tcp_t* client,int status);
	void on_close(TcpConn* client);
	void on_recv_tcp(TcpConn* client,char* buff, ssize_t size);

private:
	void process_msg();

};

#endif