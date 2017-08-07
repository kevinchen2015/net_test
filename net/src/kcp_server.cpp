

#include"kcp_server.h"
#include"header.h"
#include"kcp_conn.h"
#include"tm.h"
//---------------------------------------------------------------------------------------
static void on_alloc_buffer(uv_handle_t* handle, size_t size, uv_buf_t* buf)
{
	buf->len = (unsigned long)size;
	buf->base = (char*)malloc_buff(size);
}

static void on_recv_udp_server(
	uv_udp_t* handle,
	ssize_t nread,
	const uv_buf_t* rcvbuf,
	const struct sockaddr* addr,
	unsigned flags)
{
	KcpServer* server;
	if (nread <= 0) {
		goto Exit0;
	}

	server = static_cast<KcpServer*>(handle->data);
	server->on_recv_udp(rcvbuf->base, nread, addr);

Exit0:
	free_buff(rcvbuf->base);
}

static void on_close_done(uv_handle_t* handle)
{
	
}

//----------------------------------------------------------------------------

KcpServer::KcpServer()
	:recv_cb_(NULL)
{
	uv_loop_ = uv_loop_new();
	ikcp_allocator(malloc_buff, free_buff);
}

KcpServer::~KcpServer()
{

}

int KcpServer::Listen(const char* local_addr, int port)
{
	int r = -1;
	struct sockaddr_in bind_addr;
	uv_udp_.data = this;
	uv_udp_init(uv_loop_, &uv_udp_);
	r = uv_ip4_addr(local_addr, port, &bind_addr);
	PROC_ERR(r);
	r = uv_udp_bind(&uv_udp_, (const struct sockaddr*)&bind_addr, 0);
	PROC_ERR(r);
	r = uv_udp_recv_start(&uv_udp_, on_alloc_buffer, on_recv_udp_server);
	PROC_ERR(r);
	log_info("udp listen port: %d", port);

	return 0;
Exit0:
	return r;
}

void KcpServer::Shutdown()
{
	uv_close((uv_handle_t*)&uv_udp_, on_close_done);
	uv_run(uv_loop_, UV_RUN_DEFAULT);
	uv_loop_delete(uv_loop_);

	for (std::map<conv_t, KcpConn*>::iterator it = kcp_conn_map_.begin();
		it != kcp_conn_map_.end(); ++it) 
	{
		KcpConn* conn = it->second;
		conn->Shutdown();
		SAFE_DELETE(conn);
	}

	std::list<msg_t>::iterator it = msg_list_.begin();
	for (; it != msg_list_.end(); ++it)
	{
		msg_t& msg = *it;
		free_buff(msg.buff);
	}
	msg_list_.clear();
}

KcpConn* KcpServer::CreateKcpConn(conv_t conv)
{
	KcpConn* conn = GetKcpConnByConv(conv);

	if (conn != NULL)
	{
		conn->Shutdown();
		SAFE_DELETE(conn);
	}

	conn = NEW(KcpConn(this));
	conn->Init(conv, &uv_udp_);
	
	kcp_conn_map_[conn->GetConv()] = conn;
	return conn;
}

void KcpServer::ReleaseKcpConn(KcpConn* conn)
{
	if (conn == NULL) return;

	kcp_conn_map_.erase(conn->GetConv());
	conn->Shutdown();
	SAFE_DELETE(conn);
}

void KcpServer::Update()
{
	if (uv_loop_)
	{
		uv_run(uv_loop_, UV_RUN_NOWAIT);
	}
	
	uint64_t tick = get_tick_ms();
	//std::vector<conv_t> remove_list;
	for (std::map<conv_t, KcpConn*>::iterator it = kcp_conn_map_.begin();
		it != kcp_conn_map_.end(); ++it) 
	{
		KcpConn* conn = it->second;
		/*
		if (conn->expired() == 0) {
			remove_list.push_back(conn->GetConv());
			SAFE_DELETE(conn);
		}
		else
		*/
		{
			conn->Update(tick);
		}
	}
	/*
	for (std::vector<conv_t>::iterator it = remove_list.begin();
		it != remove_list.end(); ++it) {
		kcp_conn_map_.erase(*it);
	}
	*/

	//process msg
	process_msg();
}

KcpConn* KcpServer::GetKcpConnByConv(conv_t conv)
{
	std::map<conv_t, KcpConn*>::iterator it = kcp_conn_map_.find(conv);
	if (it != kcp_conn_map_.end())
		return it->second;
	return NULL;
}

void KcpServer::on_recv_udp(const char* buf, ssize_t size, const struct sockaddr* addr)
{
	int r = -1;
	conv_t conv;
	KcpConn* conn = NULL;
	conv = ikcp_getconv(buf);
	conn = GetKcpConnByConv(conv);
	CHK_COND_NOLOG(conn);
	conn->on_recv_udp(buf, size, addr);
Exit0:
	return;
}

void KcpServer::on_recv_kcp(KcpConn* conn, char* buf, uint32_t size)
{
	msg_t msg;
	msg.conn = conn;
	msg.buff = buf;
	msg.size = size;

	msg_list_.push_back(msg);
}

void KcpServer::process_msg()
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