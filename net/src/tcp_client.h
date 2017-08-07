#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H


#include <uv.h>
#include<list>
#include"utility.h"

class TcpClient 
{
public:
	TcpClient();
	~TcpClient();

	int		Init(const char* server, int port);
	int		Connect();
	void	Shutdown();

	void	Update();
	int		Send(const char* buf, uint32_t len);
	bool	IsConnected() { return is_connected_; }

	void*   user_data_;
	on_connected connect_cb_;
	on_recv_data recv_cb_;
	on_close close_cb_;
private:

	uv_loop_t* loop_;
	uv_tcp_t   tcp_;

	struct sockaddr_in addr_;
	std::list<msg_t> msg_list_;
	bool is_connected_;

public:
	//internal
	void on_connected(int status);
	void on_close();
	void on_recv_tcp(char* buff, ssize_t size);
private:

	void process_msg();
};

#endif