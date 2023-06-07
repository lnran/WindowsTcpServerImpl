#pragma once

#include <iostream>
#include "../IOCPImpl/CallbackAPI.h"


class IOCPServer : public CallBackAPI::NotifyAPI
{
public:
    IOCPServer()
    {
        impl = CreateAPI();
        impl->SetNotifyAPI(this);
        impl->Start("127.0.0.1", 10240);
    }
    ~IOCPServer()
    {
        impl->Stop();
    }

    void SendData(SOCKET socketContext, const char* data, int size)
    {
        impl->SendData(socketContext, data, size);
    }

protected:
    // 新连接
    virtual void __cdecl OnConnectionEstablished(IOContext *ioContext)
    {
        ioContext->index = 10;
        SYSTEMTIME sys;
        GetLocalTime(&sys);
        printf("%4d-%02d-%02d %02d:%02d:%02d.%03d：Accept a connection，Current connects：%d\n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, impl->GetConnectCnt());

    }
    virtual void __cdecl OnPostNextConnect(IOContext *ioContext)
    {
        ioContext->index = 11;
    }
    // 连接关闭
    virtual void __cdecl OnConnectionClosed(IOContext* ioContext)
    {
        SYSTEMTIME sys;
        GetLocalTime(&sys);
        printf("%4d-%02d-%02d %02d:%02d:%02d.%03d：A connection had closed，Current connects：%d\n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, impl->GetConnectCnt());
    }
    // 连接上发生错误
    virtual void __cdecl OnConnectionError(int error)
    {
        SYSTEMTIME sys;
        GetLocalTime(&sys);
        printf("%4d-%02d-%02d %02d:%02d:%02d.%03d：A connection erro： %d，Current connects：%d\n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, error, impl->GetConnectCnt());
    }
    // 读操作完成
    virtual void __cdecl OnRecvCompleted(IOContext *ioContext)
    {
        //impl->SendData(ioContext->ioSocket, "hellowrold123123", 16);
        SYSTEMTIME sys;
        GetLocalTime(&sys);
        printf("%4d-%02d-%02d %02d:%02d:%02d.%03d：Recv data： %s \n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, ioContext->wsaBuf.buf);
    }
    // 写操作完成
    virtual void __cdecl OnSendCompleted(IOContext *ioContext)
    {
        SYSTEMTIME sys;
        GetLocalTime(&sys);
        printf("%4d-%02d-%02d %02d:%02d:%02d.%03d：Send data successd！\n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
    }

private:
    CallBackAPI::RequestAPI *impl;
};

int main()
{
    IOCPServer *pServer = new IOCPServer;

   

    getchar();
    return 0;
}