#ifndef _SL_COMMON_H_
#define _SL_COMMON_H_




enum SL_ERROR
{
	SL_NO_ERROR = 0,
	TCP_CONNECT_FAILED,
	TCP_DISCONNECTED,
	BUFF_NOT_ENOUGH,
	TCP_SEND_ERROR,
};

enum SL_CMD
{
	CMD_HELLO = 0,
};

#define CPP_OS_SWAP32(x) \
    ((((x) & 0xff000000) >> 24)  \
    | (((x) & 0x00ff0000) >> 8) \
    | (((x) & 0x0000ff00) << 8) \
    | (((x) & 0x000000ff) << 24))

#define CPP_OS_SWAP16(x) \
    ((((x) & 0xff00) >> 8)  \
    | (((x) & 0x00ff) << 8))

#define NEGOTIATE_BUFF_MAX 11
inline bool make_negotiate_msg(char cmd,char mode,int conv,unsigned int time,char param, char* buff, int& len)
{
	if (len < NEGOTIATE_BUFF_MAX)
	{
		return false;
	}

	int size = 0;
	memcpy(buff + size, &cmd, sizeof(cmd));
	size += sizeof(cmd);

	char _mode = (char)mode;
	memcpy(buff+size, &_mode, sizeof(_mode));
	size += sizeof(_mode);

	conv = CPP_OS_SWAP32(conv);
	memcpy(buff + size, &conv, sizeof(conv));
	size += sizeof(conv);

	time = CPP_OS_SWAP32(time);
	memcpy(buff + size, &time, sizeof(time));
	size += sizeof(time);

	memcpy(buff + size, &param, sizeof(param));
	size += sizeof(param);

	len = size;
	return true;
}

inline bool parse_negotiate_msg(char* buff, int len, char& cmd, char& mode,int& conv,unsigned int& time,char& param)
{
	if (len < NEGOTIATE_BUFF_MAX)
		return false;

	char _mode = 0;
	int size = 0;
	memcpy(&cmd, buff + size, sizeof(cmd));
	size += sizeof(cmd);
	memcpy(&_mode,buff+size, sizeof(_mode));
	size += sizeof(_mode);
	memcpy(&conv, buff + size, sizeof(conv));
	size += sizeof(conv);
	memcpy(buff + size, &time, sizeof(time));
	size += sizeof(time);
	memcpy(&param, buff + size, sizeof(param));
	size += sizeof(param);

	time = CPP_OS_SWAP32(time);
	conv = CPP_OS_SWAP32(conv);
	mode = _mode;
	return true;
}

#endif