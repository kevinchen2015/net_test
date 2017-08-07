
#include<iostream>
#include"tcp_client.h"
#include"log.h"
#include"header.h"

//-----------------------------------------------------------------------------------

static void on_alloc_client_buffer(uv_handle_t* handle, size_t size, uv_buf_t* buf)
{
	buf->len = (unsigned long)size;
	buf->base = (char*)malloc_buff(size);
}

static void read_client_callback(uv_stream_t *client, ssize_t read, const uv_buf_t *buf)
{
	TcpClient* conn = static_cast<TcpClient*>(client->data);
	if (read < 0)
	{
		if (read != UV_EOF)
			fprintf(stderr, "read error %s\n", uv_err_name(read));

		conn->on_close();
	}
	else if (read > 0)
	{
		conn->on_recv_tcp(buf->base, read);
		return;
	}

	if (buf->base)
		free_buff(buf->base);
}

static void connect_callback(uv_connect_t *req, int status)
{
	uv_stream_t* tcp = req->handle;
	TcpClient* conn = static_cast<TcpClient*>(tcp->data);

	if (status < 0) 
	{
		// error!  
		log_err("new connection error %s\n", uv_strerror(status));
		conn->on_connected(status);
		return;
	}

	conn->on_connected(0);
}

static void write_done(uv_write_t *req, int status)
{
	if (status) {
		log_err("write error %s\n", uv_strerror(status));
	}
	free_buff(req);
}

static void on_close_done(uv_handle_t* handle)
{

}

//-------------------------------------------------------------------------------

TcpClient::TcpClient()
	:loop_(NULL)
	,recv_cb_(NULL)
	,connect_cb_(NULL)
	,close_cb_(NULL)
	,is_connected_(false)
	, user_data_(NULL)
{

}

TcpClient::~TcpClient()
{

}

int TcpClient::Init(const char* server, int port)
{
	int r = -1;
	r = uv_ip4_addr(server, port, &addr_);
	PROC_ERR(r);
	return 0;
Exit0:
	return r;
}

int TcpClient::Connect()
{
	int r = -1;

	is_connected_ = false;
	loop_ = uv_loop_new();
	tcp_.data = this;
	uv_tcp_init(loop_, &tcp_);

	uv_connect_t connect;
	connect.data = this;
	r = uv_tcp_connect(&connect, &tcp_, (const struct sockaddr*)&addr_, connect_callback);
	PROC_ERR(r);
	uv_run(loop_, UV_RUN_DEFAULT);
	r = uv_read_start((uv_stream_s*)&tcp_, on_alloc_client_buffer, read_client_callback);
	PROC_ERR(r);
	return 0;
Exit0:
	return r;
}

void TcpClient::Shutdown()
{
	if (!loop_) return;

	is_connected_ = false;
	std::list<msg_t>::iterator it = msg_list_.begin();
	for (; it != msg_list_.end(); ++it)
	{
		msg_t& msg = *it;
		free_buff(msg.buff);
	}
	msg_list_.clear();

	uv_close((uv_handle_t*)&tcp_, on_close_done);

	uv_run(loop_, UV_RUN_DEFAULT);
	uv_loop_delete(loop_);
	loop_ = NULL;
}

void TcpClient::Update()
{
	if (loop_)
	{
		uv_run(loop_, UV_RUN_NOWAIT);
	}
	process_msg();
}

void TcpClient::process_msg()
{
	std::list<msg_t>::iterator it = msg_list_.begin();
	for (; it != msg_list_.end(); ++it)
	{
		msg_t& msg = *it;

		if (recv_cb_)
			recv_cb_(msg.buff, msg.size, msg.conn);

		free_buff(msg.buff);
	}
	msg_list_.clear();
}

int TcpClient::Send(const char* buf, uint32_t len)
{
	uv_write_t *req = (uv_write_t *)malloc_buff(sizeof(uv_write_t));
	uv_buf_t wrbuf = uv_buf_init((char*)buf, len);
	int ret = uv_write(req, (uv_stream_t*)&tcp_, &wrbuf, 1, write_done);
	return ret;
}

void TcpClient::on_connected(int status)
{
	is_connected_ = (status == 0);
	if (connect_cb_)
	{
		connect_cb_(this, status, user_data_);
	}
}

void TcpClient::on_close()
{
	is_connected_ = false;
	if (close_cb_)
	{
		close_cb_(this);
	}
	this->Shutdown();
}

void TcpClient::on_recv_tcp(char* buff, ssize_t size)
{
	msg_t msg;
	msg.conn = this;
	msg.buff = buff;
	msg.size = (unsigned int)size;
	msg_list_.push_back(msg);
}





