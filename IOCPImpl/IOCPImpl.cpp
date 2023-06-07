#include "IOCPImpl.h"
#pragma comment(lib, "WS2_32.lib")

IOCPImpl::IOCPImpl()
{
    completionPort = INVALID_HANDLE_VALUE;
    workerThreadNum = 0;
    workerThreads = NULL;
    fnAcceptEx = NULL;
    connectCnt = 0;
    acceptPostCnt = 0;
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}


IOCPImpl::~IOCPImpl()
{

}

void IOCPImpl::SetNotifyAPI(CallBackAPI::NotifyAPI* api)
{
    _api = api;
}

BOOL IOCPImpl::Start(const char* ip, u_short port)
{
    memset(_ip, 0, sizeof(_ip));
    memcpy(_ip, ip, strlen(ip));

    _port = port;
    return InitializeListenSocket();
}

void IOCPImpl::Stop()
{
    if (listenSocket != INVALID_SOCKET)
    {
        SetEvent(stopEvent);
        for (int i = 0; i < workerThreadNum; i++)
        {
            PostQueuedCompletionStatus(completionPort, 0, (DWORD)EXIT_CODE, NULL);
        }

        // 等待所有工作线程退出
        WaitForMultipleObjects(workerThreadNum, workerThreads, TRUE, INFINITE);

        RELEASE_HANDLE(stopEvent);
        RELEASE_HANDLE(completionPort);
        RELEASE_SOCKET(listenSocket);
    }
}

void IOCPImpl::Disconnect(IOContext* ioContext)
{
    DoClose(ioContext);
}

int IOCPImpl::GetNumOfProcessors()
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
}

BOOL IOCPImpl::PostAccept(IOContext* ioContext)
{
    DWORD dwBytes = 0;
    ioContext->ioType = ACCEPT_POSTED;
    ioContext->ioSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);


    if (INVALID_SOCKET == ioContext->ioSocket)
        return false;
    if (false == fnAcceptEx(listenSocket, ioContext->ioSocket, ioContext->wsaBuf.buf, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwBytes, &ioContext->overLapped))
    {
        if (WSA_IO_PENDING != WSAGetLastError())
            return false;
    }
    InterlockedIncrement(&acceptPostCnt);
    return true;
}

BOOL IOCPImpl::PostRecv(IOContext* ioContext)
{
    DWORD dwFlags = 0;
    DWORD dwBytes = 0;
    ioContext->Reset();
    ioContext->ioType = RECV_POSTED;

    int nBytesRecv = WSARecv(ioContext->ioSocket, &ioContext->wsaBuf, 1, &dwBytes, &dwFlags, &ioContext->overLapped, NULL);
    if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
        return false;
    return true;
}

BOOL IOCPImpl::PostSend(IOContext* ioContext)
{
    DWORD dwFlags = 0;
    DWORD dwBytes = 0;
    ioContext->ioType = SEND_POSTED;

    if (::WSASend(ioContext->ioSocket, &ioContext->wsaBuf, 1, &dwBytes, dwFlags, &ioContext->overLapped, NULL) != NO_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
            return false;
    }
    return true;
}

BOOL IOCPImpl::InitializeListenSocket()
{
    listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (listenSocket == INVALID_SOCKET)
        return false;

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(_port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (SOCKET_ERROR == bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)))
    {
        return false;
    }

    if (SOCKET_ERROR == listen(listenSocket, SOMAXCONN))
    {
        return false;
    }
    completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (NULL == completionPort)
        return false;


    if (NULL == CreateIoCompletionPort((HANDLE)listenSocket, completionPort, ACCEPT_POSTED, 0))
    {
        RELEASE_SOCKET(listenSocket);
        return false;
    }


    GUID guidAcceptEx = WSAID_ACCEPTEX;
    GUID guidGetAcceptSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
    DWORD dwBytes = 0;
    WSAIoctl(listenSocket,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guidAcceptEx,
        sizeof(guidAcceptEx),
        &fnAcceptEx,
        sizeof(fnAcceptEx),
        &dwBytes,
        NULL,
        NULL);

    workerThreadNum = 2 * GetNumOfProcessors();
    workerThreads = new HANDLE[workerThreadNum];
    for (int i = 0; i < workerThreadNum; i++)
    {
        workerThreads[i] = CreateThread(0, 0, WorkerThreadProc, (void*)this, 0, 0);
        IOContext *context = new IOContext;
        PostAccept(context);
    }

    return true;
}

DWORD IOCPImpl::WorkerThreadProc(LPVOID lpParam)
{
    BOOL ret;

    IOCPImpl *iocp = (IOCPImpl*)lpParam;
    OVERLAPPED *ol = NULL;
    ULONG_PTR cl_key = 0;
    DWORD dwBytes = 0;
    IOContext *ioContext = NULL;

    while (WAIT_OBJECT_0 != WaitForSingleObject(iocp->stopEvent, NULL))
    {
        ret = GetQueuedCompletionStatus(iocp->completionPort, &dwBytes, &cl_key, &ol, INFINITE);

        ioContext = CONTAINING_RECORD(ol, IOContext, overLapped);

        if (EXIT_CODE == (DWORD)cl_key)
        {
            break;
        }

        if (!ret)
        {
            DWORD error = GetLastError();
            if (error == WAIT_TIMEOUT)
            {
                continue;
            }
            else if (error == ERROR_NETNAME_DELETED)
            {
                iocp->DoClose(ioContext);
                continue;
            }
            else
            {
                iocp->DoClose(ioContext);
                continue;
            }
        }
        else
        {
            if ((0 == dwBytes) && (RECV_POSTED == ioContext->ioType || SEND_POSTED == ioContext->ioType))
            {
                iocp->DoClose(ioContext);
                continue;
            }
            else
            {
                switch (ioContext->ioType)
                {
                case ACCEPT_POSTED:
                    iocp->DoAccept(ioContext);
                    break;
                case RECV_POSTED:
                    iocp->DoRecv(ioContext);
                    break;
                case SEND_POSTED:
                    iocp->DoSend(ioContext);
                    break;
                default:
                    break;
                }
            }
        }
    }

    // 释放线程参数
    RELEASE(lpParam);
    return 0;
}

BOOL IOCPImpl::DoAccept(IOContext* ioContext)
{
    InterlockedIncrement(&connectCnt);

    _api->OnConnectionEstablished(ioContext);
    IOContext *oldIoContext = new IOContext(ioContext);
    oldIoContext->ioType = RECV_POSTED;

    _api->OnPostNextConnect(ioContext);
    ioContext->Reset();
    PostAccept(ioContext);

    if (NULL == CreateIoCompletionPort((HANDLE)oldIoContext->ioSocket, completionPort, NULL, 0))
    {
        DWORD dwErr = WSAGetLastError();
        if (dwErr == ERROR_INVALID_PARAMETER)
        {
            return false;
        }
    }

    return PostRecv(oldIoContext);
}

BOOL IOCPImpl::DoRecv(IOContext* ioContext)
{
    _api->OnRecvCompleted(ioContext);
    ioContext->Reset();
    return PostRecv(ioContext);
}

BOOL IOCPImpl::DoSend(IOContext* ioContext)
{
    _api->OnSendCompleted(ioContext);
    return true;
}

BOOL IOCPImpl::DoClose(IOContext* ioContext)
{
    std::lock_guard<std::mutex> lock(smutex);
    _api->OnConnectionClosed(ioContext);
    InterlockedDecrement(&connectCnt);
    RELEASE(ioContext);
    return true;
}

BOOL IOCPImpl::SendData(SOCKET socketContext, const char* data, int size)
{
    IOContext* ioContext = new IOContext;
    ioContext->wsaBuf.len = size;
    ioContext->ioSocket = socketContext;
    memcpy(ioContext->wsaBuf.buf, data, size);
    PostSend(ioContext);
    return true;
}

int IOCPImpl::GetConnectCnt()
{   
    return connectCnt;
}
