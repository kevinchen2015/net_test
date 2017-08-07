#ifndef __NETWORK_H__
#define __NETWORK_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define _EXTERN __declspec(dllexport)
# else
#define _EXTERN 
#endif

enum SL_MODE
{
	SMART_MODE = 0,
	TCP_MODE,
};

enum SL_CHANNEL_MODE
{
	TCP = 0,
	KCP,
};

//cb
typedef int sl_conv_t;
typedef void(*sl_on_recv_data)(char* buff, unsigned int size, void* conn, int channel,void* user_data);
typedef void(*sl_on_connected)(void* conn, int status, int channel,void* user_data);
typedef void(*sl_on_close)(void* conn, int channel, void* user_data);

//client
typedef struct sl_client_s sl_client_t;
_EXTERN bool sl_client_create(sl_client_t** client,void* user_data);
_EXTERN void sl_client_release(sl_client_t* client);
_EXTERN int  sl_client_init(sl_client_t* client, const char* ip,
	int tcp_port, int udp_port);
_EXTERN int  sl_client_connect(sl_client_t* client);
_EXTERN int  sl_client_send_data(sl_client_t* client, char* buff, unsigned int size);
_EXTERN void sl_client_switch_kcp(sl_client_t* client, sl_conv_t conv);
_EXTERN void sl_client_set_mode(sl_client_t* client, SL_MODE mode);
_EXTERN void sl_client_update(sl_client_t* client);
_EXTERN void sl_client_shutdown(sl_client_t* client);
_EXTERN void sl_client_cb(sl_client_t* client, sl_on_connected conn_cb, sl_on_recv_data recv_cb, sl_on_close close_cb);
_EXTERN SL_CHANNEL_MODE sl_client_get_channel_mode(sl_client_t* client);

//server
typedef struct sl_server_s sl_server_t;
_EXTERN bool sl_server_create(sl_server_t** server, void* user_data);
_EXTERN void sl_server_release(sl_server_t* server);
_EXTERN void sl_server_set_mode(sl_server_t* server, SL_MODE mode);
_EXTERN void sl_server_start(sl_server_t* server, const char* ip,
	int tcp_port, int udp_port, int max_conn_num);
_EXTERN void sl_server_set_conv(sl_server_t* server, void* conn, sl_conv_t conv);
_EXTERN int sl_server_send_data(sl_server_t* server, void* conn, char* buff,unsigned int size);
_EXTERN void sl_server_update(sl_server_t* server);
_EXTERN void sl_server_shutdown(sl_server_t* server);
_EXTERN void sl_server_cb(sl_server_t* server, sl_on_connected conn_cb, sl_on_recv_data recv_cb, sl_on_close close_cb);
_EXTERN SL_CHANNEL_MODE sl_server_get_channel_mode(sl_server_t* server,void* conn);
_EXTERN void sl_server_set_conn_user_data(void* conn, void* user_data);
_EXTERN void* sl_server_get_conn_user_data(void* conn);

#ifdef __cplusplus
}
#endif

#endif