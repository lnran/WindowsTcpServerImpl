#pragma once
#include <WinSock2.h>

#define EXIT_CODE	(-1)
#define RELEASE(x)			{if(x != NULL) {delete x; x = NULL;}}
#define RELEASE_HANDLE(x)	{if(x != NULL && x != INVALID_HANDLE_VALUE) { CloseHandle(x); x = INVALID_HANDLE_VALUE; }}
#define RELEASE_SOCKET(x)	{if(x != INVALID_SOCKET) { closesocket(x); x = INVALID_SOCKET; }}

#define BUFF_SIZE 8192
#define MAX_POST_ACCEPT (10)	

enum IO_OPERATION_TYPE
{
    NULL_POSTED,		// 用于初始化，无意义
    ACCEPT_POSTED,		// 投递Accept操作
    SEND_POSTED,		// 投递Send操作
    RECV_POSTED,		// 投递Recv操作
};

class IOContext
{
public:
    WSAOVERLAPPED		overLapped;
    SOCKET				ioSocket;
    WSABUF				wsaBuf;
    IO_OPERATION_TYPE	ioType;

    // 备用参数
    int                 index;
    char			    message[BUFF_SIZE];
    int                 pos;
    int                 needLen;

    IOContext(IOContext* r);
    IOContext();
    ~IOContext();

    void Reset();
};