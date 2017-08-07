#include"tcp_conn.h"
#include"tcp_server.h"



//---------------------------------------------------------
void write_done(uv_write_t *req, int status)
{
	if (status) {
		log_err("write error %s\n", uv_strerror(status));
	}
	free_buff(req);
}

//---------------------------------------------------------

TcpConn::TcpConn(TcpServer* server)
	:server_(server)
	,tcp_(NULL)
	,user_data_(NULL)
{

}

TcpConn::~TcpConn()
{

}

int TcpConn::Init(uv_tcp_t* tcp) 
{ 
	tcp_ = tcp;
	tcp_->data = this;
	return 0;
}

void TcpConn::Shutdown()
{
	
}

void TcpConn::Update(uint64_t ms)
{

}

int	TcpConn::Send(const char* buf, uint32_t len)
{
	uv_write_t *req = (uv_write_t *) malloc_buff(sizeof(uv_write_t));
	uv_buf_t wrbuf = uv_buf_init((char*)buf, len);
	int ret = uv_write(req, (uv_stream_t*)tcp_, &wrbuf, 1, write_done);
	return ret;
}