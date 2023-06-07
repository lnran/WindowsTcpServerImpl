#ifndef CALL_BACK_API_H
#define CALL_BACK_API_H
#include "IOContext.h"

namespace CallBackAPI
{
    class NotifyAPI
    {
    public:
        // �¼�֪ͨ����(���������ش��庯��)
        // ������
        virtual void __cdecl OnConnectionEstablished(IOContext *ioContext) = 0;
        // Ͷ����һ������
        virtual void __cdecl OnPostNextConnect(IOContext *ioContext) = 0;
        // ���ӹر�
        virtual void __cdecl OnConnectionClosed(IOContext* ioContext) = 0;
        // �����Ϸ�������
        virtual void __cdecl OnConnectionError(int error) = 0;
        // ���������
        virtual void __cdecl OnRecvCompleted(IOContext *ioContext) = 0;
        // д�������
        virtual void __cdecl OnSendCompleted(IOContext *ioContext) = 0;
    };
    class RequestAPI
    {
    public:
        virtual void __cdecl SetNotifyAPI(CallBackAPI::NotifyAPI *api)=0;
        virtual BOOL __cdecl Start(const char*ip, u_short port) = 0;
        virtual void __cdecl Stop() = 0;
        virtual void __cdecl Disconnect(IOContext* ioContext) = 0;
        virtual BOOL __cdecl SendData(SOCKET socketContext, const char *data, int size) = 0;
        virtual int  __cdecl GetConnectCnt() = 0;
    };
}

#ifdef __cplusplus
extern "C"{
#endif
    __declspec(dllexport) CallBackAPI::RequestAPI *__cdecl CreateAPI();
#ifdef __cplusplus
}
#endif


#endif
