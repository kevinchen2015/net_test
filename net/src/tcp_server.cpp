

#include"tcp_server.h"
#include"header.h"
#include"tcp_conn.h"
#include"tm.h"
//---------------------------------------------------------------------------------------
static void on_alloc_buffer(uv_handle_t* handle, size_t size, uv_buf_t* buf)
{
	buf->len = (unsigned long)size;
	buf->base = (char*)malloc_buff(size);
}

static void on_close_done(uv_handle_t* handle)
{
	
}

static void read_callback(uv_stream_t *client, ssize_t read, const uv_buf_t *buf)
{
	TcpConn* conn = static_cast<TcpConn*>(client->data);
	if (read < 0) 
	{
		if (read != UV_EOF)
			fprintf(stderr, "read error %s\n", uv_err_name(read));

		conn->GetTcpServer()->on_close(conn);
		return;
	}
	else if (read > 0) 
	{
		conn->GetTcpServer()->on_recv_tcp(conn, buf->base, read);
		return;
	}

	if (buf->base)
		free_buff(buf->base);
}

static void on_new_connection(uv_stream_t *server, int status) {
	if (status < 0)
	{
		// error!  
		log_err("new connection error %s\n", uv_strerror(status));
		return;
	}

	uv_tcp_t* s = (uv_tcp_t*)server;
	TcpServer* svr = static_cast<TcpServer*>(s->data);
	uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	uv_tcp_init(svr->get_uvloop(), client);
	if (uv_accept(server, (uv_stream_t*)client) == 0)
	{
		uv_read_start((uv_stream_t*)client, on_alloc_buffer, read_callback);
		svr->on_connect(client,0);
	}
	else 
	{
		uv_close((uv_handle_t*)client, NULL);
	}
	log_info("on new connection, status:%d\r\n", status);
}

//----------------------------------------------------------------------------

TcpServer::TcpServer()
	:recv_cb_(NULL)
	,conn_cb_(NULL)
	,close_cb_(NULL)
	,user_data_(NULL)
{
	uv_loop_ = uv_loop_new();
}

TcpServer::~TcpServer()
{

}

//#define DEFAULT_BACKLOG 128 
int TcpServer::Listen(const char* local_addr, int port, int backlog)
{
	int r = -1;
	struct sockaddr_in bind_addr;
	uv_tcp_.data = this;
	uv_tcp_init(uv_loop_, &uv_tcp_);
	r = uv_ip4_addr(local_addr, port, &bind_addr);
	PROC_ERR(r);
	r = uv_tcp_bind(&uv_tcp_, (const struct sockaddr*)&bind_addr, 0);
	PROC_ERR(r);
	r = uv_listen((uv_stream_t*)&uv_tcp_, backlog, on_new_connection);
	PROC_ERR(r);
	log_info("tcp listen port: %d", port);
	return 0;

Exit0:
	return r;
}

void TcpServer::Shutdown()
{

	uv_close((uv_handle_t*)&uv_tcp_, on_close_done);
	uv_run(uv_loop_, UV_RUN_DEFAULT);
	uv_loop_delete(uv_loop_);

	std::list<msg_t>::iterator it = msg_list_.begin();
	for (; it != msg_list_.end(); ++it)
	{
		msg_t& msg = *it;
		free_buff(msg.buff);
	}
	msg_list_.clear();
}


void TcpServer::Update()
{
	if (uv_loop_)
	{
		uv_run(uv_loop_, UV_RUN_NOWAIT);
	}
	
	uint64_t tick = get_tick_ms();
	std::list<TcpConn*>::iterator it = conn_list_.begin();
	for (; it != conn_list_.end(); ++it)
	{
		TcpConn* conn = *it;
		conn->Update((uint32_t)tick);
	}

	//process msg
	process_msg();
}

void TcpServer::process_msg()
{
	std::list<msg_t>::iterator it = msg_list_.begin();
	for (; it != msg_list_.end(); ++it)
	{
		msg_t& msg = *it;

		if (recv_cb_)
		{
			recv_cb_(msg.buff, msg.size, msg.conn);
		}
		free_buff(msg.buff);
	}
	msg_list_.clear();
}

void TcpServer::on_connect(uv_tcp_t* client,int status)
{
	TcpConn* conn = NEW(TcpConn(this));
	conn->Init(client);
	conn_list_.push_back(conn);
	if (conn_cb_)
	{
		conn_cb_(conn, status,user_data_);
	}
}

void TcpServer::on_close(TcpConn* client)
{
	if (close_cb_)
	{
		close_cb_(client);
	}
	conn_list_.remove(client);
	client->Shutdown();
}

void TcpServer::on_recv_tcp(TcpConn* client, char* buff, ssize_t size)
{
	msg_t msg;
	msg.conn = client;
	msg.buff = buff;
	msg.size = (unsigned int)size;
	msg_list_.push_back(msg);
}

