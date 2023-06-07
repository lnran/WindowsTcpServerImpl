#include "IOContext.h"

IOContext::IOContext(IOContext* r)
{
    ZeroMemory(&overLapped, sizeof(overLapped));

    index = r->index;
    pos = r->pos;
    needLen = r->needLen;
    memset(message, 0, BUFF_SIZE);

    ioType = r->ioType;
    ioSocket = r->ioSocket;
    wsaBuf.len = r->wsaBuf.len;
    wsaBuf.buf = (char*)::HeapAlloc(GetProcessHeap(), (DWORD)r->wsaBuf.buf, r->wsaBuf.len);
}

IOContext::IOContext()
{
    ZeroMemory(&overLapped, sizeof(overLapped));
    ioSocket = INVALID_SOCKET;
    wsaBuf.len = BUFF_SIZE;
    wsaBuf.buf = (char*)::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BUFF_SIZE);
}

IOContext::~IOContext()
{
    if (wsaBuf.buf != NULL)
    {
        RELEASE_SOCKET(ioSocket);
        ::HeapFree(GetProcessHeap(), 0, wsaBuf.buf);
    }
}

void IOContext::Reset()
{
    if (wsaBuf.buf != NULL)
        ZeroMemory(wsaBuf.buf, BUFF_SIZE);
    else
        wsaBuf.buf = (char*)::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BUFF_SIZE);
    wsaBuf.len = BUFF_SIZE;
    ioType = NULL_POSTED;
    ZeroMemory(&overLapped, sizeof(overLapped));
}

