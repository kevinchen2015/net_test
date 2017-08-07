#include"kcp_conn.h"
#include"kcp_server.h"

//------------------------------------------------------------------------------

static int on_kcp_output(const char* buf, int len, struct IKCPCB* kcp, void* user)
{
	KcpConn* conn = static_cast<KcpConn*>(user);
	return conn->send_udp(buf, len);
}

static void on_send_done(uv_udp_send_t* req, int status) 
{
	send_req_s* send_req = (send_req_s*)req;
	free_buff(send_req->buf.base);
	SAFE_DELETE(send_req);
}

static void on_close_done(uv_handle_t* handle)
{

}

//--------------------------------------------------------------------------------

KcpConn::KcpConn(KcpServer* server)
	:server_(server)
	,kcp_(NULL)
	,udp_(NULL)
	,user_data_(NULL)
{
	conv_ = -1;
	need_update_ = true;
	next_update_time_ = 0;
}

KcpConn::~KcpConn()
{

}

int KcpConn::Init(conv_t conv, uv_udp_t* udp)
{
	int r = -1;
	udp_ = udp;
	kcp_ = ikcp_create(conv, (void*)this);
	CHK_COND(kcp_);
	kcp_->output = on_kcp_output;
	r = ikcp_nodelay(kcp_, 1, 10, 2, 1);
	PROC_ERR(r);
	conv_ = conv;
	return 0;
Exit0:
	ikcp_release(kcp_);
	kcp_ = NULL;
	return -1;
}

void KcpConn::Shutdown()
{
	if (kcp_ != NULL)
	{
		ikcp_release(kcp_);
		kcp_ = NULL;
	}
}

void KcpConn::Update(uint64_t ms)
{
	if (!need_update_ && next_update_time_ != 0)
	{
		if (ms < next_update_time_)
		{
			return;
		}
	}

	need_update_ = false;
	ikcp_update(kcp_, (uint32_t)ms);
	next_update_time_ = ikcp_check(kcp_, ms);

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
			server_->on_recv_kcp(this, buf, size);
		}
	}
}

int KcpConn::send_udp(const char* buf, uint32_t len)
{
	int r = -1;
	send_req_s* req = NEW(send_req_s);

	CHK_COND(req);

	req->buf.base = (char*) malloc_buff(len);
	req->buf.len = len;

	memcpy(req->buf.base, buf, len);

	r = uv_udp_send((uv_udp_send_t*)req, udp_, &req->buf, 1, &addr_, on_send_done);
	PROC_ERR(r);

	return 0;

Exit0:
	if (req) {
		free_buff(req->buf.base);
	}
	SAFE_DELETE(req);
	return r;
}

int KcpConn::Send(const char* buf, uint32_t len)
{
	need_update_ = true;
	return ikcp_send(kcp_, buf, len);
}

void KcpConn::on_recv_udp(const char* buf, ssize_t size, const struct sockaddr* addr)
{
	need_update_ = true;
	addr_ = *addr;
	ikcp_input(kcp_, buf, (long)size);
}

int KcpConn::recv_kcp(char** buf, uint32_t& size)
{
	int r = -1;
	char* data = NULL;

	int len = ikcp_peeksize(kcp_);
	CHK_COND_NOLOG(len > 0);

	data = (char*) malloc_buff(len);
	r = ikcp_recv(kcp_, data, len);
	PROC_ERR(r);

	size = (uint32_t)len;
	*buf = data;

	return 0;
	
Exit0:
	free_buff(data);
	return r;
}



