#pragma once
#include "IOContext.h"
#include "CallbackAPI.h"
#include <MSWSock.h>
#include <mutex>


class IOCPImpl :public CallBackAPI::RequestAPI
{
public:
    IOCPImpl();
    ~IOCPImpl();
private:
    int GetNumOfProcessors();
    BOOL PostAccept(IOContext * ioContext);
    BOOL PostRecv(IOContext * ioContext);
    BOOL PostSend(IOContext * ioContext);

    BOOL InitializeListenSocket();
    static DWORD WINAPI WorkerThreadProc(LPVOID lpParam);

    BOOL DoAccept(IOContext* ioContext);
    BOOL DoRecv(IOContext* ioContext);
    BOOL DoSend(IOContext* ioContext);
    BOOL DoClose(IOContext* ioContext);


    HANDLE					stopEvent;
    HANDLE					completionPort;
    HANDLE                  *workerThreads;

    SOCKET listenSocket;

    char _ip[20];
    int _port;
    int workerThreadNum;
    int ThreadID;
    long connectCnt;
    long acceptPostCnt;
    LPFN_ACCEPTEX fnAcceptEx;

    std::mutex smutex;

    CallBackAPI::NotifyAPI *_api;
protected:
    virtual void __cdecl SetNotifyAPI(CallBackAPI::NotifyAPI *api);
    virtual BOOL __cdecl Start(const char*ip, u_short port);
    virtual void __cdecl Stop();
    virtual void __cdecl Disconnect(IOContext* ioContext);
    virtual BOOL __cdecl SendData(SOCKET socketContext, const char *data, int size);
    virtual int  __cdecl GetConnectCnt();
};


