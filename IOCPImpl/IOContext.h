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
    NULL_POSTED,		// ���ڳ�ʼ����������
    ACCEPT_POSTED,		// Ͷ��Accept����
    SEND_POSTED,		// Ͷ��Send����
    RECV_POSTED,		// Ͷ��Recv����
};

class IOContext
{
public:
    WSAOVERLAPPED		overLapped;
    SOCKET				ioSocket;
    WSABUF				wsaBuf;
    IO_OPERATION_TYPE	ioType;

    // ���ò���
    int                 index;
    char			    message[BUFF_SIZE];
    int                 pos;
    int                 needLen;

    IOContext(IOContext* r);
    IOContext();
    ~IOContext();

    void Reset();
};