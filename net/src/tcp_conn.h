#ifndef TCP_CONN_H
#define TCP_CONN_H


#include "header.h"
#include "utility.h"

class TcpServer;

class TcpConn 
{
public:
	TcpConn(TcpServer* server);
	~TcpConn();

	int		Init(uv_tcp_t* tcp);
	void	Shutdown();
	void	Update(uint64_t ms);
	int		Send(const char* buf, uint32_t len);

	TcpServer* GetTcpServer(){return server_;}
	void  SetUserData(void* data) { user_data_ = data; }
	inline void* GetUserData() { return user_data_; }
private:

	TcpServer*	server_;
	uv_tcp_t*	tcp_;
	void*		user_data_;

public:


private:


};

#endif