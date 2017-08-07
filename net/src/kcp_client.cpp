
#include "kcp_client.h"
#include "header.h"
#include "err.h"
#include "utility.h"
#include "tm.h"
#include<iostream>


//-----------------------------------------------------------------------------------

static void on_alloc_buffer(uv_handle_t* handle, size_t size, uv_buf_t* buf)
{
	buf->len = (unsigned long)size;
	buf->base = (char*)malloc_buff(size);
}

static void on_recv_udp_client(
	uv_udp_t* handle,
	ssize_t nread,
	const uv_buf_t* rcvbuf,
	const struct sockaddr* addr,
	unsigned flags) 
{

	KcpClient* conn;
	if (nread <= 0) {
		goto Exit0;
	}

	conn = static_cast<KcpClient*>(handle->data);
	conn->on_recv_udp(rcvbuf->base, nread, addr);

Exit0:
	free_buff(rcvbuf->base);
}

static void on_send_done(uv_udp_send_t* req, int status)
{
	send_req_s* send_req = (send_req_s*)req;
	free_buff(send_req->buf.base);
	SAFE_DELETE(send_req);
}

static int on_kcp_output(const char* buf, int len, struct IKCPCB* kcp, void* user)
{
	KcpClient* conn = (KcpClient*)user;
	return conn->send_udp(buf, len);
}

static void on_close_done(uv_handle_t* handle)
{

}

//-------------------------------------------------------------------------------

KcpClient::KcpClient()
	:kcp_(NULL)
	,loop_(NULL)
	,recv_cb_(NULL)
	,user_data_(NULL)
{
	conv_ = -1;
	ikcp_allocator(malloc_buff, free_buff);
}

KcpClient::~KcpClient()
{

}

int KcpClient::Init(conv_t conv, const char* server, int port)
{
	int r = -1;
	conv_ = conv;
	r = uv_ip4_addr(server, port, &addr_);
	PROC_ERR(r);
	return 0;
Exit0:
	return r;
}

int KcpClient::Connect()
{
	int r = -1;
	kcp_ = ikcp_create(conv_, (void*)this);
	CHK_COND(kcp_);
	kcp_->output = on_kcp_output;
	r = ikcp_nodelay(kcp_, 1, 10, 2, 1);
	PROC_ERR(r);

	loop_ = uv_loop_new();
	udp_.data = this;
	uv_udp_init(loop_, &udp_);

	r = uv_udp_recv_start(&udp_, on_alloc_buffer, on_recv_udp_client);
	PROC_ERR(r);

	return 0;
Exit0:
	return r;
}

void KcpClient::Shutdown()
{
	if (!loop_) return;

	if(kcp_)
		ikcp_release(kcp_);

	kcp_ = NULL;

	uv_close((uv_handle_t*)&udp_, on_close_done);
	uv_run(loop_, UV_RUN_DEFAULT);
	uv_loop_delete(loop_);
	loop_ = NULL;
}

void KcpClient::Update()
{
	if (loop_)
	{
		uv_run(loop_, UV_RUN_NOWAIT);
	}

	if (kcp_)
	{
		uint64_t ms = get_tick_ms();
		ikcp_update(kcp_, (uint32_t)ms);

		//recv
		while (true)   //!!!!
		{
			char* buf;
			uint32_t size;

			int r = recv_kcp(&buf, size);
			if (r < 0) 
			{
				break;
			}
			else if (r == 0)
			{
				msg_t msg;
				msg.conn = this;
				msg.buff = buf;
				msg.size = size;
				msg_list_.push_back(msg);
			}
		}
	}

	process_msg();
}

void KcpClient::process_msg()
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

int KcpClient::Send(const char* buf, uint32_t len)
{
	if (kcp_ == NULL) return -1;
	return ikcp_send(kcp_, buf, len);
}

int KcpClient::send_udp(const char* buf, uint32_t len)
{
	int r = -1;
	send_req_s* req = NULL;

	req = NEW(send_req_s());
	CHK_COND(req);

	req->buf.base = (char*)malloc_buff(len);
	req->buf.len = len;

	memcpy(req->buf.base, buf, len);

	r = uv_udp_send((uv_udp_send_t*)req, &udp_, &req->buf, 1, (const sockaddr*)&addr_, on_send_done);
	PROC_ERR(r);

	return 0;

Exit0:
	if (req) {
		free_buff(req->buf.base);
	}

	SAFE_DELETE(req);
	return r;
}

void KcpClient::on_recv_udp(const char* buf, ssize_t size, const struct sockaddr* addr)
{
	ikcp_input(kcp_, buf, (long)size);
}

int KcpClient::recv_kcp(char** buf, uint32_t& size)
{
	int r = -1;
	char* data = NULL;

	int len = ikcp_peeksize(kcp_);
	CHK_COND_NOLOG(len > 0);

	data =(char*) malloc_buff(len);
	r = ikcp_recv(kcp_, data, len);
	PROC_ERR(r);

	size = (uint32_t)len;
	*buf = data;

	return 0;
Exit0:
	free_buff(data);
	return r;
}
